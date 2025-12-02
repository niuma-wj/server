// GuanDanRoom.cpp

#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "Game/GameMessages.h"
#include "Game/GetCapitalTask.h"
#include "Player/PlayerManager.h"
#include "MySql/MysqlPool.h"
#include "../GameDefines.h"
#include "PokerUtilities.h"
#include "GuanDanAvatar.h"
#include "GuanDanRoom.h"
#include "GuanDanRule.h"
#include "GuanDanMessages.h"
#include "Constant/RedisKeys.h"
#include "Redis/RedisPool.h"

#include "jsoncpp/include/json/json.h"

#include <mysql/jdbc.h>
#include <sstream>
#include <boost/locale.hpp>

namespace NiuMa
{
	GuanDanRoom::GuanDanRoom(const std::string& venueId, const std::string& number, int lvl)
		: GameRoom(venueId, static_cast<int>(GameType::GuanDan), 4)
		, _number(number)
		, _ownerSeat(-1)
		, _level(lvl)
		, _roundNo(0)
		, _gameState(GameState::Sitting)
		, _tribute(0)
		, _current(-1)
		, _lastOut(-1)
		, _gradePointRed(static_cast<int>(PokerPoint::Two))
		, _gradePointBlue(static_cast<int>(PokerPoint::Two))
		, _banker(0)
		, _bankerNext(0)
		, _gradePointNext(static_cast<int>(PokerPoint::Two))
		, _disbanding(false)
		, _disbander(-1)
		, _lastRegisterTick(0LL)
		, _waitTick(0LL)
		, _autoTick(0LL)
		, _beginDisbandTick(0LL)
		, _endDisbandTick(0LL)
	{
		// 练习房没有入座状态
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice))
			_gameState = GameState::Waiting;
		_rule = std::make_shared<GuanDanRule>();
		_dealer = std::make_shared<PokerDealer>(_rule);
		for (int i = 0; i < 4; i++) {
			_lastFinishedSeats[i] = -1;
			_finishedSeats[i] = -1;
			_kicks[i] = false;
			_disbandChoises[i] = 0;
		}
		if (_level != static_cast<int>(GuanDanRoomLevel::Practice))
			setDiamondNeed(2);
	}

	GuanDanRoom::~GuanDanRoom()
	{}

	void GuanDanRoom::onTimer() {
		GameRoom::onTimer();

		time_t nowTick = BaseUtils::getCurrentMillisecond();
		time_t deltaTicks = 0;
		if ((_level != static_cast<int>(GuanDanRoomLevel::Practice)) &&
			(_level != static_cast<int>(GuanDanRoomLevel::Friend))) {
			deltaTicks = 5001;
			if (_lastRegisterTick != 0LL)
				deltaTicks = nowTick - _lastRegisterTick;
			if (deltaTicks > 5000) {
				// 每5秒刷新一次注册项
				_lastRegisterTick = nowTick;
				int districtId = getDistrictId();
				std::string redisKey = RedisKeys::DISTRICT_VENUE_REGISTER + std::to_string(districtId);
				RedisPool::getSingleton().hset(redisKey, getId(), std::to_string(nowTick));
			}
		}
		if (_gameState == GameState::Waiting) {
			if ((_level != static_cast<int>(GuanDanRoomLevel::Practice)) &&
				(_level != static_cast<int>(GuanDanRoomLevel::Friend)))
				checkOffline(60000);
			if (_level == static_cast<int>(GuanDanRoomLevel::Practice))
				addRobot(nowTick);
			return;
		}
		if (_disbanding) {
			deltaTicks = nowTick - _beginDisbandTick;
			if (deltaTicks > 300000) {
				// 超时不选择默认同意解散
				for (int i = 0; i < getMaxPlayerNums(); i++) {
					if (_disbandChoises[i] == 0)
						doDisbandChoose(i, 1);
				}
			}
			return;
		}
		deltaTicks = getWaitElapsed(nowTick);
		if (_gameState == GameState::Dealing) {
			// 发牌等待2秒客户端播放发牌动画
			if (deltaTicks < 2000)
				return;
			beginPlay();
		}
		else if (_gameState == GameState::Playing) {
			if (deltaTicks < 2000)
				return;
			autoExecute(nowTick);
		}
	}

	bool GuanDanRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (GameRoom::onMessage(netMsg))
			return true;

		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgGuanDanSync::TYPE)
			onSyncGuanDan(netMsg);
		else if (msgType == MsgPlayerReady::TYPE)
			onPlayerReady(netMsg);
		else if (msgType == MsgPlayerAuthorize::TYPE)
			onPlayerAuthorize(netMsg);
		else if (msgType == MsgGuanDanStartGame::TYPE)
			onStartGame(netMsg);
		else if (msgType == MsgPresentTribute::TYPE)
			onPresentTribute(netMsg);
		else if (msgType == MsgRefundTribute::TYPE)
			onRefundTribute(netMsg);
		else if (msgType == MsgGuanDanDoPlayCard::TYPE)
			onPlayCard(netMsg);
		else if (msgType == MsgGuanDanHintCard::TYPE)
			onHintCard(netMsg);
		else if (msgType == MsgGuanDanResetHintCard::TYPE)
			onResetHintCard(netMsg);
		else if (msgType == MsgGuanDanHintStraightFlush::TYPE)
			onHintStraightFlush(netMsg);
		else if (msgType == MsgDisbandRequest::TYPE)
			onDisbandRequest(netMsg);
		else if (msgType == MsgDisbandChoose::TYPE)
			onDisbandChoose(netMsg);
		else
			ret = false;

		return ret;
	}

	GameAvatar::Ptr GuanDanRoom::createAvatar(const std::string& playerId, int seat, bool robot) const {
		std::shared_ptr<GuanDanAvatar> avatar = std::make_shared<GuanDanAvatar>(_rule, playerId, seat, robot);
		if (robot)
			avatar->setReady(true);
		return avatar;
	}

	bool GuanDanRoom::checkEnter(const std::string& playerId, std::string& errMsg, bool robot) const {
		// 座位满后不运行再进入房间
		if (isFull()) {
			errMsg = "游戏已满员";
			return false;
		}
		if ((_gameState != GameState::Sitting) && (_gameState != GameState::Waiting)) {
			// 理论上不会执行到这里
			LOG_ERROR("error");
			errMsg = "游戏进行中，不能进入房间";
			return false;
		}
		return true;
	}

	int GuanDanRoom::checkLeave(const std::string& playerId, std::string& errMsg) const {
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice)) {
			// 练习房随时可以退出
			return 0;
		}
		if ((_gameState != GameState::Sitting) && (_gameState != GameState::Waiting)) {
			if (_level != static_cast<int>(GuanDanRoomLevel::Friend)) {
				errMsg = "非好友房不能中途解散";
				return 2;
			}
			else {
				errMsg = "游戏正在进行中，不能离开房间";
				return 1;
			}
		}
		return 0;
	}

	bool GuanDanRoom::enableSpectator() const {
		// 练习房不允许观众存在，玩家一进入就立即坐下
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice))
			return false;
		// 入座状态时才允许观众存在
		if (_gameState != GameState::Sitting)
			return false;
		return true;
	}

	void GuanDanRoom::onSpectatorLeaved(const std::string& playerId) {
		if (_level != static_cast<int>(GuanDanRoomLevel::Friend))
			return;
		int count = getAvatarCount() + getSpectatorCount();
		if (count == 0) {
			// 既没有玩家也没有观众，关闭游戏房间
			if (_roundNo == 0)
				removeRoomFromDb();
			gameOver(_roundNo == 0);
		}
	}

	void GuanDanRoom::onAvatarJoined(int seat, const std::string& playerId) {
		GameRoom::onAvatarJoined(seat, playerId);
		initScoreboard(playerId);
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice)) {
			if (seat == 0) {
				// 玩家进入，加载机器人
				_waitTick = BaseUtils::getCurrentMillisecond();
			}
			if (isFull() && isAllReady())
				beginDeal();
			return;
		}
		if (_level == static_cast<int>(GuanDanRoomLevel::Friend)) {
			if (_ownerSeat == -1) {
				_ownerSeat = seat;
				// 通知房主座位号变更
				notifyOwnerSeat();
			}
		}
		// 更新区域内场地的玩家数量
		int districtId = getDistrictId();
		std::string redisKey = RedisKeys::DISTRICT_NOT_FULL_VENUES + std::to_string(districtId);
		if (isFull())
			RedisPool::getSingleton().hdel(redisKey, getId());
		else {
			int count = getAvatarCount();
			RedisPool::getSingleton().hset(redisKey, getId(), count);
		}
	}

	void GuanDanRoom::onAvatarLeaved(int seat, const std::string& playerId) {
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice)) {
			if ((seat == 0) && !playerId.empty()) {
				// 练习房玩家离开，而不是机器人离开
				_gameState = GameState::Waiting;
				kickAllAvatars();
				removeRoomFromDb();
				gameOver(true);
			}
			return;
		}
		int count = getAvatarCount();
		if (_gameState != GameState::Sitting) {
			// 房间当前处于游戏状态，有玩家离开后立即进入入座状态
			_gameState = GameState::Sitting;
			// 通知客户端当前进入入座状态
			notifySitting();
		}
		if (_level == static_cast<int>(GuanDanRoomLevel::Friend)) {
			if (count == 0) {
				_ownerSeat = -1;
				if (getSpectatorCount() == 0) {
					// 既没有玩家也没有观众，关闭游戏房间
					if (_roundNo == 0)
						removeRoomFromDb();
					gameOver(_roundNo == 0);
					return;
				}
			}
			else if (_ownerSeat == seat) {
				int tmp = 0;
				for (int i = 0; i < 4; i++) {
					tmp = (_ownerSeat + i) % 4;
					GameAvatar::Ptr avatar = getAvatar(tmp);
					if (avatar) {
						_ownerSeat = tmp;
						break;
					}
				}
			}
			// 通知房主座位号变更
			notifyOwnerSeat();
			return;
		}
		else if (count == 0) {
			// 场地房间所有玩家都离开后，重新从第一局打起
			_roundNo = 0;
		}
		// 更新区域内场地的玩家数量
		int districtId = getDistrictId();
		std::string redisKey = RedisKeys::DISTRICT_NOT_FULL_VENUES + std::to_string(districtId);
		RedisPool::getSingleton().hset(redisKey, getId(), count);

		// 记录玩家的进入场地轨迹
		redisKey = RedisKeys::DISTRICT_PLAYER_TRACK;
		std::string::size_type pos = redisKey.find("{0}");
		redisKey.replace(pos, 3, std::to_string(districtId));
		pos = redisKey.find("{1}");
		redisKey.replace(pos, 3, playerId);
		RedisPool::getSingleton().hset(redisKey, getId(), BaseUtils::getCurrentMillisecond());
	}

	void GuanDanRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {
		int64_t gold = avatar->getCashPledge() + avatar->getGold();
		Json::Value tmp(Json::objectValue);
		tmp["gold"] = static_cast<Json::Int64>(gold);
		if (!avatar->isOffline()) {
			Session::Ptr session = avatar->getSession();
			if (session)
				tmp["ip"] = session->getRemoteIp();
		}
		tmp["authorize"] = avatar->isAuthorize();
		int winNum = 0;
		int loseNum = 0;
		int drawNum = 0;
		avatar->getScoreboard(winNum, loseNum, drawNum);
		tmp["winNum"] = winNum;
		tmp["loseNum"] = loseNum;
		tmp["drawNum"] = drawNum;
		std::string json = tmp.toStyledString();
		BaseUtils::encodeBase64(base64, json.data(), static_cast<int>(json.size()));
	}

	void GuanDanRoom::clean() {
		GameRoom::clean();

		_tribute = 0;
		_current = -1;
		_lastOut = -1;
		_operation = WaitOperation::None;
		_autoTick = 0LL;
		_presentTributeTip.clear();
		_refundTributeTip.clear();
		_presentTributeCards.clear();
		_refundPlayers = 0;
		for (int i = 0; i < 4; i++) {
			_lastFinishedSeats[i] = _finishedSeats[i];
			_finishedSeats[i] = -1;
			_kicks[i] = false;
			_disbandChoises[i] = 0;
		}
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar != nullptr)
				avatar->clear();
		}
	}

	int GuanDanRoom::getDistrictId() const {
		if (_level == static_cast<int>(GuanDanRoomLevel::Beginner))
			return 5;
		if (_level == static_cast<int>(GuanDanRoomLevel::Moderate))
			return 6;
		if (_level == static_cast<int>(GuanDanRoomLevel::Advanced))
			return 7;
		if (_level == static_cast<int>(GuanDanRoomLevel::Master))
			return 8;
		return 0;
	}

	int GuanDanRoom::getWaitElapsed(time_t nowTick) const {
		int deltaTicks = 0;
		if (_disbanding) {
			// 正在投票解散，从开始投票解散之后所有游戏逻辑都停止计时
			deltaTicks = static_cast<int>(_beginDisbandTick - _waitTick);
			return deltaTicks;
		}
		if (nowTick == 0LL)
			nowTick = BaseUtils::getCurrentMillisecond();
		// 跨过中间投票解散的时间端
		if (_endDisbandTick > _waitTick)
			deltaTicks = static_cast<int>(nowTick - _endDisbandTick + _beginDisbandTick - _waitTick);
		else
			deltaTicks = static_cast<int>(nowTick - _waitTick);
		return deltaTicks;
	}

	void GuanDanRoom::addRobot(time_t nowTick) {
		GameAvatar::Ptr avatar = getAvatar(0);
		if (!avatar)
			return;
		int deltaTicks = getWaitElapsed(nowTick);
		if (deltaTicks < 1000)
			return;
		_waitTick = nowTick;
		int count = PlayerManager::getSingleton().getRobotCount();
		if (count == 0)
			return;
		int loop = 0;
		int robotId = 0;
		std::string playerId;
		while (true) {
			robotId = BaseUtils::randInt(0, count) + 1;
			if (!PlayerManager::getSingleton().loadRobot(robotId, playerId))
				return;
			if (joinRobot(playerId))
				return;
			loop++;
			// 避免因数据错误造成死循环
			if (loop > 3)
				return;
		}
	}

	void GuanDanRoom::onPlayerReady(const NetMessage::Ptr& netMsg) {
		if (!(_gameState == GameState::Sitting || _gameState == GameState::Waiting))
			return;
		MsgPlayerReady* inst = dynamic_cast<MsgPlayerReady*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		avatar->setReady(true);
		MsgPlayerReadyResp msg;
		msg.playerId = inst->getPlayerId();
		msg.seat = avatar->getSeat();
		sendMessageToAll(msg, BaseUtils::EMPTY_STRING, true);

		if (isFull() && isAllReady()) {
			// 所有人都已经准备好
			bool test = false;
			// 好友房需要等待房主点击开始游戏才从入座状态进入到游戏状态，并开始发牌
			if (_gameState == GameState::Waiting)
				test = true;
			else if ((_gameState == GameState::Sitting) && (_level != static_cast<int>(GuanDanRoomLevel::Friend))) {
				// 进入游戏状态并开始发牌
				_gameState = GameState::Dealing;
				// 通知客户端进入游戏状态，观众在收到消息后必须离开房间
				notifySitting();
				// 踢出全部观众
				kickAllSpectators();
				test = true;
			}
			if (test)
				beginDeal();
		}
	}

	void GuanDanRoom::onPlayerAuthorize(const NetMessage::Ptr& netMsg) {
		MsgPlayerAuthorize* inst = dynamic_cast<MsgPlayerAuthorize*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->isAuthorize())
			avatar->setAuthorize(false);
		else
			avatar->setAuthorize(true);
		notifyPlayerAuthorize(avatar);
	}

	void GuanDanRoom::onStartGame(const NetMessage::Ptr& netMsg) {
		MsgGuanDanStartGame* inst = dynamic_cast<MsgGuanDanStartGame*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		MsgGuanDanStartGameResp resp;
		if (_gameState != GameState::Sitting) {
			resp.result = 1;
			BaseUtils::toUtf8(std::string("当前状态不能开始游戏"), resp.errMsg);
			resp.send(netMsg->getSession());
			return;
		}
		if (_level != static_cast<int>(GuanDanRoomLevel::Friend)) {
			resp.result = 2;
			BaseUtils::toUtf8(std::string("非好友房不能开始游戏"), resp.errMsg);
			resp.send(netMsg->getSession());
			return;
		}
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if ((avatar == nullptr) || (avatar->getSeat() != _ownerSeat)) {
			resp.result = 3;
			BaseUtils::toUtf8(std::string("非房主不能开始游戏"), resp.errMsg);
			resp.send(netMsg->getSession());
			return;
		}
		if (!(isFull() && isAllReady())) {
			resp.result = 4;
			BaseUtils::toUtf8(std::string("玩家未满或未全部就绪"), resp.errMsg);
			resp.send(netMsg->getSession());
			return;
		}
		resp.send(netMsg->getSession());
		// 通知客户端进入游戏状态，观众在收到消息后必须离开房间
		_gameState = GameState::Dealing;
		notifySitting();
		if (_gameState != GameState::Sitting) {
			// 进入游戏状态，踢出全部观众
			kickAllSpectators();
		}
		// 开始发牌
		beginDeal();
	}

	void GuanDanRoom::notifyOwnerSeat() const {
		MsgGuanDanOwnerSeat msg;
		msg.ownerSeat = _ownerSeat;
		sendMessageToAll(msg, BaseUtils::EMPTY_STRING, true);
	}

	void GuanDanRoom::notifySitting() const {
		MsgGuanDanSitting msg;
		msg.gameState = static_cast<int>(_gameState);
		sendMessageToAll(msg, BaseUtils::EMPTY_STRING, true);
	}

	void GuanDanRoom::notifyPlayerAuthorize(GuanDanAvatar* avatar) const {
		MsgPlayerAuthorizeResp msg;
		msg.seat = avatar->getSeat();
		msg.authorize = avatar->isAuthorize();
		sendMessageToAll(msg, BaseUtils::EMPTY_STRING, true);
	}

	/**
	 * 判断手牌中是否包含可用于还贡的牌，即除级牌外2~10的牌
	 * @param cards 手牌数组，已从小到大排序
	 * @return 包含-true，不包含-false
	 */
	bool GuanDanRoom::canRefundTribute(const CardArray& cards) {
		bool ret = false;
		int orderUpper = _rule->getPointOrder(static_cast<int>(PokerPoint::Ten));
		int order = -1;
		for (const PokerCard& c : cards) {
			if (_rule->getGradePoint() == c.getPoint())
				continue;
			order = _rule->getPointOrder(c.getPoint());
			ret = (order <= orderUpper);
			break;
		}
		return ret;
	}

	void GuanDanRoom::beginDeal() {
		_gameState = GameState::Dealing;
		_waitTick = BaseUtils::getCurrentMillisecond();

		clean();

		getRoundNo();

		// 扣除钻石
		deductDiamond();

		_rule->setGradePoint(_gradePointNext);
		if (_bankerNext == 1)
			_gradePointRed = _gradePointNext;
		else if (_bankerNext == 2)
			_gradePointBlue = _gradePointNext;
		_banker = _bankerNext;

		int maxPlayerNums = getMaxPlayerNums();
		std::ostringstream os;
		os << "掼蛋牌桌(ID:" << getId() << "，局数:" << _roundNo << ")发牌，各玩家ID:";
		for (int i = 0; i < maxPlayerNums; i++) {
			if (i > 0)
				os << "、";
			os << getAvatar(i)->getPlayerId();
		}
		LOG_INFO(os.str());

		CardArray cards;
		CardComparator comp(_rule);
		GuanDanAvatar* avatar = nullptr;
		// 上局头游玩家座位号
		int first = -1;
		// 上局二游玩家座位号
		int second = -1;
		if (_roundNo > 1) {
			first = getLastFinishedSeat(0);
			second = getLastFinishedSeat(1);
			if (second != getFriendSeat(first))
				second = -1;
		}
		bool test = false;
		do {
			test = false;
			_dealer->shuffle();
			for (int i = 0; i < maxPlayerNums; i++) {
				avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
				if (avatar == nullptr)
					continue;
				// 每个玩家发27张牌
				_dealer->handOutCards(cards, 27, DealFilter::Ptr());
				// 按升序排序，即排序后cards中的元素是从小到大
				std::sort(cards.begin(), cards.end(), comp);
				if (i == first || i == second) {
					// 判断是否包含能用于还贡的牌，若无则重新发牌
					if (!canRefundTribute(cards)) {
						test = true;
						break;
					}
				}
				avatar->setCards(cards);
			}
		} while (test);
		// 通知发牌，客户端做发牌动画
		MsgGuanDanDealCard msg;
		sendMessageToAll(msg);
	}

	void GuanDanRoom::getRoundNo() {
		if (_roundNo == 0) {
			class GetMaxRoundNoTask : public MysqlQueryTask {
			public:
				GetMaxRoundNoTask(const std::string& venueId)
					: _venueId(venueId)
					, _maxRoundNo(0)
				{}

				virtual ~GetMaxRoundNoTask() {}

			public:
				virtual QueryType buildQuery(std::string& sql) override {
					std::stringstream ss;
					ss << "select max(`round_no`) from `game_guan_dan_round` where `venue_id` = \"" << _venueId << "\"";
					sql = ss.str();
					return QueryType::Select;
				}

				virtual int fetchResult(sql::ResultSet* res) override {
					int rows = 0;
					while (res->next()) {
						_maxRoundNo = res->getInt(1);
						rows++;
					}
					return rows;
				}

			public:
				const std::string _venueId;
				int _maxRoundNo;
			};
			std::shared_ptr<GetMaxRoundNoTask> task = std::make_shared<GetMaxRoundNoTask>(getId());
			MysqlPool::getSingleton().syncQuery(task);
			if (task->getSucceed() && task->getRows() > 0)
				_roundNo = task->_maxRoundNo;
		}
		_roundNo++;
	}

	void GuanDanRoom::deductDiamond() {
		// 练习房不需要扣除钻石
		if (_level == static_cast<int>(GuanDanRoomLevel::Practice))
			return;
		MsgPlayerDiamonds msg;
		int64_t diamond = 0;
		GameAvatar::Ptr avatar;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = getAvatar(i);
			if (!avatar)
				continue;
			GameRoom::deductDiamond(avatar->getPlayerId(), diamond);
			msg.seats.push_back(i);
			msg.diamonds.push_back(diamond);
		}
		sendMessageToAll(msg);
	}

	int GuanDanRoom::getLastFinishedSeat(int order) const {
		if (order < 0 || order > 3)
			return -1;
		return _lastFinishedSeats[order];
	}

	int GuanDanRoom::getLastFinishedOrder(int seat) const {
		if (seat < 0 || seat > 3)
			return -1;
		int ret = -1;
		for (int i = 0; i < 4; i++) {
			if (_lastFinishedSeats[i] == seat) {
				ret = i;
				break;
			}
		}
		return ret;
	}

	int GuanDanRoom::getFriendSeat(int seat) const {
		if (seat < 0 || seat > 3)
			return -1;
		return (seat + 2) % 4;
	}

	bool GuanDanRoom::isFriend(int seat1, int seat2) const {
		if (seat1 == seat2)
			return true;
		int tmp = getFriendSeat(seat1);
		return (tmp == seat2);
	}

	bool GuanDanRoom::isRedSeat(int seat) const {
		return (seat == 0 || seat == 2);
	}

	void GuanDanRoom::beginPlay() {
		_gameState = GameState::Playing;

		// 通知当前级牌
		notifyGradePoint(nullptr, true);
		// 发送每个玩家的手牌
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if ((avatar != nullptr) && !avatar->isRobot())
				sendHandCard(avatar);
		}
		// 判断是否需要进贡
		if (_roundNo > 1) {
			_tribute = 1;
			// 上局头游玩家座位号
			int first = getLastFinishedSeat(0);
			// 上局二游玩家座位号
			int second = getLastFinishedSeat(1);
			if (isFriend(first, second))
				_tribute = 2;	// 头游和二游是同一方，则另一方是双下需要双进贡
			if (_tribute == 1) {
				// 判断末游是否可以抗贡
				int seat = getLastFinishedSeat(3);
				avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
				int nums = PokerUtilities::getCardNums(avatar->getCards(), static_cast<int>(PokerPoint::Joker), static_cast<int>(PokerSuit::Big));
				if (nums < 2) {
					// 大王的张数小于2，需要进贡
					avatar->setWaitingPresentTribute(true);
					waitPresentTribute();
				}
				else {
					// 玩家拿了两张大王，提示玩家抗贡
					std::stringstream ss;
					std::string name;
					BaseUtils::fromUtf8(avatar->getNickname(), name);
					ss << "玩家【" << name << "】抗贡";
					MsgResistTribute msg;
					msg.tip = ss.str();
#ifdef _MSC_VER
					// VC环境下gb2312编码转utf8
					msg.tip = boost::locale::conv::to_utf<char>(msg.tip, std::string("gb2312"));
#endif
					msg.seat1 = seat;
					sendMessageToAll(msg);
					
					_tribute = 0;
				}
			}
			else if (_tribute == 2) {
				int seat1 = getLastFinishedSeat(2);
				int seat2 = getLastFinishedSeat(3);
				GuanDanAvatar* avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat1).get());
				int nums = PokerUtilities::getCardNums(avatar1->getCards(), static_cast<int>(PokerPoint::Joker), static_cast<int>(PokerSuit::Big));
				GuanDanAvatar* avatar2 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat2).get());
				if (nums < 2)
					nums += PokerUtilities::getCardNums(avatar2->getCards(), static_cast<int>(PokerPoint::Joker), static_cast<int>(PokerSuit::Big));
				if (nums < 2) {
					// 大王的张数小于2，需要进贡
					avatar1->setWaitingPresentTribute(true);
					avatar2->setWaitingPresentTribute(true);
					waitPresentTribute();
				}
				else {
					// 玩家拿了两张大王，提示玩家抗贡
					std::stringstream ss;
					std::string name1;
					std::string name2;
					BaseUtils::fromUtf8(avatar1->getNickname(), name1);
					BaseUtils::fromUtf8(avatar2->getNickname(), name2);
					ss << "玩家【" << name1 << "】和玩家【" << name2 << "】抗贡";
					MsgResistTribute msg;
					msg.tip = ss.str();
#ifdef _MSC_VER
					// VC环境下gb2312编码转utf8
					msg.tip = boost::locale::conv::to_utf<char>(msg.tip, std::string("gb2312"));
#endif
					msg.seat1 = seat1;
					msg.seat2 = seat2;
					sendMessageToAll(msg);

					_tribute = 0;
				}
			}
			if (_tribute == 0) {
				// 抗贡，由上局头游玩家先出牌
				_current = getLastFinishedSeat(0);
			}
		}
		else {
			// 首局随机一位玩家先出牌
			if (_level == static_cast<int>(GuanDanRoomLevel::Practice))
				_current = 0;
			else
				_current = static_cast<int>(BaseUtils::randInt(0, 4));
		}
		if (_tribute == 0) {
			// 等待玩家出牌
			_lastOut = _current;
			waitPlayCard();
		}
	}

	void GuanDanRoom::notifyGradePoint(GuanDanAvatar* avatarIn, bool realTime) {
		MsgGuanDanGradePoint msg;
		msg.gradePointRed = _gradePointRed;
		msg.gradePointBlue = _gradePointBlue;
		msg.banker = _banker;
		msg.realTime = realTime;
		if (avatarIn == nullptr)
			sendMessageToAll(msg);
		else
			msg.send(avatarIn->getSession());
	}

	void GuanDanRoom::sendHandCard(GuanDanAvatar* avatar) {
		Session::Ptr session = avatar->getSession();
		if (!session)
			return;
		MsgGuanDanHandCard msg;
		msg.cards = avatar->getCards();
		msg.gradePoint = _rule->getGradePoint();
		const PokerCard& cardIn = avatar->getCardIn();
		msg.contributeId = cardIn.getId();
		msg.send(session);
	}

	void GuanDanRoom::onSyncGuanDan(const NetMessage::Ptr& netMsg) {
		MsgGuanDanSync* inst = dynamic_cast<MsgGuanDanSync*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		int seat = -1;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar != nullptr) {
			seat = avatar->getSeat();
			//DebugS << "onSyncGuanDan: " << avatar->getPlayerId();
		}
		MsgGuanDanSyncResp resp;
		resp.number = _number;
		resp.level = _level;
		resp.ownerSeat = _ownerSeat;
		resp.gameState = static_cast<int>(_gameState);
		resp.seat = seat;
		resp.send(netMsg->getSession());
		if ((avatar == nullptr) && (_gameState != GameState::Sitting))
			return;
		// 发送全部玩家数据
		sendAvatars(netMsg->getSession());
		if (avatar == nullptr)
			return;
		if (_gameState == GameState::Playing) {
			// 通知当前级牌
			notifyGradePoint(avatar, false);
			// 发送手牌
			sendHandCard(avatar);
			if (_finishedSeats[0] != -1)
				notifyFinished(avatar);
			int deltaTicks = getWaitElapsed();
			if (_operation == WaitOperation::PresentTribute) {
				// 正在等待进贡
				notifyWaitPresentTribute(deltaTicks, avatar);
			}
			else if (_operation == WaitOperation::RefundTribute) {
				// 正在等待还贡
				notifyWaitRefundTribute(deltaTicks, avatar);
			}
			else if (_operation == WaitOperation::PlayCard) {
				// 正在等待出牌
				GuanDanAvatar* avatar1 = nullptr;
				for (int i = 0; i < getMaxPlayerNums(); i++) {
					avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
					if (avatar1 == nullptr)
						continue;
					if ((i != _current) && avatar1->isPlayed())
						notifyPlayCard(i, avatar, false);
					notifyCardNums(i, avatar);
				}
				notifyWaitPlayCard(deltaTicks, avatar);
			}
		}
		if (_disbanding)
			notifyDisbandVote(inst->getPlayerId());
	}

	void GuanDanRoom::makePresentTributeTip() {
		std::stringstream ss;
		int num = 0;
		ss << "等待玩家";
		std::string name;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (avatar->isWaitingPresentTribute()) {
				if (num > 0)
					ss << "和玩家【";
				else
					ss << "【";
				BaseUtils::fromUtf8(avatar->getNickname(), name);
				ss << name << "】";
				num++;
				if (num == 2)
					break;
			}
		}
		if (num == 0) {
			ErrorS << "掼蛋牌桌(ID: " << getId() << ", 局号: " << _roundNo << ")当前无玩家等待进贡";
			return;
		}
		ss << "进贡";
		_presentTributeTip = ss.str();
#ifdef _MSC_VER
		// VC环境下gb2312编码转utf8
		_presentTributeTip = boost::locale::conv::to_utf<char>(_presentTributeTip, std::string("gb2312"));
#endif
	}

	void GuanDanRoom::waitPresentTribute() {
		_operation = WaitOperation::PresentTribute;
		_waitTick = BaseUtils::getCurrentMillisecond();

		makePresentTributeTip();
		notifyWaitPresentTribute(0, nullptr);
	}

	void GuanDanRoom::notifyWaitPresentTribute(int elapsed, GuanDanAvatar* avatarIn) {
		MsgWaitPresentTribute msg;
		msg.tip = _presentTributeTip;
		msg.elapsed = elapsed;
		int cnt = 0;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if ((avatar == nullptr) || !(avatar->isWaitingPresentTribute()))
				continue;
			if (cnt == 0)
				msg.seat1 = i;
			else
				msg.seat2 = i;
			cnt++;
		}
		if (avatarIn == nullptr)
			sendMessageToAll(msg);
		else
			msg.send(avatarIn->getSession());
	}

	void GuanDanRoom::onPresentTribute(const NetMessage::Ptr& netMsg) {
		MsgPresentTribute* inst = dynamic_cast<MsgPresentTribute*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (!avatar->isWaitingPresentTribute())
			return;
		std::string errMsg;
		PokerCard card;
		if (!avatar->getCardById(inst->cardId, card))
			errMsg = "进贡失败，指定的牌不存在";
		if (errMsg.empty()) {
			if (card.getPoint() == _rule->getGradePoint() &&
				card.getSuit() == static_cast<int>(PokerSuit::Heart))
				errMsg = "进贡失败，不能进贡逢人配";
		}
		if (errMsg.empty()) {
			PokerCard tmp;
			PokerUtilities::getBiggestCard(avatar->getCards(), tmp, _rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
			if (tmp.getPoint() != card.getPoint())
				errMsg = "进贡失败，必须进贡最大的牌";
			else if (tmp.getPoint() == static_cast<int>(PokerPoint::Joker)) {
				if (tmp.getSuit() != card.getSuit())
					errMsg = "进贡失败，必须进贡最大的牌";
			}
		}
		if (!errMsg.empty()) {
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			errMsg = boost::locale::conv::to_utf<char>(errMsg, std::string("gb2312"));
#endif
			MsgPresentTributeResult msg;
			msg.errMsg = errMsg;
			msg.send(netMsg->getSession());
			return;
		}
		doPresentTribute(avatar, card);
	}

	void GuanDanRoom::doPresentTribute(GuanDanAvatar* avatar, const PokerCard& card) {
		avatar->setWaitingPresentTribute(false);
		avatar->setCardOut(card);
		avatar->removeCardById(card.getId());
		_presentTributeCards.emplace_back(std::make_pair(avatar->getSeat(), card));
		Session::Ptr session = avatar->getSession();
		if (session) {
			// 通知进贡成功
			MsgPresentTributeResult msg;
			msg.success = true;
			msg.cardId = card.getId();
			msg.send(session);
		}
		if (_tribute == static_cast<int>(_presentTributeCards.size())) {
			// 已全部进贡，开始等待还贡
			std::string text;
			std::stringstream ss;
			if (_tribute == 1) {
				// 头游玩家座位号
				int seat = getLastFinishedSeat(0);
				GuanDanAvatar* avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
				card.toString(text);
				std::string name;
				BaseUtils::fromUtf8(avatar->getNickname(), name);
				ss << "玩家【" << name << "】进贡" << text;
				text = ss.str();
				avatar1->setWaitingRefundTribute(true);
				avatar1->setRefundTributeSeat(avatar->getSeat());
				avatar1->setRefundTributeText(text);
				avatar1->setCardIn(card);
				avatar->setPresentTributeSeat(seat);
			}
			else {
				const std::pair<int, PokerCard>& pr1 = _presentTributeCards.front();
				const std::pair<int, PokerCard>& pr2 = _presentTributeCards.back();
				GuanDanAvatar* avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(pr1.first).get());
				GuanDanAvatar* avatar2 = dynamic_cast<GuanDanAvatar*>(getAvatar(pr2.first).get());
				int seat1 = 0;
				int seat2 = 0;
				int ret = _rule->compareCard(pr1.second, pr2.second);
				if (ret == 0) {
					// 两位玩家进贡的牌大小相同，按顺时针方向进贡
					seat1 = (pr1.first + 3) % 4;
					seat2 = (pr2.first + 3) % 4;
				}
				else {
					// 头游玩家座位号
					seat1 = getLastFinishedSeat(0);
					// 二游玩家座位号
					seat2 = getLastFinishedSeat(1);
					if (ret == 2) {
						// card1小于card2，交换两个座位号
						int temp = seat1;
						seat1 = seat2;
						seat2 = temp;
					}
				}
				GuanDanAvatar* avatar3 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat1).get());
				GuanDanAvatar* avatar4 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat2).get());
				pr1.second.toString(text);
				std::string name;
				BaseUtils::fromUtf8(avatar1->getNickname(), name);
				ss << "玩家【" << name << "】进贡" << text;
				text = ss.str();
				avatar3->setWaitingRefundTribute(true);
				avatar3->setRefundTributeSeat(pr1.first);
				avatar3->setRefundTributeText(text);
				avatar3->setCardIn(pr1.second);
				avatar1->setPresentTributeSeat(seat1);
				pr2.second.toString(text);
				ss.str("");
				BaseUtils::fromUtf8(avatar2->getNickname(), name);
				ss << "玩家【" << name << "】进贡" << text;
				text = ss.str();
				avatar4->setWaitingRefundTribute(true);
				avatar4->setRefundTributeSeat(pr2.first);
				avatar4->setRefundTributeText(text);
				avatar4->setCardIn(pr2.second);
				avatar2->setPresentTributeSeat(seat2);
			}
			waitRefundTribute();
		}
		else {
			// 还有一位玩家未进贡，再次发送等待进贡通知
			makePresentTributeTip();
			notifyWaitPresentTribute(getWaitElapsed(), nullptr);
		}
	}

	void GuanDanRoom::makeRefundTributeTip() {
		int num = 0;
		std::stringstream ss;
		ss << "等待玩家【";
		std::string name;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (avatar->isWaitingRefundTribute()) {
				if (num > 0)
					ss << "和玩家【";
				BaseUtils::fromUtf8(avatar->getNickname(), name);
				ss << name;
				ss << "】";
				num++;
				if (num == 2)
					break;
			}
		}
		if (num == 0) {
			ErrorS << "掼蛋牌桌(ID: " << getId() << ", 局号: " << _roundNo << ")当前无玩家等待还贡";
			return;
		}
		ss << "还贡";
		_refundTributeTip = ss.str();
#ifdef _MSC_VER
		// VC环境下gb2312编码转utf8
		_refundTributeTip = boost::locale::conv::to_utf<char>(_refundTributeTip, std::string("gb2312"));
#endif
	}

	void GuanDanRoom::waitRefundTribute() {
		_operation = WaitOperation::RefundTribute;
		_waitTick = BaseUtils::getCurrentMillisecond();

		makeRefundTributeTip();
		notifyWaitRefundTribute(0, nullptr);
	}

	void GuanDanRoom::notifyWaitRefundTribute(int elapsed, GuanDanAvatar* avatarIn) {
		MsgWaitRefundTribute msg;
		msg.elapsed = elapsed;
		int cnt = 0;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if ((avatar == nullptr) || !(avatar->isWaitingRefundTribute()))
				continue;
			if (cnt == 0)
				msg.seat1 = i;
			else
				msg.seat2 = i;
			cnt++;
		}
		Session::Ptr session;
		if (avatarIn == nullptr) {
			for (int i = 0; i < 4; i++) {
				avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
				if (avatar == nullptr)
					continue;
				session = avatar->getSession();
				if (!session)
					continue;
				if (avatar->isWaitingRefundTribute()) {
					msg.tip = avatar->getRefundTributeText();
					msg.cardIn = avatar->getCardIn();
				}
				else {
					msg.tip = _refundTributeTip;
					msg.cardIn = PokerCard();
				}
				msg.send(session);
			}
		}
		else {
			session = avatarIn->getSession();
			if (!session)
				return;
			if (avatarIn->isWaitingRefundTribute()) {
				msg.tip = avatarIn->getRefundTributeText();
				msg.cardIn = avatarIn->getCardIn();
			}
			else {
				msg.tip = _refundTributeTip;
				msg.cardIn = PokerCard();
			}
			msg.send(session);
		}
	}

	void GuanDanRoom::onRefundTribute(const NetMessage::Ptr& netMsg) {
		MsgRefundTribute* inst = dynamic_cast<MsgRefundTribute*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (!avatar->isWaitingRefundTribute())
			return;
		std::string errMsg;
		PokerCard card;
		if (!avatar->getCardById(inst->cardId, card))
			errMsg = "还贡失败，指定的牌不存在";
		if (errMsg.empty()) {
			int ret = _rule->compareCard(card, PokerCard(PokerPoint::Ten, PokerSuit::Spade));
			if (ret == 1)
				errMsg = "还贡失败，只能还贡10及以下的牌";
		}
		if (!errMsg.empty()) {
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			errMsg = boost::locale::conv::to_utf<char>(errMsg, std::string("gb2312"));
#endif
			MsgRefundTributeResult msg;
			msg.errMsg = errMsg;
			msg.send(netMsg->getSession());
			return;
		}
		doRefundTribute(avatar, card);
	}

	void GuanDanRoom::doRefundTribute(GuanDanAvatar* avatar, const PokerCard& card) {
		avatar->setWaitingRefundTribute(false);
		avatar->setRefundTributeText(BaseUtils::EMPTY_STRING);
		avatar->setCardOut(card);
		avatar->removeCardById(card.getId());
		avatar->addCard(avatar->getCardIn());
		avatar->sortCards();
		// 重新发送手牌
		sendHandCard(avatar);
		int seat = avatar->getRefundTributeSeat();
		GuanDanAvatar* avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
		if (avatar != nullptr) {
			std::stringstream ss;
			std::string text;
			std::string name;
			card.toString(text);
			BaseUtils::fromUtf8(avatar->getNickname(), name);
			ss << "玩家【" << name << "】还贡" << text;
			text = ss.str();
			avatar1->setCardIn(card);
			avatar1->addCard(card);
			avatar1->sortCards();
			// 重新发送手牌
			sendHandCard(avatar1);
			// 发送收到还贡提示
			sendTipText(text, avatar1->getPlayerId());
		}
		_refundPlayers++;
		Session::Ptr session = avatar->getSession();
		if (session) {
			// 通知还贡成功
			MsgRefundTributeResult msg;
			msg.success = true;
			msg.send(session);
		}
		if (_tribute == _refundPlayers) {
			// 全部完成还贡
			// 向所有玩家发送进/还贡流程结束通知
			MsgTributeComplete msg;
			sendMessageToAll(msg);
			seat = getLastFinishedSeat(0);
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
			if (avatar1 != nullptr) {
				// 上局头游玩家的贡牌玩家获得本局首轮出牌权
				_current = avatar1->getRefundTributeSeat();
				_lastOut = _current;
				// 开始等待出牌
				waitPlayCard();
			}
			else {
				ErrorS << "掼蛋牌桌(ID: " << getId() << ", 局号: " << _roundNo << ")上局头游玩家替身为null";
				return;
			}
		}
		else {
			// 还有一位玩家未还贡，再次发送等待还贡通知
			makeRefundTributeTip();
			notifyWaitRefundTribute(getWaitElapsed(), nullptr);
		}
	}

	void GuanDanRoom::waitPlayCard() {
		_operation = WaitOperation::PlayCard;
		_waitTick = BaseUtils::getCurrentMillisecond();

		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		// 分析所有出牌组合
		avatar->analyzeCombinations();
		// 判定当前是否需要抗牌
		int seat = 0;
		int nums = 0;
		int minNums = 100;
		int situation = 0;
		GuanDanAvatar* avatar1 = nullptr;
		for (int i = 0; i < 2; i++) {
			if (i == 0)
				seat = (_current + 1) % 4;
			else
				seat = (_current + 3) % 4;
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
			if (avatar1 == nullptr)
				continue;
			nums = avatar1->getCardNums();
			if (nums == 0)
				continue;
			if (minNums > nums)
				minNums = nums;
		}
		if (minNums < 4)
			situation = 2;
		else if (minNums < 9) {
			if (avatar->getCombinationNums() < 3)
				situation = 2;
			else
				situation = 1;
		}
		if (_lastOut == _current)
			avatar->candidateCombinations(situation);
		else {
			// 检索所有可出的候选组合
			GuanDanAvatar* avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_lastOut).get());
			if (avatar1 != nullptr)
				avatar->candidateCombinations(avatar1->getOuttedGenre(), situation);
		}
		// 通知玩家出牌
		notifyWaitPlayCard(0, nullptr);
	}

	void GuanDanRoom::notifyWaitPlayCard(int elapsed, GuanDanAvatar* avatarIn) const {
		MsgGuanDanWaitPlayCard msg;
		msg.elapsed = elapsed;
		msg.seat = _current;
		Session::Ptr session;
		if (avatarIn == nullptr) {
			GuanDanAvatar* avatar = nullptr;
			for (int i = 0; i < 4; i++) {
				avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
				if (avatar == nullptr)
					continue;
				session = avatar->getSession();
				if (!session)
					continue;
				if (i == _current) {
					if (_lastOut == _current) {
						msg.firstPlay = true;
						msg.canPlay = true;
					}
					else {
						msg.firstPlay = false;
						msg.canPlay = avatar->hasCandidate();
					}
				}
				else {
					msg.firstPlay = false;
					msg.canPlay = false;
				}
				msg.send(session);
			}
		}
		else {
			session = avatarIn->getSession();
			if (!session)
				return;
			if (avatarIn->getSeat() == _current) {
				if (_lastOut == _current) {
					msg.firstPlay = true;
					msg.canPlay = true;
				}
				else
					msg.canPlay = avatarIn->hasCandidate();
			}
			msg.send(session);
		}
	}

	void GuanDanRoom::onPlayCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::PlayCard)
			return;
		MsgGuanDanDoPlayCard* inst = dynamic_cast<MsgGuanDanDoPlayCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		if (inst->pass)
			doPass(avatar);
		else
			doPlayCard(avatar, inst->cardIds);
	}

	bool GuanDanRoom::doPlayCard(GuanDanAvatar* avatar, const std::vector<int>& ids, const PokerCombination::Ptr& comb) {
		GuanDanAvatar* avatar1 = nullptr;
		if (!avatar->hasCandidate()) {
			notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPlay), avatar);
			ErrorS << "掼蛋牌桌(ID: " << getId() << ")当前玩家(座位: " << _current << ")要不起";
			return false;
		}
		CardArray cards;
		if (!avatar->getCardsByIds(ids, cards)) {
			notifyPlayCardFailed(static_cast<int>(PlayCardFailed::NotFound), avatar);

			std::ostringstream os;
			os << "掼蛋牌桌(ID:" << getId() << ")当前玩家(座位:" << _current << ")出牌失败，找不到指定的牌(ID:";
			unsigned int nums = static_cast<unsigned int>(ids.size());
			for (unsigned int i = 0; i < nums; i++) {
				if (i > 0)
					os << "、";
				os << ids[i];
			}
			os << ")";
			LOG_ERROR(os.str());
			return false;
		}
		PokerGenre pg;
		if (comb) {
			pg.setCards(cards, _rule, comb->getGenre());
			pg.setOfficer(PokerCard(static_cast<PokerPoint>(comb->getOfficerPoint()), static_cast<PokerSuit>(comb->getOfficerSuit())));
		}
		else {
			pg.setCards(cards, _rule, -1);
			if (!_rule->isValidGenre(pg.getGenre())) {
				// 无效牌型
				notifyPlayCardFailed(static_cast<int>(PlayCardFailed::Invalid), avatar);
				return false;
			}
			if (_current != _lastOut) {
				int ret = 0;
				avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_lastOut).get());
				if (avatar1 != nullptr)
					ret = _rule->compareGenre(pg, avatar1->getOuttedGenre());
				if (ret != 1) {
					notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPlay), avatar);
					return false;
				}
			}
		}
		int lastNums = avatar->getCardNums();
		int nowNums = lastNums - static_cast<int>(ids.size());
		avatar->setOuttedGenre(pg);
		avatar->removeCardsByIds(ids);
		notifyPlayCard(_current, nullptr, true);
		notifyCardNums(_current, nullptr);
		_lastOut = _current;
		if (nowNums > 0) {
			if ((lastNums > 8) && (nowNums < 9))
				notifyCardAlert(_current);
		}
		else {
			for (int i = 0; i < 4; i++) {
				if (_finishedSeats[i] == -1) {
					_finishedSeats[i] = _current;
					break;
				}
			}
			// 玩家出牌完成，判定本局游戏是否结束，结束条件：友家先前已经出牌完牌
			int seat = getFriendSeat(_current);
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
			if (avatar1 == nullptr) {
				LOG_ERROR("error!");
				return false;	// 正常情况下不会执行到这里
			}
			if (avatar1->getCardNums() == 0) {
				// 友家已经出完牌，一局结束
				endRound();
				return true;
			}
			notifyFinished(nullptr);
		}
		int loop = 0;
		while (true) {
			_current++;
			if (_current >= getMaxPlayerNums())
				_current = 0;
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_current).get());
			if (avatar1 == nullptr) {
				// 正常情况下不会执行到这里
				LOG_ERROR("error!");
				return false;
			}
			if (avatar1->getCardNums() > 0)
				break;
			if (avatar1->isPlayed()) {
				avatar1->resetPlayed();
				notifyClearPlayedOut(_current);
			}
			loop++;
			if (loop > 4) {
				// 正常情况下不会执行到这里，避免因逻辑错误导致死循环
				LOG_ERROR("error!");
				return false;
			}
		}
		// 通知下一位玩家出牌
		waitPlayCard();
		return true;
	}

	bool GuanDanRoom::doPlayCard(GuanDanAvatar* avatar, const PokerCombination::Ptr& comb) {
		std::vector<int> ids;
		comb->getCards(ids);
		return doPlayCard(avatar, ids, comb);
	}

	bool GuanDanRoom::doPass(GuanDanAvatar* avatar) {
		if (_current == _lastOut) {
			notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPass), avatar);
			ErrorS << "掼蛋牌桌(ID: " << getId() << ")新一轮出牌当前玩家(座位: " << _current << ")必须出牌";
			return false;
		}
		avatar->setOuttedGenre(PokerGenre());
		notifyPlayCard(_current, nullptr, true);
		int loop = 0;
		GuanDanAvatar* avatar1 = nullptr;
		while (true) {
			_current++;
			if (_current >= getMaxPlayerNums())
				_current = 0;
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_current).get());
			if (avatar1 == nullptr) {
				// 正常情况下不会执行到这里
				LOG_ERROR("error!");
				return false;
			}
			if (avatar1->getCardNums() > 0)
				break;
			if (avatar1->isPlayed()) {
				avatar1->resetPlayed();
				notifyClearPlayedOut(_current);
			}
			if (_current == _lastOut) {
				// 玩家出完最后一手牌后，其他三位玩家都不压牌，友家可以借风出牌获得首位出牌权
				_current = getFriendSeat(_current);
				_lastOut = _current;
				notifyJieFeng(_current);
				break;
			}
			loop++;
			if (loop > 4) {
				// 正常情况下不会执行到这里，避免因逻辑错误导致死循环
				LOG_ERROR("error!");
				return false;
			}
		}
		// 通知下一位玩家出牌
		waitPlayCard();
		return true;
	}

	void GuanDanRoom::notifyPlayCardFailed(int reason, GuanDanAvatar* avatar) const {
		if (avatar == nullptr)
			return;
		MsgGuanDanPlayCardFailed msg;
		msg.reason = reason;
		msg.send(avatar->getSession());
	}

	void GuanDanRoom::notifyCardNums(int seat, GuanDanAvatar* avatarIn) const {
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgGuanDanCardNums msg;
		msg.seat = seat;
		msg.nums = avatar->getCardNums();
		if (avatarIn == nullptr)
			sendMessageToAll(msg, avatar->getPlayerId());
		else
			msg.send(avatarIn->getSession());
	}

	void GuanDanRoom::notifyCardAlert(int seat) const {
		GuanDanAvatar* avatar = nullptr;
		MsgGuanDanCardAlert msg;
		msg.seat = seat;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == seat)
				continue;
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (!isFriend(i, seat))
				msg.send(avatar->getSession());
		}
	}

	void GuanDanRoom::notifyPlayCard(int seat, GuanDanAvatar* avatarIn, bool realTime) const {
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgGuanDanPlayCard msg;
		const PokerGenre& genre = avatar->getOuttedGenre();
		msg.seat = seat;
		msg.genre = genre.getGenre();
		if (_rule->isValidGenre(genre.getGenre())) {
			msg.cards = genre.getCards();
			if (realTime)
				msg.firstPlay = (_lastOut == _current);
		}
		else
			msg.pass = true;
		msg.realTime = realTime;
		if (avatarIn == nullptr)
			sendMessageToAll(msg);
		else
			msg.send(avatarIn->getSession());
	}

	void GuanDanRoom::notifyClearPlayedOut(int seat) {
		MsgGuanDanClearPlayedOut msg;
		msg.seat = seat;
		sendMessageToAll(msg);
	}

	void GuanDanRoom::notifyFinished(GuanDanAvatar* avatarIn) {
		MsgGuanDanFinished msg;
		msg.touYou = _finishedSeats[0];
		msg.erYou = _finishedSeats[1];
		if (avatarIn == nullptr)
			sendMessageToAll(msg);
		else
			msg.send(avatarIn->getSession());
	}

	void GuanDanRoom::notifyJieFeng(int seat) {
		MsgGuanDanJieFeng msg;
		msg.seat = seat;
		sendMessageToAll(msg);
	}

	void GuanDanRoom::onHintCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::PlayCard)
			return;
		MsgGuanDanHintCard* inst = dynamic_cast<MsgGuanDanHintCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		MsgGuanDanHintCardResp resp;
		if (!avatar->getCandidateCards(resp.cardIds)) {
			std::string str;
			PokerUtilities::cardArray2String(avatar->getCards(), str);
			ErrorS << "掼蛋牌桌(ID: " << getId() << ")玩家(ID: " << inst->getPlayerId() << ")获取候选牌失败，手牌: " << str;
			return;
		}
		resp.send(netMsg->getSession());
	}

	void GuanDanRoom::onResetHintCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::PlayCard)
			return;
		MsgGuanDanResetHintCard* inst = dynamic_cast<MsgGuanDanResetHintCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		avatar->resetCandidatePos();
	}

	void GuanDanRoom::onHintStraightFlush(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing)
			return;
		MsgGuanDanHintStraightFlush* inst = dynamic_cast<MsgGuanDanHintStraightFlush*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GuanDanAvatar* avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->isCardsUpdated())
			avatar->analyzeCombinations();
		MsgGuanDanHintCardResp resp;
		if (avatar->getStraightFlush(resp.cardIds))
			resp.send(netMsg->getSession());
		else
			sendTipText("无同花顺", avatar->getPlayerId());
	}

	void GuanDanRoom::autoExecute(time_t nowTick) {
		time_t deltaTicks = 301LL;
		if (_autoTick != 0LL)
			deltaTicks = nowTick - _autoTick;
		if (deltaTicks < 300)
			return;
		_autoTick = nowTick;
		bool test = false;
		bool flag = false;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			test = false;
			flag = false;
			if (avatar->isRobot() || avatar->isAuthorize())
				test = true;
			else if (_level != static_cast<int>(GuanDanRoomLevel::Practice)) {
				deltaTicks = getWaitElapsed(nowTick);
				if (deltaTicks >= 15000) {
					// 超过15秒不执行，则自动执行，且进入托管状态
					test = true;
					flag = true;
				}
			}
			if (!test)
				continue;
			if (_operation == WaitOperation::PresentTribute) {
				// 等待进贡
				if (avatar->isWaitingPresentTribute()) {
					if (flag && !(avatar->isAuthorize())) {
						// 玩家超时未执行，自动托管
						avatar->setAuthorize(true);
						notifyPlayerAuthorize(avatar);
					}
					autoPresentTribute(avatar);
				}
			}
			else if (_operation == WaitOperation::RefundTribute) {
				// 等待还贡
				if (avatar->isWaitingRefundTribute()) {
					if (flag && !(avatar->isAuthorize())) {
						// 玩家超时未执行，自动托管
						avatar->setAuthorize(true);
						notifyPlayerAuthorize(avatar);
					}
					autoRefundTribute(avatar);
				}
			}
			else if (_operation == WaitOperation::PlayCard) {
				// 等待出牌
				if (i == _current) {
					if (flag && !(avatar->isAuthorize())) {
						// 玩家超时未执行，自动托管
						avatar->setAuthorize(true);
						notifyPlayerAuthorize(avatar);
					}
					autoPlayCard(avatar);
					break;
				}
			}
		}
	}

	void GuanDanRoom::autoPresentTribute(GuanDanAvatar* avatar) {
		PokerCard card;
		PokerUtilities::getBiggestCard(avatar->getCards(), card, _rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
		doPresentTribute(avatar, card);
	}

	void GuanDanRoom::autoRefundTribute(GuanDanAvatar* avatar) {
		// 查找10及以下且张数最少的牌用于还贡
		bool test = false;
		bool first = true;
		int orderUpper = _rule->getPointOrder(static_cast<int>(PokerPoint::Ten));
		int order = -1;
		int point1 = -1;
		int point2 = -1;
		int nums = 0;
		int minNums = 10000;
		int minPoint = -1;
		const CardArray& cards = avatar->getCards();
		for (const PokerCard& c : cards) {
			point2 = c.getPoint();
			if (_rule->getGradePoint() == point2)
				break;
			order = _rule->getPointOrder(point2);
			if (order > orderUpper)
				break;
			test = true;
			if (first) {
				first = false;
				point1 = point2;
				nums = 1;
			}
			else if (point2 != point1) {
				// 与前一张牌的牌值不相等，开始新的点数牌计数
				if (minNums > nums) {
					minNums = nums;
					minPoint = point1;
				}
				point1 = point2;
				nums = 1;
			}
			else
				nums++;
		}
		if (!test)
			return;
		if (minNums > nums) {
			minNums = nums;
			minPoint = point1;
		}
		PokerCard card;
		PokerUtilities::getFirstCardOfPoint(cards, card, minPoint);
		doRefundTribute(avatar, card);
	}

	void GuanDanRoom::autoPlayCard(GuanDanAvatar* avatar) {
		PokerCombination::Ptr comb = avatar->getFirstCandidate();
		if (!comb) {
			if (_lastOut == _current) {
				std::string str;
				PokerUtilities::cardArray2String(avatar->getCards(), str);
				ErrorS << "掼蛋牌桌(ID:" << getId() << ")首位出牌玩家(座位:" << _current << ")无候选出牌组合,当前手牌: " << str;
				return;
			}
			doPass(avatar);
			return;
		}
		bool ret = false;
		if (_lastOut == _current)
			ret = doPlayCard(avatar, comb);
		else {
			int num1 = avatar->getCardNums();
			int num2 = _rule->getGenreCardNums(comb->getGenre());
			// 若还剩下最后一首牌并且可出，则必出
			bool test = false;
			if (num1 > num2)
				test = makePassDecision(avatar, comb);
			if (test)
				ret = doPass(avatar);
			else
				ret = doPlayCard(avatar, comb);
		}
		if (!ret) {
			std::string str;
			PokerUtilities::cardArray2String(avatar->getCards(), str);
			ErrorS << "掼蛋牌桌(ID:" << getId() << ")玩家(座位:" << _current << ")自动出牌失败，当前手牌: " << str;
		}
	}

	bool GuanDanRoom::makePassDecision(GuanDanAvatar* avatar, const PokerCombination::Ptr& comb) {
		int num1 = 0;
		int num2 = 0;
		int genre = comb->getGenre();
		GuanDanAvatar* avatar1 = nullptr;
		int order = _rule->getBombOrder(genre);
		if (isFriend(_current, _lastOut)) {
			// 友家，不用大小王压友家，不拆牌压友家
			if ((order != -1) || comb->isBad())
				return true;
			if (comb->getOfficerPoint() == static_cast<int>(PokerPoint::Joker))
				return true;
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_lastOut).get());
			if (avatar1 != nullptr)
				num1 = avatar1->getCardNums();
			if (num1 < 3) {
				// 友家剩余小于3张牌
				avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar((_current + 1) % 4).get());
				if (avatar1 != nullptr)
					num1 = avatar1->getCardNums();
				avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar((_current + 3) % 4).get());
				if (avatar1 != nullptr)
					num2 = avatar1->getCardNums();
				if (num1 > num2)
					num1 = num2;
				if (num1 > 3)
					return true;	// 敌家剩余大于3张牌，不压友家牌
				// 二分之一的概率压牌
				num2 = BaseUtils::randInt(0, 10) + 1;
				if (num2 < 6)
					return true;
			}
			return false;
		}
		// 敌家
		if (comb->getOfficerPoint() == static_cast<int>(PokerPoint::Joker)) {
			// 大小王单张、对子、三张必出
			if (genre == static_cast<int>(GuanDanGenre::Single) ||
				genre == static_cast<int>(GuanDanGenre::Pair1) ||
				genre == static_cast<int>(GuanDanGenre::Triple1))
				return false;
		}
		if (avatar->isAllBig() && !comb->isBad())
			return false;	// 全是大牌且为优选组合，必出
		if ((order != -1) || comb->isBad()) {
			// 炸弹或者非优选组合，根据不同情形进行概率抉择
			avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(_lastOut).get());
			if (avatar1 != nullptr) {
				num1 = avatar1->getCardNums();
				if (num1 == 0) {
					// 上一个出牌的敌对阵营玩家已出完牌，查看其友家剩余张数
					int seat = getFriendSeat(_lastOut);
					avatar1 = dynamic_cast<GuanDanAvatar*>(getAvatar(seat).get());
					if (avatar1 != nullptr)
						num1 = avatar1->getCardNums();
				}
			}
			num2 = BaseUtils::randInt(0, 12) + 1;
			if (num1 > 12) {
				// 敌家剩余张数大于12张，有四分之一的概率进行压牌
				if (comb->isBad() || (num2 < 10))
					return true;
			}
			else if (num1 > 5) {
				// 敌家剩余牌数小于6张，则必定压牌
				if (comb->isBad())
					return true;
				else if (num1 > 8) {
					// 敌家剩余牌数大于8张，有三分之一的概率进行压牌
					if (num2 < 9)
						return true;
				}
				else if (num2 < 5) {
					// 敌家剩余牌数小于等于8张，有三分之二的概率进行压牌
					return true;
				}
			}
		}
		return false;
	}

	void GuanDanRoom::endRound() {
		_gameState = GameState::Waiting;
		_waitTick = BaseUtils::getCurrentMillisecond();

		for (int i = 0; i < 4; i++) {
			// 增加输赢局记录
			GameAvatar::Ptr ptr = getAvatar((_finishedSeats[0] + i) % 4);
			if (!ptr)
				continue;
			if (i == 0 || i == 2)
				incWinNum(ptr->getPlayerId());
			else
				incLoseNum(ptr->getPlayerId());
		}
		int64_t diamond = 0LL;
		GuanDanAvatar* avatar = nullptr;
		for (int i = 0; i < 4; i++) {
			avatar = dynamic_cast<GuanDanAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (avatar->getCardNums() > 0) {
				// 该玩家尚未出完牌
				for (int j = 2; j < 4; j++) {
					if (_finishedSeats[j] == -1) {
						_finishedSeats[j] = i;
						break;
					}
				}
			}
			if (_level != static_cast<int>(GuanDanRoomLevel::Practice)) {
				// 检查玩家钻石是否还够下局扣除
				std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
				MysqlPool::getSingleton().syncQuery(task);
				if (task->getSucceed() && (task->getRows() > 0))
					diamond = task->getDiamond();
				else
					diamond = 0;
				if (2LL > diamond)
					_kicks[i] = true;	// 玩家剩余钻石已经不足下局扣除，踢出
			}
		}
		int upgrade = 0;
		int gradePoint = 0;
		int seat1 = _finishedSeats[0];
		int seat2 = getFriendSeat(seat1);
		if (seat2 == _finishedSeats[1])
			upgrade = 3;	// 一方占了头游和二游，升三级
		else if (seat2 == _finishedSeats[2])
			upgrade = 2;	// 一方占了头游和三游，升两级
		else
			upgrade = 1;	// 一方占了头游和末游，升一级
		if (isRedSeat(seat1)) {
			gradePoint = _gradePointRed;
			_bankerNext = 1;
		}
		else {
			gradePoint = _gradePointBlue;
			_bankerNext = 2;
		}
		_gradePointNext = gradePoint + upgrade;
		if (gradePoint != static_cast<int>(PokerPoint::Ace)) {
			if (_gradePointNext > static_cast<int>(PokerPoint::King))
				_gradePointNext = static_cast<int>(PokerPoint::Ace);	// 升级不能跳过A
		}
		MsgGuanDanResult msg;
		for (int i = 0; i < 4; i++) {
			msg.finishedSeats[i] = _finishedSeats[i];
			msg.kicks[i] = _kicks[i];
		}
		msg.gradePointNext = _gradePointNext;
		sendMessageToAll(msg);
		for (int i = 0; i < 4; i++) {
			GameAvatar::Ptr ptr = getAvatar(i);
			if (!ptr)
				continue;
			if (_kicks[i])
				kickAvatar(ptr);	// 将玩家踢出房间
			else if (!ptr->isRobot())
				ptr->setReady(false);
		}
	}

	void GuanDanRoom::onDisbandRequest(const NetMessage::Ptr& netMsg) {
		if (_gameState == GameState::Sitting || _gameState == GameState::Waiting)
			return;	// 入座和等待状态，可以直接离开
		if (_level != static_cast<int>(GuanDanRoomLevel::Friend))
			return;	// 非好友房间，不可解散
		if (_disbanding)
			return;
		MsgDisbandRequest* inst = dynamic_cast<MsgDisbandRequest*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		GameAvatar::Ptr avatar = getAvatar(inst->getPlayerId());
		if (!avatar)
			return;
		_disbander = avatar->getSeat();
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _disbander)
				_disbandChoises[i] = 1;
			else
				_disbandChoises[i] = 0;
		}
		_disbanding = true;
		_beginDisbandTick = BaseUtils::getCurrentMillisecond();
		notifyDisbandVote(BaseUtils::EMPTY_STRING);
	}

	void GuanDanRoom::notifyDisbandVote(const std::string& playerId) {
		MsgGuanDanDisbandVote msg;
		msg.disbander = _disbander;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int elapsed = static_cast<int>(nowTick - _beginDisbandTick);
		msg.elapsed = elapsed;
		for (int i = 0; i < getMaxPlayerNums(); i++)
			msg.choices[i] = _disbandChoises[i];
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void GuanDanRoom::onDisbandChoose(const NetMessage::Ptr& netMsg) {
		MsgDisbandChoose* inst = dynamic_cast<MsgDisbandChoose*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if ((inst->choice != 1) && (inst->choice != 2))
			return;
		GameAvatar::Ptr avatar = getAvatar(inst->getPlayerId());
		if (avatar)
			doDisbandChoose(avatar->getSeat(), inst->choice);
	}

	void GuanDanRoom::doDisbandChoose(int seat, int choise) {
		int maxPlayerNums = getMaxPlayerNums();
		if (seat < 0 || seat >= maxPlayerNums)
			return;
		int half = (maxPlayerNums >> 1);
		int nums1 = 0;
		int nums2 = 0;
		_disbandChoises[seat] = choise;
		for (int i = 0; i < maxPlayerNums; i++) {
			if (_disbandChoises[i] == 1)
				nums1++;
			else if (_disbandChoises[i] == 2)
				nums2++;
		}
		bool test = false;
		if ((maxPlayerNums & 1) != 0) {
			if (nums2 > half)
				test = true;
		}
		else if (nums2 >= half)
			test = true;

		MsgDisbandChoice msg;
		msg.seat = seat;
		msg.choice = choise;
		sendMessageToAll(msg);

		if (nums1 > half)
			disbandRoom();		// 超过一半的玩家同意解散，解散
		else if (test)
			disbandObsolete();	// 有一半及以上的玩家不同意解散，解散失败
	}

	void GuanDanRoom::disbandRoom() {
		// 房间解散
		MsgDisband msg;
		sendMessageToAll(msg);

		_gameState = GameState::Waiting;

		kickAllAvatars();
		// 在解散房间后游戏结束
		gameOver();
	}

	void GuanDanRoom::disbandObsolete() {
		// 取消解散
		_disbanding = false;
		_endDisbandTick = BaseUtils::getCurrentMillisecond();
		MsgDisbandObsolete msg;
		sendMessageToAll(msg);
	}

	void GuanDanRoom::removeRoomFromDb() {
		std::string sql = "delete from `game_guan_dan` where `venue_id` = \"" + getId() + "\"";
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Delete);
		MysqlPool::getSingleton().asyncQuery(task);
	}
}