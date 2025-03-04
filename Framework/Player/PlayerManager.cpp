// PlayerManager.cpp

#include "Base/BaseUtils.h"
#include "Constant/RedisKeys.h"
#include "Redis/RedisPool.h"
#include "Timer/TimerManager.h"
#include "MySql/MysqlPool.h"
#include "PlayerManager.h"
#include "LoadPlayerTask.h"

#include <mysql/jdbc.h>
#include <boost/locale.hpp>

#include <chrono>

namespace NiuMa {
	template<> PlayerManager* Singleton<PlayerManager>::_inst = nullptr;

	PlayerManager::PlayerManager() {
		_inexactTime = BaseUtils::getCurrentSecond();
	}

	PlayerManager::~PlayerManager() {}

	void PlayerManager::init() {
		// 添加定时任务
		_timer = std::make_shared<int>();
		std::weak_ptr<int> weak(_timer);
		TimerManager::getSingleton().addAsyncTimer(5000, [weak] {
			std::shared_ptr<int> strong = weak.lock();
			if (!strong)
				return true;
			return PlayerManager::getSingleton().onTimer();
			});
	}

	Player::Ptr PlayerManager::getPlayer(const std::string& playerId) const {
		Player::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Player::Ptr>::const_iterator it = _players.find(playerId);
		if (it != _players.end()) {
			ret = it->second;
			// 更新引用时间
			ret->touch(_inexactTime);
		}
		return ret;
	}

	Player::Ptr PlayerManager::loadPlayer(const std::string& playerId) {
		Player::Ptr player = getPlayer(playerId);
		if (player)
			return player;
		player = std::make_shared<Player>(playerId);
		if (loadPlayer(player)) {
			if (!addPlayer(player))
				player = getPlayer(playerId);
			return player;
		}
		return nullptr;
	}

	bool PlayerManager::loadPlayer(const Player::Ptr& player) const {
		if (!player)
			return false;
		std::shared_ptr<LoadPlayerTask> task = std::make_shared<LoadPlayerTask>(player->getId());
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed())
			return false;
		player->setName(task->_name);
		player->setNickname(task->_nickname);
		player->setPhone(task->_phone);
		player->setSex(task->_sex);
		player->setAvatar(task->_avatar);
		return true;
	}

	bool PlayerManager::addPlayer(const Player::Ptr& player) {
		if (!player)
			return false;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Player::Ptr>::iterator it = _players.find(player->getId());
		if (it != _players.end())
			return false;
		_players.insert(std::make_pair(player->getId(), player));
		return true;
	}

	bool PlayerManager::verifySignature(
		const std::string& playerId,
		const std::string& timestamp,
		const std::string& nonce,
		const std::string& signature,
		bool& outdate) {
		if (playerId.empty())
			return false; // 玩家id非法
		time_t time1 = BaseUtils::getCurrentSecond();
		time_t time2 = atoll(timestamp.c_str());
		time_t delta = abs(time1 - time2);
		if (delta > 60L) {
			// 传入时间戳与当前时间戳相差大于60秒
			outdate = true;
			return false;
		}
		std::string secret;
		Player::Ptr player = loadPlayer(playerId);
		if (!player) {
			// 加载玩家失败
			return false;
		}
		if (!player->testNonce(nonce, time2))
			return false;	// 随机串冲突
		player->getSecret(secret);
		bool test = false;
		std::string redisKey;
		if (secret.empty()) {
			// 从redis中获取
			redisKey = RedisKeys::PLAYER_MESSAGE_SECRET + playerId;
			if (!RedisPool::getSingleton().get(redisKey, secret))
				return false;	// 从Redis获取密钥失败
			if (!secret.empty())
				player->setSecret(secret);
			test = true;
		}
		if (secret.empty())
			return false; // 未分配密钥
		std::string text = playerId + '&' + timestamp + '&' + nonce + '&' + secret;
		std::string md5;
		BaseUtils::encodeMD5(text, md5);
		if (md5 != signature) {
			if (!test) {
				// 尝试从Redis中获取密钥
				std::string temp;
				redisKey = RedisKeys::PLAYER_MESSAGE_SECRET + playerId;
				if (!RedisPool::getSingleton().get(redisKey, temp))
					return false;	// 从Redis获取密钥失败
				if (temp == secret)
					return false;	// 密钥未发生变化
				secret = temp;
				player->setSecret(secret);
				if (secret.empty())
					return false;
				text = playerId + '&' + timestamp + '&' + nonce + '&' + secret;
				BaseUtils::encodeMD5(text, md5);
				if (md5 != signature)
					return false;
			}
		}
		return true;
	}

	void PlayerManager::setSessionPlayerId(const std::string& sessionId, const std::string& playerId) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::string>::iterator it = _sessionMap.find(sessionId);
		if (it == _sessionMap.end())
			_sessionMap.insert(std::make_pair(sessionId, playerId));
		else
			it->second = playerId;
	}

	void PlayerManager::removeSessionId(const std::string& sessionId) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::string>::iterator it = _sessionMap.find(sessionId);
		if (it != _sessionMap.end())
			_sessionMap.erase(it);
	}

	bool PlayerManager::getPlayerId(const std::string& sessionId, std::string& playerId) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::string>::const_iterator it = _sessionMap.find(sessionId);
		if (it != _sessionMap.end()) {
			playerId = it->second;
			return true;
		}
		return false;
	}

	Player::Ptr PlayerManager::getPlayerBySessionId(const std::string& sessionId) {
		Player::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::string>::const_iterator it1 = _sessionMap.find(sessionId);
		if (it1 == _sessionMap.end())
			return ret;
		std::unordered_map<std::string, Player::Ptr>::const_iterator it2 = _players.find(it1->second);
		if (it2 != _players.end())
			ret = it2->second;
		return ret;
	}

	void PlayerManager::addOfflinePlayer(const std::string& playerId) {
		std::lock_guard<std::mutex> lck(_mtx);

		_offlineIds.push_back(playerId);
	}

	bool PlayerManager::onTimer() {
		freeDormantPlayers();
		return false;
	}

	void PlayerManager::freeDormantPlayers() {
		std::lock_guard<std::mutex> lck(_mtx);

		if (_offlineIds.empty())
			return;
		time_t delta = 0LL;
		time_t nowTime = BaseUtils::getCurrentSecond();
		time_t offlineTime = 0LL;
		time_t referenceTime = 0LL;
		_inexactTime = nowTime;
		std::string venueId;
		std::list<std::string>::const_iterator it1 = _offlineIds.begin();
		std::unordered_map<std::string, Player::Ptr>::const_iterator it2;
		while (it1 != _offlineIds.end()) {
			it2 = _players.find(*it1);
			if (it2 == _players.end()) {
				it1 = _offlineIds.erase(it1);
				break;
			}
			const Player::Ptr& player = it2->second;
			if (!player->getOffline()) {
				it1 = _offlineIds.erase(it1);
				break;
			}
			player->getVenueId(venueId);
			if (!venueId.empty()) {
				// 玩家还在场地中，暂不释放
				it1++;
				continue;
			}
			player->getTime(offlineTime, referenceTime);
			delta = nowTime - offlineTime;
			if (delta < 30LL) {
				// 离线时间少于30秒，退出循环
				break;
			}
			delta = nowTime - referenceTime;
			if (delta < 20LL) {
				// 最新引用时间少于20秒，暂不释放
				it1++;
				continue;
			}
			// 释放玩家
			it1 = _offlineIds.erase(it1);
			_players.erase(it2);
		}
	}
}