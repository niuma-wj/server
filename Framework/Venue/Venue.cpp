// Venue.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Venue.h"
#include "Constant/RedisKeys.h"
#include "Redis/RedisPool.h"
#include "Player/PlayerManager.h"
#include "MySql/MysqlPool.h"
#include "VenueInnerHandler.h"
#include "VenueManager.h"
#include "VenueMessages.h"

#include <boost/locale.hpp>

namespace NiuMa {
	Venue::Venue(const std::string& id, int gameType)
		: _id(id)
		, _gameType(gameType)
		, _status(0)
		, _obsolete(false)
		, _obsoleteTime(0)
	{}

	Venue::~Venue() {}

	const std::string& Venue::getId() const {
		return _id;
	}

	int Venue::getGameType() const {
		return _gameType;
	}

	void Venue::setHandler(const std::shared_ptr<VenueInnerHandler>& handler) {
		_handler = handler;
	}

	std::shared_ptr<VenueInnerHandler> Venue::getHandler() {
		return _handler.lock();
	}

	void Venue::initialize() {}

	bool Venue::onMessage(const NetMessage::Ptr& netMsg) {
		bool ret = true;
		if (MsgHeartbeat::TYPE == netMsg->getType())
			onHeartbeat(netMsg);
		else
			ret = false;
		return ret;
	}

	void Venue::onTimer() {}

	int Venue::getStatus() {
		return _status;
	}

	bool Venue::onEnter(const std::string& playerId, const std::string& base64, std::string& errMsg) {
		if (hasPlayer(playerId))
			return true;	// 玩家已经在场地内，直接返回成功
		if (_status != 0) {
			errMsg = "Enter venue error, the venue status is abnormal";
			return false;	// 非正常状态的场地不能进入
		}
		bool ret = enterImpl(playerId, base64, errMsg);
		if (ret)
			afterEnter(playerId);
		return ret;
	}

	int Venue::onLeave(const std::string& playerId, std::string& errMsg) {
		int ret = 0;
		if (hasPlayer(playerId)) {
			ret = leaveImpl(playerId, errMsg);
			if (ret == 0)
				afterLeave(playerId);
		}
		return ret;
	}

	void Venue::afterEnter(const std::string& playerId) {
		// 设置玩家当前所在场地
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (player)
			player->setVenueId(getId());
		std::string redisKey = RedisKeys::PLAYER_CURRENT_VENUE + playerId;
		RedisPool::getSingleton().set(redisKey, getId());
		// 更新玩家数量
		int count = updatePlayerCount();

		InfoS << "Player entered, playerId: " << playerId << ", venueId: " << getId() << ", current player count: " << count;
	}

	void Venue::afterLeave(const std::string& playerId) {
		// 删除玩家当前所在场地
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (player)
			player->setVenueId(std::string());
		std::string redisKey = RedisKeys::PLAYER_CURRENT_VENUE + playerId;
		RedisPool::getSingleton().del(redisKey);
		// 更新玩家数量
		int count = updatePlayerCount();

		InfoS << "Player leaved, playerId: " << playerId << ", venueId: " << getId() << ", current player count: " << count;
	}

	int Venue::updatePlayerCount() {
		int count = getPlayerCount();
		std::shared_ptr<VenueInnerHandler> strong = _handler.lock();
		if (strong)
			strong->setPlayerCount(_id, count);
		// 将场地内当前玩家数量更新到Redis
		std::string redisKey = RedisKeys::VENUE_PLAYER_COUNT + _id;
		if (count == 0)
			RedisPool::getSingleton().del(redisKey);
		else
			RedisPool::getSingleton().set(redisKey, count);
		return count;
	}

	void Venue::gameOver(bool delDb) {
		_status = 2;

		Player::Ptr player;
		std::string redisKey;
		std::vector<std::string> playerIds;
		getPlayerIds(playerIds, false);
		for (const std::string& playerId : playerIds) {
			// 删除玩家当前所在场地
			player = PlayerManager::getSingleton().getPlayer(playerId);
			if (player)
				player->setVenueId(std::string());
			redisKey = RedisKeys::PLAYER_CURRENT_VENUE + playerId;
			RedisPool::getSingleton().del(redisKey);
		}
		redisKey = RedisKeys::VENUE_SERVER_MAP + _id;
		RedisPool::getSingleton().del(redisKey);

		updatePlayerCount();
		if (delDb)
			deleteVenueFromDb();
		else
			updateStatus2Db();
		setObsolete();

		InfoS << "Game over, venueId: " << _id;
	}

	void Venue::updateStatus2Db() {
		std::string sql = "update `venue` set `status` = " + std::to_string(_status) + " where `id` = \"" + _id + "\"";
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
		MysqlPool::getSingleton().syncQuery(task);
	}

	void Venue::deleteVenueFromDb() {
		std::string sql = "delete from `venue` where `id` = \"" + _id + "\"";
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Delete);
		MysqlPool::getSingleton().asyncQuery(task);
	}

	void Venue::onHeartbeat(const NetMessage::Ptr& netMsg) {
		Session::Ptr session = netMsg->getSession();
		if (!session)
			return;
		MsgHeartbeat* msg = dynamic_cast<MsgHeartbeat*>(netMsg->getMessage().get());
		if (msg == nullptr)
			return;
		if (!hasPlayer(msg->getPlayerId()))
			return;
		heartbeatImpl(msg->getPlayerId());
		session->heartbeat();
		MsgHeartbeatResp resp;
		resp.counter = msg->counter;
		resp.send(session);
	}

	void Venue::heartbeatImpl(const std::string& playerId) {}

	void Venue::onConnect(const std::string& playerId) {
		InfoS << "Player(id: " << playerId << ") reconnect, venueId: " << _id;
	}

	void Venue::onDisconnect(const std::string& playerId) {
		InfoS << "Player(id: " << playerId << ") disconnect, venueId: " << _id;
	}

	void Venue::onLeaveVenue(const NetMessage::Ptr& netMsg) {
		MsgLeaveVenue* inst = dynamic_cast<MsgLeaveVenue*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		std::string errMsg;
		int result = onLeave(inst->getPlayerId(), errMsg);
#ifdef _MSC_VER
		// VC环境下gb2312编码转utf8
		errMsg = boost::locale::conv::to_utf<char>(errMsg, std::string("gb2312"));
#endif
		MsgLeaveVenueResp resp;
		resp.venueId = inst->getVenueId();
		resp.errMsg = errMsg;
		resp.result = result;
		resp.send(netMsg->getSession());
	}

	bool Venue::isObsolete() const {
		return _obsolete;
	}

	time_t Venue::getObsoleteTime() const {
		return _obsoleteTime;
	}

	void Venue::setObsolete() {
		_obsolete = true;
		_obsoleteTime = BaseUtils::getCurrentMillisecond();
	}
}