// GameRoom.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "GameRoom.h"
#include "Player/PlayerManager.h"
#include "MySql/MysqlPool.h"
#include "Timer/TimerManager.h"
#include "GetCapitalTask.h"
#include "GetCashPledgeTask.h"
#include "GameMessages.h"
#include "GetAgencyTask.h"

#include <boost/locale.hpp>
#include <sstream>

namespace NiuMa
{
	GameRoom::GameRoom(const std::string& id, int gameType, int maxPlayerNums, RoomCategory category)
		: Venue(id, gameType)
		, _category(category)
		, _maxPlayerNums(maxPlayerNums)
		, _cashPledge(0)
		, _diamondNeed(0)
	{}

	GameRoom::~GameRoom() {}

	RoomCategory GameRoom::getCategory() const {
		return _category;
	}

	int GameRoom::getMaxPlayerNums() const {
		return _maxPlayerNums;
	}

	int64_t GameRoom::getCashPledge() const {
		return _cashPledge;
	}

	void GameRoom::setCashPledge(int64_t cashPledge) {
		_cashPledge = cashPledge;
	}

	int64_t GameRoom::getDiamondNeed() const {
		return _diamondNeed;
	}

	void GameRoom::setDiamondNeed(int64_t diamondNeed) {
		_diamondNeed = diamondNeed;
	}

	GameAvatar::Ptr GameRoom::getAvatar(int seat) const {
		if (seat >= MAX_SEAT_NUMS)
			return nullptr;
		return _avatarSeats[seat];
	}

	GameAvatar::Ptr GameRoom::getAvatar(const std::string& playerId) const {
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = _allAvatars.find(playerId);
		if (it != _allAvatars.end())
			return it->second;
		return nullptr;
	}

	int GameRoom::getAvatarCount() const {
		return static_cast<int>(_allAvatars.size());
	}

	bool GameRoom::hasSpectator(const std::string& playerId) const {
		std::unordered_set<std::string>::const_iterator it_s = _spectators.find(playerId);
		return (it_s != _spectators.end());
	}

	const std::unordered_map<std::string, GameAvatar::Ptr>& GameRoom::getAllAvatars() const {
		return _allAvatars;
	}

	bool GameRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (Venue::onMessage(netMsg))
			return true;
		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (MsgGetAvatars::TYPE == msgType)
			onGetAvatars(netMsg);
		else if (MsgGetSpectators::TYPE == msgType)
			onGetSpectators(netMsg);
		else if (msgType == MsgPlayerGeolocation::TYPE)
			onPlayerGeolocation(netMsg);
		else if (msgType == MsgGetDistances::TYPE)
			onDistancesRequest(netMsg);
		else if (MsgChatClient::TYPE == msgType)
			onChatClient(netMsg);
		else if (MsgEffectClient::TYPE == msgType)
			onEffectClient(netMsg);
		else if (MsgVoiceClient::TYPE == msgType)
			onVoiceClient(netMsg);
		else
			ret = false;
		return ret;
	}

	void GameRoom::onConnect(const std::string& playerId) {
		Venue::onConnect(playerId);
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar)
			return;
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (!player)
			return;
		avatar->setOffline(false);
		avatar->setSession(player->getSession());
		// 通知玩家上线
		MsgAvatarConnect msg;
		msg.playerId = playerId;
		msg.seat = avatar->getSeat();
		msg.offline = false;
		player->getIp(msg.ip);
		sendMessageToAll(msg, BaseUtils::EMPTY_STRING, true);
	}

	void GameRoom::onDisconnect(const std::string& playerId) {
		Venue::onDisconnect(playerId);
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar)
			return;
		avatar->setOffline(true);
		avatar->setSession(Session::Ptr());
		// 通知玩家离线
		MsgAvatarConnect msg;
		msg.playerId = playerId;
		msg.seat = avatar->getSeat();
		msg.offline = true;
		sendMessageToAll(msg, playerId, true);
	}

	int GameRoom::getEmptySeat() const {
		if (RoomCategory::RoomCategoryA == _category) {
			for (int i = 0; i < _maxPlayerNums; i++) {
				if (!_avatarSeats[i])
					return i;
			}
		}
		return -1;
	}

	bool GameRoom::isFull() const {
		if (RoomCategory::RoomCategoryA != _category)
			return false;
		if (_maxPlayerNums <= 0)
			return false;
		if (_maxPlayerNums > static_cast<int>(_allAvatars.size()))
			return false;
		return true;
	}

	bool GameRoom::isAllReady() const {
		bool ret = true;
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = _allAvatars.begin();
		while (it != _allAvatars.end()) {
			const GameAvatar::Ptr& avatar = it->second;
			if (!avatar->isReady()) {
				ret = false;
				break;
			}
			++it;
		}
		return ret;
	}

	bool GameRoom::hasPlayer(const std::string& playerId) {
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = _allAvatars.find(playerId);
		if (it != _allAvatars.end())
			return true;
		if (RoomCategory::RoomCategoryA == _category) {
			if (hasSpectator(playerId))
				return true;
		}
		return false;
	}

	void GameRoom::getPlayerIds(std::vector<std::string>& playerIds) {
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it1 = _allAvatars.begin();
		while (it1 != _allAvatars.end()) {
			playerIds.emplace_back(it1->first);
			++it1;
		}
		std::unordered_set<std::string>::const_iterator it2 = _spectators.begin();
		while (it2 != _spectators.end()) {
			playerIds.emplace_back(*it2);
			++it2;
		}
	}

	int GameRoom::getPlayerCount() {
		return static_cast<int>(_allAvatars.size() + _spectators.size());
	}

	bool GameRoom::enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) {
		bool test = enableSpectator();
		int seat = -1;
		if (!test && (RoomCategory::RoomCategoryA == _category)) {
			// A类房且不允许观众存在，即进入房间就必须加入游戏，检查是否还有空位
			seat = getEmptySeat();
			if (seat == -1) {
				errMsg = "游戏已满员";
				return false;
			}
		}
		if (!checkEnter(playerId, errMsg))
			return false;
		if (test) {
			// 添加观众
			_spectators.insert(playerId);
			// 通知观众加入
			notifyAddSpectator(playerId);
		}
		else {
			// 加入游戏
			if (!joinGame(seat, playerId, errMsg))
				return false;
			// 通知玩家加入
			notifyAddAvatar(seat, playerId);
			// 执行玩家加入后的上层逻辑
			onAvatarJoined(seat, playerId);
		}
		return true;
	}

	bool GameRoom::joinGame(int seat, const std::string& playerId, std::string& errMsg) {
		if (!checkJoin(seat, playerId, errMsg))
			return false;
		// 当前已经预扣初的押金数量
		int64_t cashPledge = 0LL;
		if ((_cashPledge > 0) || (_diamondNeed > 0)) {
			// 检查是否有足够金币预扣押金，以及是否有足够的钻石数量用于扣除
			if (_cashPledge > 0) {
				// 查询当前已经预扣初的押金数量
				std::shared_ptr<GetCashPledgeTask> task1 = std::make_shared<GetCashPledgeTask>(playerId, getId());
				MysqlPool::getSingleton().syncQuery(task1);
				if (!task1->getSucceed()) {
					errMsg = "数据库错误";
					ErrorS << "玩家(Id: " << playerId << ")加入游戏(Id: " << getId() << ")：查询押金失败";
					return false;
				}
				if (task1->getRows() > 0)
					cashPledge = task1->getAmount();
			}
			int64_t gold = 0LL;
			int64_t diamond = 0LL;
			std::shared_ptr<GetCapitalTask> task2;
			if ((cashPledge < _cashPledge) || (_diamondNeed > 0)) {
				task2 = std::make_shared<GetCapitalTask>(playerId);
				MysqlPool::getSingleton().syncQuery(task2);
				if (!task2->getSucceed()) {
					errMsg = "数据库错误";
					ErrorS << "玩家(Id: " << playerId << ")加入游戏(Id: " << getId() << ")：查询资产失败";
					return false;
				}
				if (task2->getRows() > 0) {
					gold = task2->getGold();
					diamond = task2->getDiamond();
				}
			}
			if ((_diamondNeed > 0) && (diamond < _diamondNeed)) {
				errMsg = std::string("钻石不足，最低需要") + std::to_string(_diamondNeed) + std::string("枚钻石");
				return false;
			}
			if (cashPledge < _cashPledge) {
				int64_t delta = _cashPledge - cashPledge;
				cashPledge = _cashPledge;
				if (gold < delta) {
					// 金币不足
					errMsg = std::string("金币不足，最低需要") + std::to_string(_cashPledge) + std::string("金币");
					return false;
				}
				// 预扣除押金
				gold -= delta;
				std::stringstream ss;
				ss << "update `capital` set `gold` = " << gold
					<< ", `version` = `version` + 1 where `player_id` = \""
					<< playerId << "\" and `version` = " << task2->getVersion();
				std::string sql = ss.str();
				std::shared_ptr<MysqlCommonTask> task3 = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
				MysqlPool::getSingleton().syncQuery(task3);
				if (!task3->getSucceed()) {
					// 扣除金币失败
					errMsg = "数据库错误";
					ErrorS << "玩家(Id: " << playerId << ")加入游戏(Id: " << getId() << ")：扣除押金失败";
					return false;
				}
				if ((task3->getAffectedRecords() < 1)) {
					errMsg = "数据库冲突";
					return false;
				}
				// 添加押金记录
				if (!updateCashPledge(playerId, _cashPledge)) {
					// 更新押金失败
					errMsg = "数据库错误";
					ErrorS << "玩家(Id: " << playerId << ")加入游戏(Id: " << getId() << ")：添加或者更新押金失败，应返还金币数量: " << delta;
					return false;
				}
			}
		}
		// 创建玩家替身
		GameAvatar::Ptr avatar = createAvatar(playerId, seat, false);
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (player) {
			avatar->setNickname(player->getNickname());
			avatar->setPhone(player->getPhone());
			avatar->setSex(player->getSex());
			avatar->setHeadUrl(player->getAvatar());
			avatar->setSession(player->getSession());
		}
		if (seat > -1) {
			avatar->setSeat(seat);
			_avatarSeats[seat] = avatar;
		}
		if (_cashPledge > 0)
			avatar->setCashPledge(static_cast<int>(cashPledge));
		addAvatar(avatar);
		return true;
	}

	int GameRoom::leaveImpl(const std::string& playerId, std::string& errMsg) {
		int ret = checkLeave(playerId, errMsg);
		if (ret != 0)
			return ret;
		if (hasSpectator(playerId)) {
			// 删除观众
			removeSpectator(playerId);
			// 通知观众离开
			notifyRemoveSpectator(playerId);
		}
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar)
			return ret;
		// 返还玩家押金
		if (returnCashPledge(playerId))
			avatar->setCashPledge(0LL);
		int seat = -1;
		if (RoomCategory::RoomCategoryA == _category) {
			seat = avatar->getSeat();
			_avatarSeats[seat].reset();
		}
		removeAvatar(playerId);
		// 通知玩家离开
		notifyRemoveAvatar(seat, playerId);
		// 执行玩家离开后的上层逻辑
		onAvatarLeaved(seat, playerId);
		return ret;
	}

	bool GameRoom::enableSpectator() const {
		return false;
	}

	bool GameRoom::checkJoin(int seat, const std::string& playerId, std::string& errMsg) {
		return true;
	}

	void getAvatarInfo(AvatarInfo& info, const GameAvatar::Ptr& avatar) {
		info.playerId = avatar->getPlayerId();
		info.nickname = avatar->getNickname();
		info.headUrl = avatar->getHeadUrl();
		info.seat = avatar->getSeat();
		info.sex = avatar->getSex();
		info.ready = avatar->isReady();
		info.offline = avatar->isOffline();
	}

	void GameRoom::notifyAddAvatar(int seat, const std::string& playerId) {
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar)
			return;
		AvatarInfo info;
		getAvatarInfo(info, avatar);
		getAvatarExtraInfo(avatar, info.base64);
		MsgAddAvatar msg;
		msg.avatars.push_back(info);
		sendMessageToAll(msg, playerId, true);
	}

	void GameRoom::notifyRemoveAvatar(int seat, const std::string& playerId) {
		MsgRemoveAvatar msg;
		msg.playerId = playerId;
		msg.seat = seat;
		sendMessageToAll(msg, playerId, true);
	}

	void GameRoom::notifyAddSpectator(const std::string& playerId) {
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (!player)
			return;
		SpectatorInfo info;
		info.playerId = playerId;
		info.nickname = player->getNickname();
		info.headUrl = player->getAvatar();
		getSpectatorExtraInfo(playerId, info.base64);
		MsgAddSpectator msg;
		msg.spectators.push_back(info);
		sendMessageToAll(msg, playerId, true);
	}

	void GameRoom::notifyRemoveSpectator(const std::string& playerId) {
		MsgRemoveSpectator msg;
		msg.playerId = playerId;
		sendMessageToAll(msg, playerId, true);
	}

	void GameRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {}

	void GameRoom::getSpectatorExtraInfo(const std::string& playerId, std::string& base64) const {}

	void GameRoom::onAvatarJoined(int seat, const std::string& playerId) {
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (!avatar)
			return;
		std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(playerId);
		MysqlPool::getSingleton().syncQuery(task);
		if (task->getSucceed())
			avatar->setGold(task->getGold());
	}

	void GameRoom::onAvatarLeaved(int seat, const std::string& playerId) {}

	void GameRoom::addAvatar(const GameAvatar::Ptr& avatar) {
		if (!avatar)
			return;
		std::unordered_map<std::string, GameAvatar::Ptr>::iterator it = _allAvatars.find(avatar->getPlayerId());
		if (it != _allAvatars.end())
			it->second = avatar;
		else
			_allAvatars.insert(std::make_pair(avatar->getPlayerId(), avatar));
	}

	void GameRoom::removeAvatar(const std::string& playerId) {
		std::unordered_map<std::string, GameAvatar::Ptr>::iterator it = _allAvatars.find(playerId);
		if (it != _allAvatars.end())
			_allAvatars.erase(it);
	}

	void GameRoom::removeSpectator(const std::string& playerId) {
		std::unordered_set<std::string>::iterator it_s = _spectators.find(playerId);
		if (it_s != _spectators.end())
			_spectators.erase(it_s);
	}

	bool GameRoom::returnCashPledge(const std::string& playerId) const {
		// 查询当前押金数量
		std::shared_ptr<GetCashPledgeTask> task1 = std::make_shared<GetCashPledgeTask>(playerId, getId());
		MysqlPool::getSingleton().syncQuery(task1);
		if (!task1->getSucceed()) {
			ErrorS << "查询押金失败，场地Id: " << getId() << ", 玩家Id: " << playerId;
			return false;
		}
		bool flag = false;
		int64_t cashPledge = 0LL;
		if (task1->getRows() > 0) {
			flag = true;
			cashPledge = task1->getAmount();
		}
		std::stringstream ss;
		// 删除押金记录
		ss << "delete from `cash_pledge` where `player_id` = \"" << playerId << "\" and `venue_id` = \"" << getId() << "\"";
		std::string sql = ss.str();
		std::shared_ptr<MysqlCommonTask> taskDelete = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Delete);
		MysqlPool::getSingleton().syncQuery(taskDelete);
		if (!taskDelete->getSucceed()) {
			ErrorS << "删除押金记录失败，场地Id: " << getId() << ", 玩家Id: " << playerId;
			return false;
		}
		if (cashPledge <= 0)
			return true;
		// 更新金币数量，循环多次以应对可能的数据库冲突的情况
		for (int i = 0; i < 10; i++) {
			std::shared_ptr<GetCapitalTask> task2 = std::make_shared<GetCapitalTask>(playerId);
			MysqlPool::getSingleton().syncQuery(task2);
			if (!task2->getSucceed()) {
				ErrorS << "返还押金失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 应返还金币数量: " << cashPledge;
				return false;
			}
			int64_t gold = 0LL;
			if (task2->getRows() > 0)
				gold = task2->getGold();
			gold += cashPledge;
			ss.str("");
			ss << "update `capital` set `gold` = " << gold
				<< ", `version` = `version` + 1 where `player_id` = \""
				<< playerId << "\" and `version` = " << task2->getVersion();
			sql = ss.str();
			std::shared_ptr<MysqlCommonTask> task3 = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			MysqlPool::getSingleton().syncQuery(task3);
			if (!task3->getSucceed()) {
				ErrorS << "返还押金失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 应返还金币数量: " << cashPledge;
				return false;
			}
			if (task3->getAffectedRecords() > 0)
				return true;
		}
		ErrorS << "返还押金失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 应返还金币数量: " << cashPledge;
		return false;
	}

	bool GameRoom::deductCashPledge(const GameAvatar::Ptr& avatar) const {
		return deductCashPledge(avatar, _cashPledge, true);
	}

	bool GameRoom::deductCashPledge(const GameAvatar::Ptr& avatar, int64_t goldNeed, bool kick) const {
		if (!avatar)
			return false;
		int64_t cashPledge = avatar->getCashPledge();
		if (cashPledge >= goldNeed)
			return true;
		std::stringstream ss;
		std::string sql;
		int64_t delta = goldNeed - cashPledge;
		// 循环多次以应对可能的数据库冲突的情况
		for (int i = 0; i < 10; i++) {
			std::shared_ptr<GetCapitalTask> task1 = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
			MysqlPool::getSingleton().syncQuery(task1);
			if (!task1->getSucceed() || (task1->getRows() < 1)) {
				ErrorS << "扣除押金失败：查询失败，场地Id: " << getId() << ", 玩家Id: "
					<< avatar->getPlayerId() << ", 当前押金数量: " << cashPledge;
				return false;
			}
			int64_t gold = task1->getGold();
			bool test = true;
			if (gold < delta) {
				if (kick) {
					// 玩家金币不足够补充扣除，将剩余的全部押金转成玩家金币，之后玩家会被踢出游戏房
					gold += cashPledge;
					test = false;
				}
				else {
					// 不将玩家踢出，简单返回补充扣除失败
					return false;
				}
			}
			else
				gold -= delta;
			if (test) {
				// 更新数据库中的押金数量
				if (!updateCashPledge(avatar->getPlayerId(), goldNeed)) {
					ErrorS << "扣除押金失败：更新押金数量失败，场地Id: " << getId() << ", 玩家Id: "
						<< avatar->getPlayerId() << ", 当前押金数量: " << cashPledge;
					return false;
				}
			}
			else {
				// 删除数据库中的押金记录
				if (!updateCashPledge(avatar->getPlayerId(), 0)) {
					ErrorS << "扣除押金失败：删除押金记录失败，场地Id: " << getId() << ", 玩家Id: "
						<< avatar->getPlayerId() << ", 当前押金数量: " << cashPledge;
					return false;
				}
			}
			ss.str("");
			ss << "update `capital` set `gold` = " << gold
				<< ", `version` = `version` + 1 where `player_id` = \""
				<< avatar->getPlayerId() << "\" and `version` = " << task1->getVersion();
			sql = ss.str();
			std::shared_ptr<MysqlCommonTask> task2 = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			MysqlPool::getSingleton().syncQuery(task2);
			if (!task2->getSucceed()) {
				ErrorS << "扣除押金失败：更新金币数量失败，场地Id: " << getId() << ", 玩家Id: "
					<< avatar->getPlayerId() << ", 当前押金数量: " << cashPledge;
				return false;
			}
			if (task2->getAffectedRecords() > 0) {
				if (test)
					cashPledge = goldNeed;
				else
					cashPledge = 0;
				avatar->setGold(gold);
				avatar->setCashPledge(cashPledge);
				return test;
			}
		}
		ErrorS << "扣除押金失败，场地Id: " << getId() << ", 玩家Id: " << avatar->getPlayerId() << ", 当前押金数量: " << cashPledge;
		return false;
	}

	bool GameRoom::updateCashPledge(const std::string& playerId, int64_t cashPledge) const {
		std::stringstream ss;
		std::string sql;
		ss << "select count(*) from `cash_pledge` where `player_id` = \"" << playerId << "\" and `venue_id` = \"" << getId() << "\"";
		sql = ss.str();
		std::shared_ptr<MysqlCountTask> countTask = std::make_shared<MysqlCountTask>(sql);
		MysqlPool::getSingleton().syncQuery(countTask);
		if (!countTask->getSucceed()) {
			ErrorS << "查询押金记录数量失败，场地Id: " << getId() << ", 玩家Id: " << playerId;
			return false;
		}
		int count = countTask->getCount();
		std::shared_ptr<MysqlCommonTask> task;
		ss.str("");
		if (cashPledge > 0) {
			if (count == 0) {
				ss << "insert into `cash_pledge`(`player_id`, `venue_id`, `amount`, `time`) values(\"" << playerId << "\", \"" << getId() << "\", " << cashPledge << ", now())";
				sql = ss.str();
				task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Insert);
			}
			else {
				ss << "update `cash_pledge` set `amount` = " << cashPledge << ", `time` = now() where `player_id` = \"" << playerId << "\" and `venue_id` = \"" << getId() << "\"";
				sql = ss.str();
				task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			}
		}
		else {
			if (count > 0) {
				ss << "delete from `cash_pledge` where `player_id` = \"" << playerId << "\" and `venue_id` = \"" << getId() << "\"";
				sql = ss.str();
				task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Delete);
			}
			else
				return true;
		}
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			if (cashPledge > 0) {
				ErrorS << "更新押金数量失败，场地Id: " << getId() << ", 玩家Id: " << playerId;
			}
			else {
				ErrorS << "删除押金记录失败，场地Id: " << getId() << ", 玩家Id: " << playerId;
			}
			return false;
		}
		return true;
	}

	bool GameRoom::deductDiamond(const std::string& playerId, int64_t& diamond) {
		std::stringstream ss;
		std::string sql;
		// 循环多次以应对可能的数据库冲突的情况
		for (int i = 0; i < 10; i++) {
			std::shared_ptr<GetCapitalTask> task1 = std::make_shared<GetCapitalTask>(playerId);
			MysqlPool::getSingleton().syncQuery(task1);
			if (!task1->getSucceed() || (task1->getRows() < 1)) {
				ErrorS << "扣除钻石失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 查询失败";
				return false;
			}
			if (_diamondNeed == 0) {
				diamond = task1->getDiamond();
				return true;
			}
			if (_diamondNeed > task1->getDiamond()) {
				ErrorS << "扣除钻石失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 数量不足";
				return false;
			}
			diamond = task1->getDiamond() - _diamondNeed;
			ss.str("");
			ss << "update `capital` set `diamond` = " << diamond
				<< ", `version` = `version` + 1 where `player_id` = \""
				<< playerId << "\" and `version` = " << task1->getVersion();
			sql = ss.str();
			std::shared_ptr<MysqlCommonTask> task2 = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			MysqlPool::getSingleton().syncQuery(task2);
			if (!task2->getSucceed()) {
				ErrorS << "扣除钻石失败，场地Id: " << getId() << ", 玩家Id: " << playerId << ", 更新失败";
				return false;
			}
			if (task2->getAffectedRecords() > 0)
				return true;
		}
		return false;
	}

	void GameRoom::rewardAgency(const std::string& playerId, double reward, int64_t winGold) {
		if (reward < 1.0)
			return;
		class RewardAgencyTask {
		public:
			RewardAgencyTask() {}
			virtual ~RewardAgencyTask() {}

		public:
			std::string _venueId;
			std::string _playerId;
			int _gameType = 0;
			double _reward = 0.0;
			int64_t _winGold = 0LL;
		};
		// 在异步线程中执行代理奖励任务
		std::shared_ptr<RewardAgencyTask> task = std::make_shared<RewardAgencyTask>();
		task->_venueId = getId();
		task->_playerId = playerId;
		task->_gameType = getGameType();
		task->_winGold = winGold;
		TimerManager::getSingleton().addAsyncTimer(50, [task] {
			double reward = task->_reward;
			double part = 0.0;
			// 下取整
			int64_t amount = 0LL;
			// 间接奖励
			int indirect = 0;
			// 是否结束循环
			bool test = false;
			std::string playerId = task->_playerId;
			std::string agencyId1;
			std::string agencyId2;
			std::stringstream ss;
			std::string remark;
			std::string sql;
			ss << "玩家(Id:" << playerId << ")在游戏(类型: " << task->_gameType << ")中获利" << task->_winGold << "金币";
			remark = ss.str();
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			remark = boost::locale::conv::to_utf<char>(remark, std::string("gb2312"));
#endif

			while (true) {
				if (agencyId1.empty()) {
					std::shared_ptr<GetAgencyTask> task1 = std::make_shared<GetAgencyTask>(playerId);
					MysqlPool::getSingleton().syncQuery(task1);
					if (!task1->getSucceed() || (task1->getRows() < 1))
						break;
					agencyId1 = task1->getAgencyId();
					if (agencyId1.empty())
						break;
				}
				std::shared_ptr<GetAgencyTask> task2 = std::make_shared<GetAgencyTask>(agencyId1);
				MysqlPool::getSingleton().syncQuery(task2);
				if (task2->getSucceed() && (task2->getRows() > 0))
					agencyId2 = task2->getAgencyId();
				if (agencyId2.empty()) {
					// 再上一级代理(agencyId2)为空，本级代理(agencyId1)获得全部奖励
					amount = static_cast<int64_t>(reward);
					test = true;
				}
				else {
					// 再上一级代理agencyId2)能获得奖励的20%，本级代理(agencyId1)获得奖励的80%
					part = reward * 0.2;
					if (part < 1.0) {
						// 不够再上一级代理agencyId2)分配，本级代理(agencyId1)获得全部奖励
						test = true;	
					}
					else
						reward -= part;
					amount = static_cast<int64_t>(reward);
					reward = part;
				}
				ss.str("");
				ss << "insert into `agency_reward` (`player_id`, `amount`, `junior_id`, `indirect`, `venue_id`, `remark`,  `time`) values (\""
					<< agencyId1 << "\", " << amount << ", \"" << playerId << "\", " << indirect << ", \"" << task->_venueId << "\", \"" << remark
					<< "\", now())";
				sql = ss.str();
				std::shared_ptr<MysqlCommonTask> task3 = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Insert);
				MysqlPool::getSingleton().syncQuery(task3);
				if (!task3->getSucceed() || (task3->getAffectedRecords() < 1)) {
					ss.str("");
					ss << "添加代理奖励失败，代理玩家Id: " << agencyId1 << ", 数量: " << amount << ", 下级玩家Id: " << playerId
						<< ", 间接奖励: " << indirect << ", 场地Id: " << task->_venueId << ", 说明: " << remark;
					LOG_ERROR(ss.str());
				}
				if (test)
					break;
				indirect = 1;
				playerId = agencyId1;
				agencyId1 = agencyId2;
				agencyId2.clear();
			}
			return true;
		});
	}

	void GameRoom::kickAvatar(const GameAvatar::Ptr& avatar) {
		if (!avatar)
			return;
		// 返还玩家押金
		if (returnCashPledge(avatar->getPlayerId()))
			avatar->setCashPledge(0LL);
		int seat = -1;
		if (RoomCategory::RoomCategoryA == _category) {
			seat = avatar->getSeat();
			_avatarSeats[seat].reset();
		}
		removeAvatar(avatar->getPlayerId());
		// 离开后续处理
		afterLeave(avatar->getPlayerId());
		// 通知玩家离开
		notifyRemoveAvatar(seat, avatar->getPlayerId());
		// 执行玩家离开后的上层逻辑
		onAvatarLeaved(seat, avatar->getPlayerId());
	}

	void GameRoom::kickAllAvatars() {
		for (int i = 0; i < _maxPlayerNums; i++)
			_avatarSeats[i].reset();
		std::vector<std::string> playerIds;
		std::unordered_map<std::string, GameAvatar::Ptr>::iterator it = _allAvatars.begin();
		while (it != _allAvatars.end()) {
			GameAvatar::Ptr& avatar = it->second;
			playerIds.push_back(avatar->getPlayerId());
			// 返还玩家押金
			if (returnCashPledge(avatar->getPlayerId()))
				avatar->setCashPledge(0LL);
			it++;
		}
		_allAvatars.clear();
		for (const std::string& playerId : playerIds) {
			// 离开后续处理
			afterLeave(playerId);
		}
		// 执行玩家离开后的上层逻辑
		onAvatarLeaved(0, BaseUtils::EMPTY_STRING);
	}

	void GameRoom::sendMessage(const MsgBase& msg, const std::string& playerId) const {
		GameAvatar::Ptr avatar = getAvatar(playerId);
		if (avatar)
			msg.send(avatar->getSession());
	}

	void GameRoom::sendMessageToAll(const MsgBase& msg, const std::string& playerExcepted, bool spectator) const {
		std::shared_ptr<std::string> data;
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it1;
		for (it1 = _allAvatars.begin(); it1 != _allAvatars.end(); ++it1) {
			if (playerExcepted == it1->first)
				continue;
			Session::Ptr session = (it1->second)->getSession();
			if (session) {
				if (!data) {
					data = msg.pack();
					if (!data) {
						ErrorS << "Pack message(type: \"" << msg.getType() << "\") failed, check whether the \"MSG_PACK_IMPL\" macro has been added to the message declaration.";
						return;
					}
				}
				session->send(data);
			}
		}
		if (spectator) {
			std::unordered_set<std::string>::const_iterator it2;
			for (it2 = _spectators.begin(); it2 != _spectators.end(); ++it2) {
				if (playerExcepted == *it2)
					continue;
				Player::Ptr player = PlayerManager::getSingleton().getPlayer(*it2);
				if (!player)
					continue;
				Session::Ptr session = player->getSession();
				if (session) {
					if (!data) {
						data = msg.pack();
						if (!data) {
							ErrorS << "Pack message(type: \"" << msg.getType() << "\") failed, check whether the \"MSG_PACK_IMPL\" macro has been added to the message declaration.";
							return;
						}
					}
					session->send(data);
				}
			}
		}
	}

	void GameRoom::onGetAvatars(const NetMessage::Ptr& netMsg) {
		MsgGetAvatars* inst = dynamic_cast<MsgGetAvatars*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if (!hasPlayer(inst->getPlayerId()))
			return;
		sendAvatars(netMsg->getSession());
	}

	void GameRoom::sendAvatars(const Session::Ptr& session) const {
		if (!session)
			return;
		AvatarInfo info;
		MsgAddAvatar msg;
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it;
		for (it = _allAvatars.begin(); it != _allAvatars.end(); ++it) {
			const GameAvatar::Ptr& avatar = it->second;
			getAvatarInfo(info, avatar);
			getAvatarExtraInfo(avatar, info.base64);
			msg.avatars.push_back(info);
		}
		msg.send(session);
	}

	void GameRoom::onGetSpectators(const NetMessage::Ptr& netMsg) {
		MsgGetSpectators* inst = dynamic_cast<MsgGetSpectators*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if (!hasPlayer(inst->getPlayerId()))
			return;
		
		SpectatorInfo info;
		MsgAddSpectator msg;
		std::unordered_set<std::string>::const_iterator it;
		for (it = _spectators.begin(); it != _spectators.end(); ++it) {
			Player::Ptr player = PlayerManager::getSingleton().getPlayer(*it);
			if (!player)
				continue;
			info.playerId = *it;
			info.nickname = player->getNickname();
			info.headUrl = player->getAvatar();
			getSpectatorExtraInfo(*it, info.base64);
			msg.spectators.push_back(info);
		}
		msg.send(netMsg->getSession());
	}

	void GameRoom::onPlayerGeolocation(const NetMessage::Ptr& netMsg) {
		MsgPlayerGeolocation* inst = dynamic_cast<MsgPlayerGeolocation*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GameAvatar::Ptr avatar = getAvatar(inst->getPlayerId());
		if (!avatar)
			return;
		double* distance = getDistances();
		if (distance == nullptr)
			return;
		avatar->setGeolocation(inst->latitude, inst->longitude, inst->altitude);
		calcDistances(avatar->getSeat(), distance);
	}

	void GameRoom::onDistancesRequest(const NetMessage::Ptr& netMsg) {
		MsgGetDistancesResp msg;
		getDistances(msg.distances);
		msg.send(netMsg->getSession());
	}

	void GameRoom::calcDistances(int seat, double* distances) {
		int idx = 0;
		double lat1 = 0.0f;
		double lon1 = 0.0f;
		double alt1 = 0.0f;
		double lat2 = 0.0f;
		double lon2 = 0.0f;
		double alt2 = 0.0f;
		double dist = 0.0f;
		GameAvatar::Ptr avatar1 = getAvatar(seat);
		GameAvatar::Ptr avatar2;
		avatar1->getGeolocation(lat1, lon1, alt1);
		if (BaseUtils::real64Equals(lat1, 0.0) &&
			BaseUtils::real64Equals(lon1, 0.0) &&
			BaseUtils::real64Equals(alt1, 0.0))
			return;
		for (int i = 0; i < _maxPlayerNums; i++) {
			if (i == seat)
				continue;
			idx = getDistanceIndex(i, seat);
			if (idx > 5) {
				LOG_ERROR("calcDistances找不到索引");
				continue;
			}
			distances[idx] = -1;
			avatar2 = getAvatar(i);
			if (!avatar2)
				continue;
			avatar2->getGeolocation(lat2, lon2, alt2);
			if (BaseUtils::real64Equals(lat2, 0.0) &&
				BaseUtils::real64Equals(lon2, 0.0) &&
				BaseUtils::real64Equals(alt2, 0.0))
				continue;
			distances[idx] = BaseUtils::calcGeoDistance(lat1, lon1, alt1, lat2, lon2, alt2);
		}
	}

	void GameRoom::clearDistances(int seat, double* distances) {
		int idx = 0;
		for (int i = 0; i < _maxPlayerNums; i++) {
			if (i == seat)
				continue;
			idx = getDistanceIndex(i, seat);
			if (idx == -1) {
				LOG_ERROR("clearDistances找不到索引");
				continue;
			}
			distances[idx] = -1.0;
		}
	}

	double* GameRoom::getDistances() {
		return nullptr;
	}

	void GameRoom::getDistances(std::vector<int>& distances) const {}

	int GameRoom::getDistanceIndex(int seat1, int seat2) const {
		return -1;
	}

	void GameRoom::onChatClient(const NetMessage::Ptr& netMsg) {
		MsgChatClient* inst = dynamic_cast<MsgChatClient*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		
		MsgChatServer msg;
		if (getCategory() == RoomCategory::RoomCategoryA) {
			GameAvatar::Ptr avatar = getAvatar(inst->getPlayerId());
			if (avatar)
				msg.seat = avatar->getSeat();
		}
		msg.type = inst->type;
		msg.index = inst->index;
		msg.text = inst->text;
		msg.playerId = inst->getPlayerId();
		sendMessageToAll(msg);
	}

	void GameRoom::onEffectClient(const NetMessage::Ptr& netMsg) {
		MsgEffectClient* inst = dynamic_cast<MsgEffectClient*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GameAvatar::Ptr avatar1 = getAvatar(inst->getPlayerId());
		GameAvatar::Ptr avatar2 = getAvatar(inst->targetId);
		if (!avatar1 || !avatar2)
			return;

		MsgEffectServer msg;
		msg.index = inst->index;
		msg.srcSeat = avatar1->getSeat();
		msg.dstSeat = avatar2->getSeat();
		sendMessageToAll(msg);
	}

	void GameRoom::onVoiceClient(const NetMessage::Ptr& netMsg) {
		MsgVoiceClient* inst = dynamic_cast<MsgVoiceClient*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GameAvatar::Ptr avatar = getAvatar(inst->getPlayerId());
		if (!avatar)
			return;
		MsgVoiceServer msg;
		msg.seat = avatar->getSeat();
		msg.playerId = inst->getPlayerId();
		msg.base64 = inst->base64;
		sendMessageToAll(msg, inst->getPlayerId());
	}

	void GameRoom::clean() {}
}