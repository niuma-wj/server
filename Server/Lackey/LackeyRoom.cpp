// LackeyRoom.cpp

#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "Game/DebtLiquidation.h"
#include "Game/GameMessages.h"
#include "Game/GetCapitalTask.h"
#include "Player/PlayerManager.h"
#include "MySql/MysqlPool.h"
#include "../GameDefines.h"
#include "PokerUtilities.h"
#include "LackeyRule.h"
#include "LackeyAvatarEx.h"
#include "LackeyMessages.h"
#include "LackeyRoom.h"
#include "Constant/RedisKeys.h"
#include "Redis/RedisPool.h"

#include "jsoncpp/include/json/json.h"

#include <mysql/jdbc.h>

#include <algorithm>
#include <sstream>

namespace NiuMa
{
	const int LackeyRoom::RULER_TABLE[10] = { 1, 2, 3, 4, 18, 19, 20, 35, 36, 52 };

	LackeyRoom::LackeyRoom(const std::shared_ptr<LackeyRule>& rule,
		const std::string& venueId, const std::string& number, int lvl, int mode, int diZhu)
		: GameRoom(venueId, static_cast<int>(GameType::Lackey), 5)
		, _rule(rule)
		, _dealer(rule)
		, _number(number)
		, _level(lvl)
		, _mode(mode)
		, _diZhu(diZhu)
		, _roundNo(0)
		, _gameState(GameState::Waiting)
		, _current(-1)
		, _lastOut(-1)
		, _operation(WaitOperation::None)
		, _landlord(-1)
		, _lackey(-1)
		, _lackeyCard(-1)
		, _callLackey(-1)
		, _showCardDuration(30000)
		, _beiLv(1)
		, _showCard1(false)
		, _showCard2(false)
		, _showLackey(false)
		, _firstCircle(true)
		, _disbanding(false)
		, _disbander(0)
		, _lastRegisterTick(0)
		, _waitTick(0)
		, _beginDisbandTick(0)
		, _endDisbandTick(0)
	{
		for (int i = 0; i < 5; i++) {
			_kicks[i] = false;
			_disbandChoises[i] = 0;
		}
		for (int i = 0; i < 10; i++)
			_distances[i] = -1.0;
		if (_mode == 0)
			setDiamondNeed(2);
		setCashPledge(_diZhu * 15);
	}

	LackeyRoom::~LackeyRoom()
	{}

	void LackeyRoom::initialize() {
		GameRoom::initialize();
	}

	void LackeyRoom::onTimer() {
		GameRoom::onTimer();

		int deltaTicks = 0;
		if (_level != static_cast<int>(LackeyRoomLevel::Friend)) {
			time_t nowTick = BaseUtils::getCurrentMillisecond();
			deltaTicks = 5001;
			if (_lastRegisterTick != 0)
				deltaTicks = static_cast<int>(nowTick - _lastRegisterTick);
			if (deltaTicks > 5000) {
				// 每5秒刷新一次注册项
				_lastRegisterTick = nowTick;
				int districtId = getDistrictId();
				std::string redisKey = RedisKeys::DISTRICT_VENUE_REGISTER + std::to_string(districtId);
				RedisPool::getSingleton().hset(redisKey, getId(), std::to_string(nowTick));
			}
		}
		if (_gameState == GameState::Waiting) {
			checkOffline();
			return;
		}
		if (_disbanding) {
			time_t nowTick = BaseUtils::getCurrentMillisecond();
			deltaTicks = static_cast<int>(nowTick - _beginDisbandTick);
			if (deltaTicks > 300000) {
				// 超时不选择默认同意解散
				for (int i = 0; i < getMaxPlayerNums(); i++) {
					if (_disbandChoises[i] == 0)
						doDisbandChoose(i, 1);
				}
			}
			return;
		}
		deltaTicks = getWaitElapsed();
		if (_gameState == GameState::Dealing) {
			// 发牌等待2秒客户端播放发牌动画
			if (deltaTicks < 2000)
				return;
			beginPlay();
		}
		else if (_gameState == GameState::Playing) {
			if (_operation == WaitOperation::CallLackey) {
				// 等待地主叫狗腿，默认叫狗腿
				if (deltaTicks >= 20000)
					doCallLackey(true);
			}
			else if (_operation == WaitOperation::ShowCard) {
				// 等待玩家明牌，默认不明牌
				if (deltaTicks >= _showCardDuration)
					doShowCard(false);
			}
			else if (_operation == WaitOperation::PlayCard) {
				// 等待玩家出牌，超时自动出牌
				if (deltaTicks >= 15000)
					timePlayCard();
				else if (deltaTicks >= 3000)
					authPlayCard();	// 托管玩家只等待3秒就自动出牌
			}
		}
	}

	bool LackeyRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (GameRoom::onMessage(netMsg))
			return true;

		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgLackeySync::TYPE)
			onSyncLackey(netMsg);
		else if (msgType == MsgPlayerReady::TYPE)
			onPlayerReady(netMsg);
		else if (msgType == MsgPlayerAuthorize::TYPE)
			onPlayerAuthorize(netMsg);
		else if (msgType == MsgDoCallLackey::TYPE)
			onCallLackey(netMsg);
		else if (msgType == MsgLackeyDoShowCard::TYPE)
			onShowCard(netMsg);
		else if (msgType == MsgLackeyDoPlayCard::TYPE)
			onPlayCard(netMsg);
		else if (msgType == MsgLackeyHintCard::TYPE)
			onHintCard(netMsg);
		else if (msgType == MsgDisbandChoose::TYPE)
			onDisbandChoose(netMsg);
		else
			ret = false;

		return ret;
	}

	GameAvatar::Ptr LackeyRoom::createAvatar(const std::string& playerId, int seat, bool robot) const {
		return std::make_shared<LackeyAvatarEx>(_rule, playerId, seat, robot);
	}

	bool LackeyRoom::checkEnter(const std::string& playerId, std::string& errMsg) const {
		if (_gameState != GameState::Waiting) {
			errMsg = "游戏正在进行中，不能进入房间";
			return false;
		}
		return true;
	}

	int LackeyRoom::checkLeave(const std::string& playerId, std::string& errMsg) const {
		if (_gameState != GameState::Waiting) {
			if (_level != static_cast<int>(LackeyRoomLevel::Friend)) {
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

	void LackeyRoom::onAvatarJoined(int seat, const std::string& playerId) {
		if (_level == static_cast<int>(LackeyRoomLevel::Friend))
			return;
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

	void LackeyRoom::onAvatarLeaved(int seat, const std::string& playerId) {
		clearDistances(seat, _distances);
		int count = getAvatarCount();
		if (count == 0 && (_level == static_cast<int>(LackeyRoomLevel::Friend)))
			gameOver();
		if (_level != static_cast<int>(LackeyRoomLevel::Friend)) {
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
	}

	void LackeyRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {
		std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
		MysqlPool::getSingleton().syncQuery(task);
		int64_t gold = avatar->getCashPledge();
		int64_t diamod = 0LL;
		if (task->getSucceed() && task->getRows() > 0) {
			gold += task->getGold();
			diamod = task->getDiamond();
		}
		Json::Value tmp(Json::objectValue);
		tmp["gold"] = static_cast<Json::Int64>(gold);
		tmp["diamond"] = static_cast<Json::Int64>(diamod);
		if (!avatar->isOffline()) {
			Session::Ptr session = avatar->getSession();
			tmp["ip"] = session->getRemoteIp();
		}
		tmp["authorize"] = avatar->isAuthorize();
		std::string json = tmp.toStyledString();
		BaseUtils::encodeBase64(base64, json.data(), static_cast<int>(json.size()));
	}

	void LackeyRoom::onDisconnect(const std::string& playerId) {
		GameRoom::onDisconnect(playerId);

		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(playerId).get());
		if (avatar != nullptr)
			avatar->setOfflineTick(BaseUtils::getCurrentMillisecond());
	}

	void LackeyRoom::clean() {
		_current = -1;
		_lastOut = -1;
		_operation = WaitOperation::None;
		_landlord = -1;
		_lackey = -1;
		_lackeyCard = -1;
		_callLackey = -1;
		_showCardDuration = 30000;
		_beiLv = 1;
		_showCard1 = false;
		_showCard2 = false;
		_showLackey = false;
		_firstCircle = true;
		for (int i = 0; i < 5; i++)
			_kicks[i] = false;

		LackeyAvatarEx* avatar = NULL;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar != nullptr)
				avatar->clear();
		}
	}

	int LackeyRoom::getDistrictId() const {
		if (_level == static_cast<int>(LackeyRoomLevel::Friend))
			return 0;
		if (_level == static_cast<int>(LackeyRoomLevel::Beginner))
			return 1;
		if (_level == static_cast<int>(LackeyRoomLevel::Moderate))
			return 2;
		if (_level == static_cast<int>(LackeyRoomLevel::Advanced))
			return 3;
		if (_level == static_cast<int>(LackeyRoomLevel::Master))
			return 4;
		return 0;
	}

	void LackeyRoom::checkOffline() {
		if (_level == static_cast<int>(LackeyRoomLevel::Friend))
			return;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int deltaTicks = 0;
		LackeyAvatarEx* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if ((avatar == nullptr) || !(avatar->isOffline()))
				continue;
			deltaTicks = static_cast<int>(nowTick - avatar->getOfflineTick());
			if (deltaTicks > 60000)	// 将离线超过一分钟的玩家踢出游戏场
				kickAvatar(getAvatar(i));
		}
	}

	int LackeyRoom::getWaitElapsed() const {
		int deltaTicks = 0;
		if (_disbanding) {
			// 正在投票解散，从开始投票解散之后所有游戏逻辑都停止计时
			deltaTicks = static_cast<int>(_beginDisbandTick - _waitTick);
			return deltaTicks;
		}
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		// 跨过中间投票解散的时间端
		if (_endDisbandTick > _waitTick)
			deltaTicks = static_cast<int>(nowTick - _endDisbandTick + _beginDisbandTick - _waitTick);
		else
			deltaTicks = static_cast<int>(nowTick - _waitTick);
		return deltaTicks;
	}

	void LackeyRoom::onSyncLackey(const NetMessage::Ptr& netMsg) {
		MsgLackeySync* inst = dynamic_cast<MsgLackeySync*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		int seat = avatar->getSeat();
		MsgLackeySyncResp msg;
		msg.number = _number;
		msg.level = _level;
		msg.mode = _mode;
		msg.diZhu = _diZhu;
		msg.gameState = static_cast<int>(_gameState);
		msg.seat = seat;
		msg.landlord = _landlord;
		msg.send(netMsg->getSession());
		// 发送全部玩家数据
		sendAvatars(netMsg->getSession());

		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (getAvatar(i))
				sendWinLose(i, inst->getPlayerId());
		}
		if (_gameState == GameState::Playing) {
			sendHandCard(seat);
			notifyLackeyCard(inst->getPlayerId());
			if (_showLackey || (seat == _lackey))
				notifyLackeySeat(inst->getPlayerId(), false);
			if (avatar->isShowCard())
				notifyShowCard(seat, avatar->getPlayerId());
			if ((seat != _current) && avatar->isPlayed())
				notifyPlayCard(seat, inst->getPlayerId(), 0, false);
			if (avatar->getXiQian() > 0)
				notifyXiQian(seat, inst->getPlayerId(), false);
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				if (i == seat)
					continue;
				avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
				if (avatar == nullptr)
					continue;
				if (avatar->isShowCard())
					notifyShowCard(i, inst->getPlayerId());
				if ((i != _current) && avatar->isPlayed())
					notifyPlayCard(i, inst->getPlayerId(), 0, false);
				notifyCardNums(i, inst->getPlayerId());
				if (avatar->getXiQian() > 0)
					notifyXiQian(i, inst->getPlayerId(), false);
			}
			int deltaTicks = getWaitElapsed();
			if (seat == _current) {
				if (_operation == WaitOperation::CallLackey)
					notifyCallLackey(deltaTicks);
				else if (_operation == WaitOperation::ShowCard)
					notifyWaitShowCard(deltaTicks);
				else if (_operation == WaitOperation::PlayCard)
					notifyWaitPlayCard(deltaTicks);
			}
			else {
				int duration = 15000;
				if (_operation == WaitOperation::CallLackey)
					duration = 20000;
				else if (_operation == WaitOperation::ShowCard)
					duration = _showCardDuration;
				notifyWaitOption(deltaTicks, duration, inst->getPlayerId());
			}
		}
		if (_disbanding)
			notifyDisbandVote(inst->getPlayerId());
	}

	void LackeyRoom::onPlayerReady(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Waiting)
			return;
		MsgPlayerReady* inst = dynamic_cast<MsgPlayerReady*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		avatar->setReady(true);
		MsgPlayerReadyResp msg;
		msg.playerId = inst->getPlayerId();
		msg.seat = avatar->getSeat();
		sendMessageToAll(msg);
		
		if (isFull() && isAllReady()) {
			// 所有人都已经准备好，开始发牌
			beginDeal();
		}
	}

	void LackeyRoom::onPlayerAuthorize(const NetMessage::Ptr& netMsg) {
		MsgPlayerAuthorize* inst = dynamic_cast<MsgPlayerAuthorize*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->isAuthorize())
			avatar->setAuthorize(false);
		else
			avatar->setAuthorize(true);
		notifyPlayerAuthorize(avatar->getSeat(), BaseUtils::EMPTY_STRING);
	}

	void LackeyRoom::beginDeal() {
		_gameState = GameState::Dealing;
		_waitTick = BaseUtils::getCurrentMillisecond();

		clean();

		getRoundNo();

		// 扣除钻石
		deductDiamond();

		// 随机指定一名玩家为地主
		int maxPlayerNums = getMaxPlayerNums();
		_landlord = static_cast<int>(BaseUtils::randInt(0, maxPlayerNums));
		std::ostringstream os;
		os << "逮狗腿牌桌(ID:" << getId() << ")发牌，各玩家ID:";
		for (int i = 0; i < maxPlayerNums; i++) {
			if (i > 0)
				os << "、";
			os << getAvatar(i)->getPlayerId();
		}
		os << "，地主座位号:" << _landlord;
		LOG_INFO(os.str());

		CardArray cards;
		CardComparator comp(_rule);
		while (true) {
			cards.clear();
			_dealer.shuffle();
			// 地主发38张牌
			_dealer.handOutCards(cards, 38, DealFilter::Ptr());
			std::sort(cards.begin(), cards.end(), comp);
			if (findLackeyCard(cards))
				break;
		}
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_landlord).get());
		avatar->setCards(cards);
		for (int i = 0; i < maxPlayerNums; i++) {
			if (_landlord == i)
				continue;
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			// 非地主发31张牌
			_dealer.handOutCards(cards, 31, DealFilter::Ptr());
			std::sort(cards.begin(), cards.end(), comp);
			avatar->setCards(cards);
			// 当地主没拿到默认狗腿牌时，拿到默认狗腿牌的玩家自动成为狗腿
			if ((_callLackey == -1) && PokerUtilities::hasCard(cards, _lackeyCard))
				_lackey = i;
		}
		// 通知发牌，客户端做发牌动画
		MsgLackeyDealCard msg;
		msg.landlord = _landlord;
		sendMessageToAll(msg);
	}

	void LackeyRoom::getRoundNo() {
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
					ss << "select max(`round_no`) from `game_lackey_round` where `venue_id` = \"" << _venueId << "\"";
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

	void LackeyRoom::deductDiamond() {
		if (_mode != 0)
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

	bool LackeyRoom::findLackeyCard(const CardArray& cards) {
		int tmp = _rule->getDefaultLackeyCard(_dealer);
		if (!PokerUtilities::hasCard(cards, tmp)) {
			// 地主没拿到默认狗腿牌
			_lackeyCard = tmp;
			return true;
		}
		// 地主手上拿着默认狗腿牌，由大到小查找地主手牌上两张(一定是两张)大小及花色都相同的牌，如果能找到，则该牌作为狗腿牌
		bool test = false;
		int i = static_cast<int>(cards.size()) - 1;
		PokerCard c0 = cards[i];
		PokerCard c1;
		for (; i > 0; i--) {
			if (c0.getPoint() == static_cast<int>(PokerPoint::Joker) ||
				c0.getPoint() == static_cast<int>(PokerPoint::Two)) {
				c0 = cards[i - 1];
				continue;
			}
			if (cards[i - 1] == c0) {
				if ((i > 1) && cards[i - 2] == c0) {
					// 找到三张大小及花色都相同的牌
					if (i > 2) {
						c0 = cards[i - 3];
						i -= 2;
					}
				}
				else {
					// 找到两张大小及花色都相同的牌
					c1 = cards[i - 1];
					test = true;
					break;
				}
			}
			else
				c0 = cards[i - 1];
		}
		if (test) {
			PokerCard c2(c0);
			_dealer.getFirstCard(c2);
			tmp = c2.getId();
			if (tmp == c0.getId()) {
				tmp++;
				if (tmp == c1.getId())
					tmp++;
			}
			else if (tmp == c1.getId()) {
				tmp++;
				if (tmp == c0.getId())
					tmp++;
			}
			_callLackey = tmp;
			return true;
		}
#if defined(_DEBUG) || defined(DEBUG)
		std::string str1;
		std::string str2("地主无法叫狗腿再次发牌，手牌: ");
		PokerUtilities::cardArray2String(cards, str1);
		str2 += str1;
		LOG_DEBUG(str2);
#endif
		return false;
	}

	void LackeyRoom::beginPlay() {
		_gameState = GameState::Playing;

		// 发送每个玩家的手牌
		for (int i = 0; i < getMaxPlayerNums(); i++)
			sendHandCard(i);

		_current = _landlord;
		_lastOut = _landlord;
		if (_callLackey != -1) {
			// 等待地主叫狗腿
			waitCallLackey();
			return;
		}
		// 通知狗腿牌
		notifyLackeyCard(BaseUtils::EMPTY_STRING);
		// 通知拿到狗腿牌的玩家成为狗腿
		if (_lackey != -1) {
			GameAvatar::Ptr avatar = getAvatar(_lackey);
			if (avatar)
				notifyLackeySeat(avatar->getPlayerId(), false);
		}
		// 通知地主是否明牌，无机会1v4等待40秒，否则等待20秒，这样做的目的是为了防止
		// 其他玩家能从等待时间上判定地主是否决定1v4
		_showCardDuration = 40000;
		waitShowCard();
	}

	void LackeyRoom::waitCallLackey() {
		_operation = WaitOperation::CallLackey;
		_waitTick = BaseUtils::getCurrentMillisecond();

		notifyCallLackey(0);
		// 通知其他玩家等待操作
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _current)
				continue;
			GameAvatar::Ptr avatar = getAvatar(i);
			if (avatar)
				notifyWaitOption(0, 40000, avatar->getPlayerId());
		}
	}

	void LackeyRoom::waitShowCard() {
		_operation = WaitOperation::ShowCard;
		_waitTick = BaseUtils::getCurrentMillisecond();

		notifyWaitShowCard(0);
		// 通知其他玩家等待操作
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _current)
				continue;
			GameAvatar::Ptr avatar = getAvatar(i);
			if (avatar)
				notifyWaitOption(0, _showCardDuration, avatar->getPlayerId());
		}
	}

	void LackeyRoom::waitPlayCard() {
		_operation = WaitOperation::PlayCard;
		_waitTick = BaseUtils::getCurrentMillisecond();

		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		// 分析所有组合
		avatar->analyzeCombinations();
		if (_lastOut == _current)
			avatar->candidateCombinations();
		else {
			// 检索所有可出的候选组合
			LackeyAvatarEx* avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(_lastOut).get());
			if (avatar1 != nullptr)
				avatar->candidateCombinations(avatar1->getOuttedGenre());
		}
		// 通知玩家出牌
		notifyWaitPlayCard(0);
		// 通知其他玩家等待操作
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _current)
				continue;
			GameAvatar::Ptr tmp = getAvatar(i);
			if (tmp)
				notifyWaitOption(0, 15000, tmp->getPlayerId());
		}
	}

	void LackeyRoom::timePlayCard() {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		if (avatar->isOffline() && !(avatar->isAuthorize())) {
			// 离线玩家超时未牌，自动托管
			avatar->setAuthorize(true);
			notifyPlayerAuthorize(avatar->getSeat(), BaseUtils::EMPTY_STRING);
		}
		autoPlayCard();
	}

	void LackeyRoom::authPlayCard() {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		if (avatar->isAuthorize())
			autoPlayCard();
	}

	bool LackeyRoom::getCamp(int seat) const {
		if (seat == _landlord || seat == _lackey)
			return true;
		return false;
	}

	void LackeyRoom::notifyPlayerAuthorize(int seat, const std::string& playerId) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgPlayerAuthorizeResp msg;
		msg.seat = seat;
		msg.authorize = avatar->isAuthorize();
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	double* LackeyRoom::getDistances() {
		return _distances;
	}

	void LackeyRoom::getDistances(std::vector<int>& distances) const {
		for (int i = 0; i < 10; i++)
			distances.push_back(static_cast<int>(_distances[i]));
	}

	int LackeyRoom::getDistanceIndex(int seat1, int seat2) const {
		int i1 = seat1;
		int i2 = seat2;
		if (seat1 > seat2) {
			i1 = seat2;
			i2 = seat1;
		}
		int i3 = (i1 << 4) | i2;
		for (int i = 0; i < 10; i++) {
			if (i3 == RULER_TABLE[i])
				return i;
		}
		return -1;
	}

	void LackeyRoom::sendWinLose(int seat, const std::string& playerId) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgLackeyWinLose msg;
		msg.seat = seat;
		avatar->getWinLose(msg.win, msg.lose, msg.draw);
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::sendHandCard(int seat) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgLackeyHandCard msg;
		msg.cards = avatar->getCards();
		sendMessage(msg, avatar->getPlayerId());
	}

	void LackeyRoom::notifyWaitOption(int elapsed, int duration, const std::string& playerId) const {
		if (playerId.empty())
			return;
		// 通知等待玩家操作
		MsgLackeyWaitOption msg;
		msg.seat = _current;
		msg.elapsed = elapsed;
		msg.duration = duration;
		sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyCallLackey(int elapsed) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_landlord).get());
		if (avatar == NULL)
			return;
		MsgWaitCallLackey msg;
		msg.elapsed = elapsed;
		msg.send(avatar->getSession());
	}

	void LackeyRoom::onCallLackey(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::CallLackey)
			return;
		MsgDoCallLackey* inst = dynamic_cast<MsgDoCallLackey*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current || _current != _landlord)
			return;
		doCallLackey(inst->yes);
	}

	void LackeyRoom::doCallLackey(bool yes) {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		if (yes) {
			// 地主叫狗腿
			_lackeyCard = _callLackey;
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				if (i == _landlord)
					continue;
				avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
				if (avatar == nullptr)
					continue;
				if (avatar->hasCard(_lackeyCard)) {
					_lackey = i;
					break;
				}
			}
			if (_lackey == -1) {
				ErrorS << "牌桌(ID: " << getId() << ")地主叫狗腿(牌ID: " << _lackeyCard << ")，但所有其他玩家都没有拿到该牌";
				return;
			}
		}
		else {
			// 地主1打4
			_lackey = _landlord;
			_lackeyCard = _rule->getDefaultLackeyCard(_dealer);
			avatar->setLackeyCard(_lackeyCard);
		}
		// 通知狗腿牌
		notifyLackeyCard(BaseUtils::EMPTY_STRING);
		// 通知狗腿玩家他成为狗腿(或者地主1v4)
		GameAvatar::Ptr tmp = getAvatar(_lackey);
		if (tmp)
			notifyLackeySeat(tmp->getPlayerId(), false);
		tmp = getAvatar(_landlord);
		if (tmp) {
			// 通知地主叫狗腿完成
			MsgCallLackeyDone msg;
			msg.send(tmp->getSession());
		}
		// 通知地主是否明牌
		_showCardDuration = 20000;
		waitShowCard();
	}

	void LackeyRoom::notifyLackeyCard(const std::string& playerId) const {
		// 狗腿牌还未确定
		if (_lackeyCard == -1)
			return;
		MsgLackeyCard msg;
		_dealer.getCard(msg.card, _lackeyCard);
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyLackeySeat(const std::string& playerId, bool playSound) const {
		MsgLackeySeat msg;
		msg.seat = _lackey;
		msg.playSound = playSound;
		if (playerId.empty()) {
			std::string exceptedId;
			GameAvatar::Ptr tmp = getAvatar(_lackey);
			if (tmp)
				exceptedId = tmp->getPlayerId();
			sendMessageToAll(msg, exceptedId);
		}
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyWaitShowCard(int elapsed) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		MsgLackeyWaitShowCard msg;
		msg.elapsed = elapsed;
		msg.duration = _showCardDuration;
		msg.send(avatar->getSession());
	}

	void LackeyRoom::onShowCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::ShowCard)
			return;
		MsgLackeyDoShowCard* inst = dynamic_cast<MsgLackeyDoShowCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		doShowCard(inst->yes);
	}

	void LackeyRoom::doShowCard(bool yes) {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		if (yes) {
			bool camp = getCamp(_current);
			if (camp)
				_showCard1 = true;
			else
				_showCard2 = true;
			if (_current == _lackey) {
				// 狗腿现身
				_showLackey = true;
				notifyLackeySeat(BaseUtils::EMPTY_STRING, true);
			}
			avatar->setShowCard();
			notifyShowCard(_current, BaseUtils::EMPTY_STRING);
		}
		// 通知明牌操作执行完成
		MsgLackeyShowCardDone msg;
		msg.show = yes;
		msg.send(avatar->getSession());
		// 玩家选择明牌或者不明之后，继续通知该玩家出牌
		waitPlayCard();
	}

	void LackeyRoom::notifyShowCard(int seat, const std::string& playerId) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgLackeyShowCard msg;
		msg.seat = seat;
		if (avatar->getPlayerId() != playerId) {
			if (seat == _landlord)
				msg.role = 0;
			else if (seat == _lackey)
				msg.role = 1;
			else
				msg.role = 2;
			msg.cards = avatar->getCards();
		}
		if (playerId.empty())
			sendMessageToAll(msg, avatar->getPlayerId());
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyWaitPlayCard(int elapsed) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return;
		MsgLackeyWaitPlayCard msg;
		if (_lastOut == _current) {
			msg.firstPlay = true;
			msg.canPlay = true;
		}
		else {
			msg.firstPlay = false;
			msg.canPlay = avatar->hasCandidate();
		}
		msg.elapsed = elapsed;
		msg.send(avatar->getSession());
	}

	void LackeyRoom::onPlayCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::PlayCard)
			return;
		MsgLackeyDoPlayCard* inst = dynamic_cast<MsgLackeyDoPlayCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		doPlayCard(inst->pass, inst->cardIds);
	}

	bool LackeyRoom::doPlayCard(bool pass, const std::vector<int>& ids) {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return false;
		int xiQian = 0;
		if (pass) {
			if (_current == _lastOut) {
				notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPass), avatar);
				ErrorS << "牌桌(ID: " << getId() << ")新一轮出牌当前玩家(座位: " << _current << ")必须出牌";
				return false;
			}
			avatar->setOuttedGenre(PokerGenre());
		}
		else if (!avatar->hasCandidate()) {
			notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPlay), avatar);
			ErrorS << "牌桌(ID: " << getId() << ")当前玩家(座位: " << _current << ")要不起";
			return false;
		}
		else {
			CardArray cards;
			if (!avatar->getCardsByIds(ids, cards)) {
				notifyPlayCardFailed(static_cast<int>(PlayCardFailed::NotFound), avatar);

				std::ostringstream os;
				os << "牌桌(ID:" << getId() << ")当前玩家(座位:" << _current << ")出牌失败，找不到指定的牌(ID:";
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
			PokerGenre genre;
			genre.setCards(cards, _rule);
			if ((cards.size() == 1) && (ids[0] == avatar->getLackeyCard()))
				genre.setGenre(static_cast<int>(LackeyGenre::BombLackey));
			if (!_rule->isValidGenre(genre.getGenre())) {
				// 无效牌型
				notifyPlayCardFailed(static_cast<int>(PlayCardFailed::Invalid), avatar);
				return false;
			}
			if (_current != _lastOut) {
				int ret = 0;
				LackeyAvatarEx* avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(_lastOut).get());
				if (avatar1 != nullptr)
					ret = _rule->compareGenre(genre, avatar1->getOuttedGenre());
				if (ret != 1) {
					notifyPlayCardFailed(static_cast<int>(PlayCardFailed::CanNotPlay), avatar);
					return false;
				}
			}
			int lastNums = avatar->getCardNums();
			int nowNums = lastNums - static_cast<int>(ids.size());
			avatar->setOuttedGenre(genre);
			avatar->removeCardsByIds(ids);
			_lastOut = _current;
			if (_current == _lackey) {
				if (PokerUtilities::hasCard(cards, _lackeyCard)) {
					// 狗腿现身
					_showLackey = true;
					notifyLackeySeat(BaseUtils::EMPTY_STRING, true);
				}
			}
			xiQian = LackeyRule::calcXiQian(genre.getGenre(), genre.getCards());
			if (xiQian > 0) {
				avatar->addXiQian(xiQian);
				notifyXiQian(_current, BaseUtils::EMPTY_STRING, true);
			}
			notifyCardNums(_current, BaseUtils::EMPTY_STRING);
			if (lastNums > 5 && nowNums <= 5 && nowNums > 0)
				notifyCardAlert(_current);
		}
		notifyPlayCard(_current, BaseUtils::EMPTY_STRING, xiQian, true);
		if (avatar->getCards().empty()) {	
			if (!_showLackey) {
				// 若一局结束时，狗腿还没现身，显示狗腿玩家
				_showLackey = true;
				notifyLackeySeat(BaseUtils::EMPTY_STRING, false);
			}
			// 显示所有玩家的剩余手牌
			sendLeftCards();
			// 出完牌获胜，一局结束
			endRound();
			return true;
		}
		_current++;
		if (_current >= getMaxPlayerNums())
			_current = 0;
		if (_firstCircle) {
			if (_current == _landlord)
				_firstCircle = false;	// 回到地主出牌，第一圈结束
			else {
				bool camp = getCamp(_current);
				if (camp) {
					if (!_showCard1 && _current == _lackey) {
						// 地主阵营尚未明牌，询问狗腿是否要明牌
						_showCardDuration = 15000;
						waitShowCard();
						return true;
					}
				}
				else if (!_showCard2) {
					// 农民阵营尚未明牌，询问农民是否要明牌
					_showCardDuration = 15000;
					waitShowCard();
					return true;
				}
			}
		}
		// 通知下一位玩家出牌
		waitPlayCard();
		return true;
	}

	void LackeyRoom::autoPlayCard() {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(_current).get());
		if (avatar == nullptr)
			return ;
		std::vector<int> ids;
		if (!avatar->hasCandidate()) {
			if (_lastOut == _current) {
				std::string str;
				PokerUtilities::cardArray2String(avatar->getCards(), str);
				ErrorS << "牌桌(ID:" << getId() << ")新出牌玩家(座位:" << _current << ")无候选出牌组合,当前手牌: " << str;
				return;
			}
			doPlayCard(true, ids);
			return;
		}
		bool ret = false;
		bool test = false;
		int genre = 0;
		DouDiZhuCombination::Ptr comb = avatar->getFirstCandidate();
		if (comb) {
			genre = comb->getGenre();
			comb->getCards(ids);
		}
		if (_lastOut == _current)
			ret = doPlayCard(false, ids);
		else {
			int tmp = getRivalry(_current, _lastOut);
			int order = LackeyRule::getBombOrder(genre);
			if (tmp == 1) {
				// 敌对阵营
				if (comb->getDamages() > 0)
					test = true;
				else {
					tmp = 10;
					LackeyAvatarEx* avatar1 = nullptr;
					if (_lastOut < getMaxPlayerNums() && _lastOut > -1)
						avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(_lastOut).get());
					if (avatar1 != nullptr)
						tmp = avatar1->getCardNums();
					if ((order != -1) && (tmp > 18) && (avatar->getCardNums() > 10))
						test = true;
				}
			}
			else {
				// 未知或同阵营
				if ((order != -1) || (comb->getDamages() > 0))
					test = true;
			}
			ret = doPlayCard(test, ids);
		}
		if (!ret) {
			std::ostringstream os;
			std::string str;
			PokerUtilities::cardArray2String(avatar->getCards(), str);
			os << "牌桌(ID:" << getId() << ")玩家(座位:" << _current << ")自动出牌失败，当前手牌: " << str;
			os << "，打出的牌ID:(";
			test = true;
			std::vector<int>::const_iterator it = ids.begin();
			while (it != ids.end()) {
				if (test)
					test = false;
				else
					os << ",";
				os << *it;
				++it;
			}
			os << ")";
			LOG_ERROR(os.str());
		}
	}

	int LackeyRoom::getRivalry(int seat1, int seat2) const {
		LackeyAvatarEx* avatar2 = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat2).get());
		if (avatar2 == NULL)
			return 2;
		if (seat1 == seat2)
			return 2;
		int ret = 0;
		if (seat1 == _landlord) {
			// 玩家1为地主
			if (_landlord == _lackey)
				ret = 1;	// 地主1v4，所有其他玩家都是敌对阵营
			else if (_showLackey) {
				if (seat2 == _lackey)
					ret = 2;
				else
					ret = 1;
			}
			else if (avatar2->isShowCard())
				ret = 1;	// 已经明牌
		}
		else if (seat1 == _lackey) {
			// 玩家1为狗腿
			if (seat2 == _landlord)
				ret = 2;
			else
				ret = 1;
		}
		else {
			// 玩家1为农民
			if (seat2 == _landlord)
				ret = 1;
			else if (_showLackey) {
				if (seat2 == _lackey)
					ret = 1;
				else
					ret = 2;
			}
			else if (avatar2->isShowCard())
				ret = 2;
		}
		return ret;
	}

	void LackeyRoom::notifyPlayCardFailed(int reason, LackeyAvatarEx* avatar) const {
		MsgLackeyPlayCardFailed msg;
		msg.reason = reason;
		msg.send(avatar->getSession());
	}

	void LackeyRoom::notifyPlayCard(int seat, const std::string& playerId, int xiQian, bool realTime) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return ;
		MsgLackeyPlayCard msg;
		const PokerGenre& genre = avatar->getOuttedGenre();
		msg.seat = seat;
		msg.xiQian = xiQian;
		msg.genre = genre.getGenre();
		if (_rule->isValidGenre(genre.getGenre())) {
			msg.pass = false;
			PokerUtilities::sortCardsByPoints(genre.getCards(), msg.cards);
		}
		else
			msg.pass = true;
		msg.realTime = realTime;
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyXiQian(int seat, const std::string& playerId, bool realTime) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgLackeyXiQian msg;
		msg.seat = seat;
		msg.xiQian = avatar->getXiQian();
		msg.realTime = realTime;
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyCardNums(int seat, const std::string& playerId) const {
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(seat).get());
		if (avatar == nullptr)
			return;
		MsgLackeyCardNums msg;
		msg.seat = seat;
		msg.cardNums = avatar->getCardNums();
		if (playerId.empty())
			sendMessageToAll(msg, avatar->getPlayerId());
		else
			sendMessage(msg, playerId);
	}

	void LackeyRoom::notifyCardAlert(int seat) const {
		LackeyAvatarEx* avatar = nullptr;
		MsgLackeyCardAlert msg;
		msg.seat = seat;
		int tmp = 0;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == seat)
				continue;
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			tmp = getRivalry(i, seat);
			if (tmp != 2)
				msg.send(avatar->getSession());
		}
	}

	void LackeyRoom::onHintCard(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Playing || _operation != WaitOperation::PlayCard)
			return;
		MsgLackeyHintCard* inst = dynamic_cast<MsgLackeyHintCard*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		if (avatar->getSeat() != _current)
			return;
		MsgLackeyHintCardResp resp;
		if (!avatar->getCandidateCards(resp.cardIds)) {
			std::string str;
			PokerUtilities::cardArray2String(avatar->getCards(), str);
			ErrorS << "牌桌(ID: " << getId() << ")玩家(ID: " << inst->getPlayerId() << ")获取候选牌失败，手牌: " << str;
			return;
		}
		resp.send(netMsg->getSession());
	}

	void LackeyRoom::endRound() {
		_gameState = GameState::Waiting;

		calcScore();
		doPayment();
		sendResult();
		saveRoundRecord();
		kickAvatars();
	}

	void LackeyRoom::calcScore() {
		// 计算剩余手牌的喜钱
		LackeyAvatarEx* avatar1 = nullptr;
		LackeyAvatarEx* avatar2 = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _current)
				continue;
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar1 != nullptr)
				avatar1->calcLeftXiQian();
		}
		double tmp = 0.0;
		// 计算输赢分
		bool camp1 = getCamp(_current);		// 赢方阵营
		bool camp2 = false;
		bool camp3 = false;
		int wins = 0;	// 赢方阵营人数
		int loses = 0;	// 输方阵营人数
		int xiQian = 0;
		int xiQianScores[5] = { 0, 0, 0, 0, 0 };
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar1 == nullptr)
				continue;
			camp2 = getCamp(i);
			if (camp1 == camp2)
				wins++;
			else
				loses++;
			xiQian = avatar1->getXiQian();
			if (xiQian == 0)
				continue;
			// 一个喜钱对应0.5倍底注
			tmp = static_cast<double>(_diZhu) * 0.5 * static_cast<double>(xiQian);
			for (int j = 0; j < getMaxPlayerNums(); j++) {
				if (i == j)
					continue;
				avatar2 = dynamic_cast<LackeyAvatarEx*>(getAvatar(j).get());
				if (avatar2 == nullptr)
					continue;
				xiQianScores[i] += xiQian;
				xiQianScores[j] -= xiQian;
				avatar1->addLoseGold(j, -tmp);
				avatar2->addLoseGold(i, tmp);
			}
		}
		// 当赢方人数大于输方人数，那么赢方各得1分，输方平摊，当输方人数大于赢方人数，
		// 那么输方各付1分，赢方平摊
		float winScore = 0.0f;		// 每个赢家赢得的总分
		float loseScore = 0.0f;		// 每个输家输掉的总分
		float beiLv = 1.0f;
		if (_showCard1)
			beiLv *= 2.0f;
		if (_showCard2)
			beiLv *= 2.0f;
		// 地主1v4并且获胜则翻倍
		if (_landlord == _lackey && _landlord == _current)
			beiLv *= 2.0f;
		_beiLv = static_cast<int>(beiLv);
		if (wins > loses) {
			winScore = 1.0f * beiLv;
			loseScore = winScore * static_cast<float>(wins) / static_cast<float>(loses);
		}
		else {
			loseScore = 1.0f * beiLv;
			winScore = loseScore * static_cast<float>(loses) / static_cast<float>(wins);
		}
		// 每个输家把自己输掉的总金额平分给全部赢家
		tmp = static_cast<double>(_diZhu) * loseScore / static_cast<double>(wins);
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar1 == nullptr)
				continue;
			camp2 = getCamp(i);
			if (camp1 == camp2)
				avatar1->setScore(winScore);		// 赢方
			else
				avatar1->setScore(-loseScore);		// 输方
			avatar1->setXiQianScore(xiQianScores[i]);
			for (int j = i + 1; j < getMaxPlayerNums(); j++) {
				avatar2 = dynamic_cast<LackeyAvatarEx*>(getAvatar(j).get());
				if (avatar2 == nullptr)
					continue;
				camp3 = getCamp(j);
				if (camp2 == camp3)	// i和j都是赢家或者都是输家，双方互不相欠
					continue;
				if (camp1 == camp2) {
					// i赢j输
					avatar1->addLoseGold(j, -tmp);
					avatar2->addLoseGold(i, tmp);
				}
				else {
					// i输j赢
					avatar1->addLoseGold(j, tmp);
					avatar2->addLoseGold(i, -tmp);
				}
			}
		}
		bool test = false;
		double gold = 0.0f;
		const double* loseGolds = nullptr;
		DebtNode* node = nullptr;
		std::unordered_map<int, DebtNode*> debtNet;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar1 == nullptr)
				continue;
			test = false;
			loseGolds = avatar1->getLoseGolds();
			for (int j = 0; j < getMaxPlayerNums(); j++) {
				if (i == j || BaseUtils::real64Equals(loseGolds[j], 0.0))
					continue;
				test = true;
				break;
			}
			if (!test)
				continue;
			// 与其他玩家存在赔付关系
			gold = static_cast<double>(avatar1->getCashPledge());
			node = new DebtNode(i, gold);
			debtNet.insert(std::make_pair(i, node));
		}
		std::unordered_map<int, DebtNode*>::const_iterator it1 = debtNet.begin();
		std::unordered_map<int, DebtNode*>::const_iterator it2;
		while (it1 != debtNet.end()) {
			node = it1->second;
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(it1->first).get());
			loseGolds = avatar1->getLoseGolds();
			for (int j = 0; j < getMaxPlayerNums(); j++) {
				if ((it1->first) == j || BaseUtils::real64Equals(loseGolds[j], 0.0))
					continue;
				it2 = debtNet.find(j);
				if (it2 != debtNet.end())
					node->tally((it2->second), loseGolds[j]);
			}
			++it1;
		}
		std::string logDebt;
		DebtLiquidation dl;
		dl.printDebtNet(debtNet, logDebt);
		if (!dl(debtNet)) {
			dl.releaseDebtNet(debtNet);
			LOG_ERROR("清算结果不正确，原始债务网如下:");
			LOG_ERROR(logDebt);
			return;
		}
		test = false;
		it1 = debtNet.begin();
		while (it1 != debtNet.end()) {
			node = it1->second;
			if (node->getCapital() < 0.0) {
				test = true;
				break;
			}
			++it1;
		}
		if (test) {
			// 清算完后存在现金为负的节点
			dl.releaseDebtNet(debtNet);
			LOG_ERROR("清算之后存在负数结果，原始债务网如下:");
			LOG_ERROR(logDebt);
			return;
		}
		it1 = debtNet.begin();
		while (it1 != debtNet.end()) {
			avatar1 = dynamic_cast<LackeyAvatarEx*>(getAvatar(it1->first).get());
			node = it1->second;
			gold = static_cast<double>(avatar1->getCashPledge());
			tmp = node->getCapital();
			gold = tmp - gold;
			avatar1->setWinGold(gold);
			++it1;
		}
		dl.releaseDebtNet(debtNet);
	}

	void LackeyRoom::doPayment() {
		// 纳税额(抽水)
		double tax = 0.0;
		double delta = 0.0;
		// 纳税(抽水)比例，固定为1%
		const double rate = 0.01;
		int64_t temp = 0LL;
		int64_t cashPledge = 0LL;
		int64_t goldNeed = getCashPledge();
		int64_t diamond = 0;
		int64_t diamondNeed = getDiamondNeed();
		GameAvatar::Ptr ptr;
		LackeyAvatarEx* avatar = nullptr;
		bool test = true;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			ptr = getAvatar(i);
			avatar = dynamic_cast<LackeyAvatarEx*>(ptr.get());
			if (avatar == nullptr)
				continue;
			delta = avatar->getWinGold();
			test = true;
			if (!BaseUtils::real64Equals(delta, 0.0)) {
				if ((_mode != 0) && (delta > 0.0)) {
					temp = static_cast<int64_t>(delta);
					// 扣利模式，赢家扣利1%
					tax = rate * delta;
					delta = delta - tax;
					// 四舍五入
					delta = floor(delta + 0.5);
					// 将税收(抽水)的50%奖励给玩家的代理玩家
					rewardAgency(avatar->getPlayerId(), tax * 0.5, temp);
					avatar->setWinGold(delta, false);
				}
				cashPledge = avatar->getCashPledge();
				cashPledge += static_cast<int64_t>(delta);
				if (cashPledge < goldNeed) {
					// 押金不足，尝试从玩家金币中扣除
					test = deductCashPledge(ptr);
				}
				else {
					// 将当前押金数额保存到数据库
					updateCashPledge(avatar->getPlayerId(), cashPledge);
				}
			}
			if (!test)
				_kicks[i] = true;	// 玩家剩余金币已经低于房间最低限制，踢出
			else if (_mode == 0) {
				// 扣钻模式，检查玩家钻石是否还够下局扣除
				std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
				MysqlPool::getSingleton().syncQuery(task);
				if (task->getSucceed() && (task->getRows() > 0))
					diamond = task->getDiamond();
				else
					diamond = 0;
				if (diamondNeed > diamond)
					_kicks[i] = true;	// 玩家剩余钻石已经不足下局扣除，踢出
			}
		}
	}

	void LackeyRoom::saveRoundRecord() {
		std::stringstream ss;
		ss << "insert into `game_lackey_round` (`venue_id`, `round_no`, `bei_lv`, `landlord`, `lackey`, `time`) values(\""
			<< getId() << "\", " << _roundNo << ", " << _beiLv << ", " << _landlord << ", " << _lackey << ", now())";
		std::string sql = ss.str();
		std::shared_ptr<MysqlCommonTask> task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::InsertAI);
		// 同步执行
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed() || (task->getAffectedRecords() < 1))
			return;
		int64_t roundId = task->getAutoInc();
		LackeyAvatarEx* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			ss.str("");
			ss << "insert into `game_lackey_round_player` (`round_id`, `player_id`, `seat`, `show_card`, `score`, `xi_qian`, `win_gold`) values("
				<< roundId << ", \"" << avatar->getPlayerId() << "\", " << i << ", " << (avatar->isShowCard() ? 1 : 0)
				<< "," << avatar->getScore() << ", " << avatar->getXiQianScore() << ", " << static_cast<int>(avatar->getWinGold()) << ")";
			sql = ss.str();
			task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Insert);
			// 异步执行
			MysqlPool::getSingleton().asyncQuery(task);
		}
	}

	void LackeyRoom::kickAvatars() {
		GameAvatar::Ptr avatar;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = getAvatar(i);
			if (!avatar)
				continue;
			if (_kicks[i])
				kickAvatar(avatar);	// 将玩家踢出房间
			else
				avatar->setReady(false);
		}
	}

	void LackeyRoom::sendLeftCards() const {
		MsgLackeyLeftCards msg;
		LackeyAvatarEx* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (i == 0)
				PokerUtilities::sortCardsByPoints(avatar->getCards(), msg.cards1);
			else if (i == 1)
				PokerUtilities::sortCardsByPoints(avatar->getCards(), msg.cards2);
			else if (i == 2)
				PokerUtilities::sortCardsByPoints(avatar->getCards(), msg.cards3);
			else if (i == 3)
				PokerUtilities::sortCardsByPoints(avatar->getCards(), msg.cards4);
			else if (i == 4)
				PokerUtilities::sortCardsByPoints(avatar->getCards(), msg.cards5);
		}
		sendMessageToAll(msg);
	}

	void LackeyRoom::sendResult() const {
		MsgLackeyResult msg;
		msg.beiLv = _beiLv;
		msg.first = _current;
		LackeyAvatarEx* avatar = nullptr;
		LackeyResult result;
		std::shared_ptr<GetCapitalTask> task;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			result.score = avatar->getScore();
			result.xiQian = avatar->getXiQianScore();
			result.winGold = static_cast<int>(avatar->getWinGold());
			task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
			result.gold = avatar->getCashPledge();
			MysqlPool::getSingleton().syncQuery(task);
			if (task->getSucceed() && (task->getRows() > 0))
				result.gold += task->getGold();
			result.showCard = avatar->isShowCard();
			msg.results.push_back(result);
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(i).get());
			if (avatar != nullptr) {
				msg.kick = _kicks[i];
				msg.send(avatar->getSession());
			}
		}
	}

	void LackeyRoom::onDisbandRequest(const NetMessage::Ptr& netMsg) {
		if (_disbanding)
			return;
		MsgDisbandRequest* inst = dynamic_cast<MsgDisbandRequest*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
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

	void LackeyRoom::notifyDisbandVote(const std::string& playerId) {
		MsgLackeyDisbandVote msg;
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

	void LackeyRoom::onDisbandChoose(const NetMessage::Ptr& netMsg) {
		MsgDisbandChoose* inst = dynamic_cast<MsgDisbandChoose*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if ((inst->choice != 1) && (inst->choice != 2))
			return;
		LackeyAvatarEx* avatar = dynamic_cast<LackeyAvatarEx*>(getAvatar(inst->getPlayerId()).get());
		if (avatar != nullptr)
			doDisbandChoose(avatar->getSeat(), inst->choice);
	}

	void LackeyRoom::doDisbandChoose(int seat, int choise) {
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

	void LackeyRoom::disbandRoom() {
		// 房间解散
		MsgDisband msg;
		sendMessageToAll(msg);

		_gameState = GameState::Waiting;

		kickAllAvatars();
		// 在解散房间后游戏结束
		gameOver();
	}

	void LackeyRoom::disbandObsolete() {
		// 取消解散
		_disbanding = false;
		_endDisbandTick = BaseUtils::getCurrentMillisecond();
		MsgDisbandObsolete msg;
		sendMessageToAll(msg);
	}
}