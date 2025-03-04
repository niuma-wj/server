// BiJiRoom.cpp

#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "PokerUtilities.h"
#include "BiJiRule.h"
#include "BiJiAvatar.h"
#include "BiJiRoom.h"
#include "BiJiMessages.h"
#include "Game/GetCapitalTask.h"
#include "MySql/MysqlPool.h"
#include "../GameDefines.h"

#include "jsoncpp/include/json/json.h"

#include <boost/locale.hpp>
#include <sstream>

namespace NiuMa
{
	class BiJiFilter : public DealFilter
	{
	public:
		BiJiFilter(const std::shared_ptr<BiJiRoom>& room)
			: _room(room)
		{}

		virtual ~BiJiFilter() {}

	private:
		std::weak_ptr<BiJiRoom> _room;

	public:
		virtual bool isOk(const PokerCard& c) const {
			std::shared_ptr<BiJiRoom> strong = _room.lock();
			if (!strong)
				return true;
			if (c.getPoint() == static_cast<int>(PokerPoint::Joker)) {
				// 2~5人不发王牌，6人才发王牌
				if (strong->getAvatarCount() < 6)
					return false;
			}
			return true;
		}
	};

	BiJiRoom::BiJiRoom(const std::string& venueId, const std::string& number, int mode, int diZhu, const std::shared_ptr<BiJiRule>& rule)
		: GameRoom(venueId, static_cast<int>(GameType::LiuAnBiJi), 6)
		, _number(number)
		, _rule(rule)
		, _dealer(rule)
		, _mode(mode)
		, _diZhu(diZhu)
		, _commander(-1)
		, _gameState(GameState::Waiting)
		, _stateTime(0)
		, _compare(0)
		, _disbanding(false)
		, _disbander(0)
		, _beginDisbandTick(0)
		, _endDisbandTick(0)
	{
		for (int i = 0; i < 6; i++) {
			_qiPais[i] = -1;
			_kicks[i] = false;
		}
		if (_mode == 0) {
			// 固定每局每人一个钻
			setDiamondNeed(1);
		}
		// 牌桌押金为底注的10倍
		setCashPledge(diZhu * 10);
	}

	BiJiRoom::~BiJiRoom()
	{}

	void BiJiRoom::initialize() {
		GameRoom::initialize();

		_filter = std::make_shared<BiJiFilter>(std::dynamic_pointer_cast<BiJiRoom>(shared_from_this()));
	}

	void BiJiRoom::onTimer() {
		GameRoom::onTimer();

		if (_disbanding || _gameState == GameState::Waiting)
			return;
		int deltaTicks = getStateElapsed();
		if (_gameState == GameState::Dealing) {
			if (deltaTicks >= 1000)
				beginCombine();
		}
		else if (_gameState == GameState::Combining) {
			if (deltaTicks >= 30000)
				endCombine();
		}
		else if (_gameState == GameState::Comparing) {
			int criticalTick = (_compare + 1) * 1000;
			if (_compare < 3) {
				for (int i = 0; i < 3; i++) {
					if ((_compare == i) && (deltaTicks >= criticalTick)) {
						compareDun(i);
						notifyDunResult(i, BaseUtils::EMPTY_STRING);
						_compare++;
						break;
					}
				}
			}
			else if (_compare == 3) {
				if (deltaTicks >= criticalTick) {
					doAggregate();
					notifyAggregate(BaseUtils::EMPTY_STRING);
					_compare++;
				}
			}
			else if (_compare == 4) {
				if (deltaTicks >= criticalTick) {
					_compare++;
					notifySettlement();
					beginRest();
				}
			}
		}
		else if (_gameState == GameState::Resting) {
			if (deltaTicks >= 10000)
				endRest();
		}
	}

	bool BiJiRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (GameRoom::onMessage(netMsg))
			return true;

		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgBiJiSync::TYPE)
			onSyncBiJi(netMsg);
		else if (msgType == MsgPlayerReady::TYPE)
			onPlayerReady(netMsg);
		else if (msgType == MsgBiJiStartGame::TYPE)
			onStartGame(netMsg);
		else if (msgType == MsgBiJiMakeDun::TYPE)
			onMakeDun(netMsg);
		else if (msgType == MsgBiJiRevocateDun::TYPE)
			onRevocateDun(netMsg);
		else if (msgType == MsgBiJiResetDun::TYPE)
			onResetDun(netMsg);
		else if (msgType == MsgBiJiFixDun::TYPE)
			onFixDun(netMsg);
		else if (msgType == MsgBiJiGiveUp::TYPE)
			onGiveUp(netMsg);
		else
			ret = false;

		return ret;
	}

	void BiJiRoom::onConnect(const std::string& playerId) {
		GameRoom::onConnect(playerId);
		// 玩家重新上线
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(playerId).get());
		if (avatar != nullptr)
			avatar->emptyOfflines();
	}

	GameAvatar::Ptr BiJiRoom::createAvatar(const std::string& playerId, int seat, bool robot) const {
		return std::make_shared<BiJiAvatar>(playerId, seat, robot);
	}

	void BiJiRoom::onAvatarJoined(int seat, const std::string& playerId) {
		if (getAvatarCount() == 1)
			_commander = seat;
	}

	void BiJiRoom::onAvatarLeaved(int seat, const std::string& playerId) {
		// 离开成功
		int nums = getAvatarCount();
		if (nums > 0) {
			if (seat == _commander) {
				int tmp = 0;
				int maxPlayerNums = getMaxPlayerNums();
				GameAvatar::Ptr avatar;
				for (int i = 1; i < maxPlayerNums; i++) {
					tmp = (_commander + i) % maxPlayerNums;
					avatar = getAvatar(tmp);
					if (avatar) {
						_commander = tmp;
						break;
					}
				}
				notifyCommander();
			}
			if (nums == 1) {
				// 仅剩最后一个玩家没离开牌桌，牌桌进入等待状态
				setState(GameState::Waiting);
				notifyState(BaseUtils::EMPTY_STRING, 0);
			}
			if ((_gameState == GameState::Resting) && isAllReady()) {
				// 剩下的所有人都已经准备好，开始一局
				beginDeal();
			}
		}
		else {
			_commander = -1;
			// 所有玩家离开，游戏结束解散房间
			gameOver();
		}
	}

	bool BiJiRoom::checkEnter(const std::string& playerId, std::string& errMsg) const {
		// 任意时候只要还有空位都可以加入
		return true;
	}

	int BiJiRoom::checkLeave(const std::string& playerId, std::string& errMsg) const {
		if (_gameState != GameState::Waiting && _gameState != GameState::Resting) {
			BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(playerId).get());
			if (avatar && avatar->isJoinRound()) {
				errMsg = "当前牌局尚未结束，不能离开游戏";
				return 1;
			}
		}
		return 0;
	}

	void BiJiRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {
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
		BiJiAvatar* inst = dynamic_cast<BiJiAvatar*>(avatar.get());
		if (inst != nullptr)
			tmp["joinRound"] = inst->isJoinRound();
		std::string json = tmp.toStyledString();
		BaseUtils::encodeBase64(base64, json.data(), static_cast<int>(json.size()));
	}

	int BiJiRoom::getStateElapsed() const {
		int deltaTicks = 0;
		if (_disbanding) {
			// 正在投票解散，从开始投票解散之后所有游戏逻辑都停止计时
			deltaTicks = static_cast<int>(_beginDisbandTick - _stateTime);
			return deltaTicks;
		}
		long long nowTick = BaseUtils::getCurrentMillisecond();
		// 跨过中间投票解散的时间端
		if (_endDisbandTick > _stateTime)
			deltaTicks = static_cast<int>(nowTick - _endDisbandTick + _beginDisbandTick - _stateTime);
		else
			deltaTicks = static_cast<int>(nowTick - _stateTime);
		return deltaTicks;
	}

	void BiJiRoom::onSyncBiJi(const NetMessage::Ptr& netMsg) {
		MsgBiJiSync* inst = dynamic_cast<MsgBiJiSync*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == NULL)
			return;
		MsgBiJiSyncResp resp;
		resp.number = _number;
		resp.mode = _mode;
		resp.diZhu = _diZhu;
		resp.seat = avatar->getSeat();
		resp.commander = _commander;
		resp.gameState = static_cast<int>(_gameState);
		resp.disbanding = _disbanding;
		resp.send(netMsg->getSession());
		sendAvatars(netMsg->getSession());
		if (_gameState == GameState::Dealing ||
			_gameState == GameState::Combining ||
			_gameState == GameState::Comparing) {
			int deltaTicks = getStateElapsed();
			int deltaTicks1 = deltaTicks;
			if (_gameState == GameState::Combining)
				deltaTicks1 += 1000;
			if (_gameState == GameState::Comparing)
				deltaTicks1 += 30000;
			notifyDeal(inst->getPlayerId(), deltaTicks1);
			if (_gameState >= GameState::Combining) {
				deltaTicks1 = deltaTicks;
				if (_gameState == GameState::Comparing)
					deltaTicks1 += 30000;
				notifyState(inst->getPlayerId(), deltaTicks1);
				if (avatar->isJoinRound() && !(avatar->isFixed() || avatar->isGiveUp())) {
					bool dun1 = avatar->isDunOK(0);
					bool dun2 = avatar->isDunOK(1);
					bool dun3 = avatar->isDunOK(2);
					if (dun1 || dun2 || dun3)
						notifyMakeDun(avatar, false, dun1, dun2, dun3);
				}
				for (int i = 0; i < getMaxPlayerNums(); i++) {
					avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
					if (avatar == nullptr || !(avatar->isJoinRound()))
						continue;
					if (avatar->isFixed())
						notifyFixDun(false, avatar, inst->getPlayerId());
					else if (avatar->isGiveUp())
						notifyFixDun(true, avatar, inst->getPlayerId());
				}
			}
			if (_gameState == GameState::Comparing) {
				notifyState(inst->getPlayerId(), deltaTicks);
				for (int i = 0; i < 3; i++) {
					if (_compare > i)
						notifyDunResult(i, inst->getPlayerId(), false);
				}
				if (_compare > 3)
					notifyAggregate(inst->getPlayerId(), false);
			}
		}
		else if (_gameState == GameState::Resting)
			notifyState(inst->getPlayerId(), getStateElapsed());
	}

	void BiJiRoom::notifyCommander() const {
		MsgBiJiCommander msg;
		msg.commander = _commander;
		sendMessageToAll(msg);
	}

	void BiJiRoom::onStartGame(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Waiting)
			return;
		MsgBiJiStartGame* inst = dynamic_cast<MsgBiJiStartGame*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || avatar->getSeat() != _commander)	// 只有指挥官才有权力开始游戏
			return;
		int count = 0;
		MsgBiJiStartGameResp resp;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			GameAvatar::Ptr ptr = getAvatar(i);
			if (!ptr)
				continue;
			if (ptr->isReady()) {
				count++;
				continue;
			}
			if (_commander == i) {
				// 指挥官尚未就绪，自动就绪
				ptr->setReady(true);
				notifyReady(i);
				count++;
			}
			else {
				// 玩家尚未就绪，不能开始游戏
#ifdef _MSC_VER
				// VC环境下gb2312编码转utf8
				std::string str1("玩家【");
				std::string str2("】尚未准备，不能开始游戏");
				str1 = boost::locale::conv::to_utf<char>(str1, std::string("gb2312"));
				str2 = boost::locale::conv::to_utf<char>(str2, std::string("gb2312"));
				resp.errMsg = str1 + ptr->getNickname() + str2;
#else
				resp.errMsg = std::string("玩家【") + ptr->getNickname() + std::string("】尚未准备，不能开始游戏");
#endif
				resp.send(avatar->getSession());
				return;
			}
		}
		if (count < 2) {
			// 人数不足2人
			resp.errMsg = "当前房间玩家人数不足2人，不能开始游戏";
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			resp.errMsg = boost::locale::conv::to_utf<char>(resp.errMsg, std::string("gb2312"));
#endif
			resp.send(avatar->getSession());
			return;
		}
		// 进入发牌状态
		beginDeal();
	}

	void BiJiRoom::setState(GameState s) {
		_gameState = s;
		_stateTime = BaseUtils::getCurrentMillisecond();
	}

	void BiJiRoom::beginDeal() {
		// 此时正常情况下所有玩家已全部就绪
		std::stringstream ss;
		ss << "房间(ID: " << getId() << ")发牌";
		LOG_INFO(ss.str());

		_dealer.shuffle();
		CardArray cards;
		std::string str;
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			_kicks[i] = false;
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (!avatar->isReady()) {
				ss.str("");
				ss << "房间(ID: " << getId() << ")已发牌，但玩家(ID: " << avatar->getPlayerId() << ")尚未就绪，这种情况不应该出现";
				LOG_ERROR(ss.str());
				return;
			}
			if (avatar->isOffline())
				avatar->addOfflines();
			
			avatar->clear();
			avatar->setJoinRound();
			_dealer.handOutCards(cards, 9, _filter);
			avatar->dealCards(cards, _rule);
			PokerUtilities::cardArray2String(avatar->getCards(), str);
			ss.str("");
			ss << "房间(ID: " << getId() << ")发牌，玩家(ID: " << avatar->getPlayerId() << ")手牌：" << str;
			LOG_INFO(ss.str());
		}
		setState(GameState::Dealing);
		deductDiamond();
		notifyJoinRound(BaseUtils::EMPTY_STRING);
		notifyDeal(BaseUtils::EMPTY_STRING, 0);
	}

	void BiJiRoom::deductDiamond() {
		if (_mode != 0)
			return;
		MsgPlayerDiamonds msg;
		int64_t diamond = 0;
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if ((avatar == nullptr) || !avatar->isJoinRound())
				continue;
			GameRoom::deductDiamond(avatar->getPlayerId(), diamond);
			msg.seats.push_back(i);
			msg.diamonds.push_back(diamond);
		}
		sendMessageToAll(msg);
	}

	void BiJiRoom::beginCombine() {
		setState(GameState::Combining);

		notifyState(BaseUtils::EMPTY_STRING, 0);
	}

	void BiJiRoom::endCombine() {
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			// 还未确认及弃牌的玩家，自动确认
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			autoFixDun(avatar);
		}
		beginCompare();
	}

	void BiJiRoom::notifyJoinRound(const std::string& playerId) const {
		MsgBiJiJoinRound msg;
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if ((avatar != nullptr) && avatar->isJoinRound())
				msg.seats.push_back(i);
		}
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void BiJiRoom::notifyDeal(const std::string& playerId, int elapsed) const {
		MsgBiJiDealCard msg;
		msg.elapsed = elapsed;
		BiJiAvatar* avatar = NULL;
		if (playerId.empty()) {
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
				if (avatar == NULL)
					continue;
				if (avatar->isJoinRound()) {
					msg.cards = avatar->getCards();
					avatar->getOrderRaw(msg.orderRaw);
					avatar->getOrderSuit(msg.orderSuit);
				}
				sendMessage(msg, avatar->getPlayerId());
			}
		}
		else {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(playerId).get());
			if (avatar == NULL)
				return;
			if (avatar->isJoinRound()) {
				msg.cards = avatar->getCards();
				avatar->getOrderRaw(msg.orderRaw);
				avatar->getOrderSuit(msg.orderSuit);
			}
			sendMessage(msg, playerId);
		}
	}

	void BiJiRoom::notifyState(const std::string& playerId, int elapsed) const {
		MsgBiJiGameState msg;
		msg.gameState = static_cast<int>(_gameState);
		msg.elapsed = elapsed;
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void BiJiRoom::onMakeDun(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Combining)
			return;
		MsgBiJiMakeDun* inst = dynamic_cast<MsgBiJiMakeDun*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		bool dun11 = avatar->isDunOK(0);
		bool dun21 = avatar->isDunOK(1);
		bool dun31 = avatar->isDunOK(2);
		if (!avatar->makeDun(inst->dun, inst->cardIds, _rule))
			return;
		bool dun12 = avatar->isDunOK(0);
		bool dun22 = avatar->isDunOK(1);
		bool dun32 = avatar->isDunOK(2);
		notifyMakeDun(avatar, true, (!dun11 && dun12), (!dun21 && dun22), (!dun31 && dun32));
		// 每次配墩之后都对墩进行排序
		autoSortDun(avatar);
	}

	void BiJiRoom::combineOver() {
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr || !avatar->isJoinRound())
				continue;
			if (!(avatar->isFixed() || avatar->isGiveUp()))
				return;
		}
		// 全部确认或弃牌
		beginCompare();
	}

	void BiJiRoom::autoFixDun(BiJiAvatar* avatar) {
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		bool dunOKs1[3] = { false, false, false };
		for (int i = 0; i < 3; i++)
			dunOKs1[i] = avatar->isDunOK(i);
		if (!dunOKs1[0] || !dunOKs1[1] || !dunOKs1[2]) {
			int ids[3] = { 0, 0, 0 };
			const BiJiGenre GENRES[6] = { BiJiGenre::Triple, BiJiGenre::FlushStraight, BiJiGenre::Flush, BiJiGenre::Straight, BiJiGenre::Pair, BiJiGenre::Single };
			for (int i = 2; i > -1; i--) {
				if (avatar->isDunOK(i))
					continue;
				avatar->checkSupportGenre(_rule);
				for (int j = 0; j < 6; j++) {
					if (!avatar->isSupportGenre(GENRES[j]))
						continue;
					if (!avatar->getGenreIds(GENRES[j], ids, _rule)) {
						ErrorS << "玩家(ID: " << avatar->getPlayerId() << ")获取牌型(" << static_cast<int>(GENRES[j]) << ")牌ID失败";
					}
					if (!avatar->makeDun(i, ids, _rule)) {
						ErrorS << "玩家(ID: " << avatar->getPlayerId() << ")自动配墩失败";
					}
					break;
				}
			}
		}
		bool duns[3] = { false, false, false };
		avatar->sortDuns(duns, _rule);
		avatar->setFixed();
		notifyFixDun(false, avatar, BaseUtils::EMPTY_STRING);
	}

	void BiJiRoom::autoSortDun(BiJiAvatar* avatar) {
		if (avatar == nullptr || avatar->isFixed() || avatar->isGiveUp())
			return;
		bool duns[3] = { false, false, false };
		if (!avatar->sortDuns(duns, _rule))
			return;
		MsgBiJiSortDun msg;
		for (int i = 0; i < 3; i++)
			msg.duns[i] = duns[i];
		if (duns[0] && !avatar->getDunIds(0, msg.dunIds1))
			return;
		if (duns[1] && !avatar->getDunIds(1, msg.dunIds2))
			return;
		if (duns[2] && !avatar->getDunIds(2, msg.dunIds3))
			return;
		msg.send(avatar->getSession());
	}

	void BiJiRoom::notifyMakeDun(BiJiAvatar* avatar, bool animate, bool dun1, bool dun2, bool dun3) const {
		if (avatar == nullptr)
			return;
		MsgBiJiMakeDunResp resp;
		resp.animate = animate;
		resp.duns[0] = dun1;
		resp.duns[1] = dun2;
		resp.duns[2] = dun3;
		if (dun1 && !avatar->getDunIds(0, resp.dunIds1))
			return;
		if (dun2 && !avatar->getDunIds(1, resp.dunIds2))
			return;
		if (dun3 && !avatar->getDunIds(2, resp.dunIds3))
			return;
		resp.send(avatar->getSession());
	}

	void BiJiRoom::onRevocateDun(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Combining)
			return;
		MsgBiJiRevocateDun* inst = dynamic_cast<MsgBiJiRevocateDun*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		int ids[3] = { 0, 0, 0 };
		if (!avatar->getDunIds(inst->dun, ids))
			return;
		if (!avatar->revocateDun(inst->dun))
			return;
		MsgBiJiRevocateDunResp msg;
		msg.dun = inst->dun;
		for (int i = 0; i < 3; i++)
			msg.cardIds[i] = ids[i];
		msg.send(avatar->getSession());
	}

	void BiJiRoom::onResetDun(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Combining)
			return;
		MsgBiJiResetDun* inst = dynamic_cast<MsgBiJiResetDun*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		int ids1[3] = { 0, 0, 0 };
		int ids2[3] = { 0, 0, 0 };
		int ids3[3] = { 0, 0, 0 };
		if (!(avatar->isDunOK(0) && avatar->getDunIds(0, ids1) && avatar->revocateDun(0)))
			return;
		if (!(avatar->isDunOK(1) && avatar->getDunIds(1, ids2) && avatar->revocateDun(1)))
			return;
		if (!(avatar->isDunOK(2) && avatar->getDunIds(2, ids3) && avatar->revocateDun(2)))
			return;
		MsgBiJiResetDunResp resp;
		for (int i = 0; i < 3; i++)
			resp.cardIds[i] = ids1[i];
		for (int i = 0; i < 3; i++)
			resp.cardIds[3 + i] = ids2[i];
		for (int i = 0; i < 3; i++)
			resp.cardIds[6 + i] = ids3[i];
		resp.send(avatar->getSession());
	}

	void BiJiRoom::onFixDun(const NetMessage::Ptr& netMsg)
	{
		if (_gameState != GameState::Combining)
			return;
		MsgBiJiFixDun* inst = dynamic_cast<MsgBiJiFixDun*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		for (int i = 0; i < 3; i++) {
			if (!avatar->isDunOK(i))
				return;
		}
		avatar->setFixed();
		notifyFixDun(false, avatar, BaseUtils::EMPTY_STRING);

		combineOver();
	}

	void BiJiRoom::onGiveUp(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Combining)
			return;
		MsgBiJiGiveUp* inst = dynamic_cast<MsgBiJiGiveUp*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr || !avatar->isJoinRound() || avatar->isFixed() || avatar->isGiveUp())
			return;
		for (int i = 0; i < 6; i++) {
			if (_qiPais[i] != -1)
				continue;

			_qiPais[i] = avatar->getSeat();
			avatar->setQiPaiOrder(i);
			break;
		}
		notifyFixDun(true, avatar, BaseUtils::EMPTY_STRING);

		combineOver();
	}

	void BiJiRoom::notifyFixDun(bool qiPai, BiJiAvatar* avatar, const std::string& targetId) const {
		if (avatar == nullptr)
			return;
		MsgBiJiFixDunResp resp;
		resp.seat = avatar->getSeat();
		resp.qiPai = qiPai;
		if (targetId.empty())
			sendMessageToAll(resp, avatar->getPlayerId());
		else if (targetId != avatar->getPlayerId())
			sendMessage(resp, targetId);
		if (targetId.empty() || targetId == avatar->getPlayerId()) {
			if (!qiPai) {
				int ids[3] = { 0, 0, 0 };
				avatar->getDunIds(0, ids);
				for (int i = 0; i < 3; i++)
					resp.cardIds[i] = ids[i];
				avatar->getDunIds(1, ids);
				for (int i = 0; i < 3; i++)
					resp.cardIds[3 + i] = ids[i];
				avatar->getDunIds(2, ids);
				for (int i = 0; i < 3; i++)
					resp.cardIds[6 + i] = ids[i];
			}
			resp.send(avatar->getSession());
		}
	}

	void BiJiRoom::beginCompare() {
		setState(GameState::Comparing);

		_compare = 0;
		notifyState(BaseUtils::EMPTY_STRING, 0);
	}

	void BiJiRoom::beginRest() {
		setState(GameState::Resting);

		BiJiAvatar* avatar = NULL;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (_kicks[i]) {
				// 将玩家踢出房间
				kickAvatar(getAvatar(i));
			}
			else if (avatar->isJoinRound())
				avatar->setReady(false);
		}
		notifyState(BaseUtils::EMPTY_STRING, 0);
	}

	void BiJiRoom::endRest() {
		// 结束休息
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (!avatar->isReady()) {
				// 休息时间结束自动就绪
				avatar->setReady(true);
				notifyReady(i);
			}
		}
		// 开始一局
		beginDeal();
	}

	void BiJiRoom::compareDun(int dun) {
		if (dun < 0 || dun > 2)
			return;
		int wins[6] = { 0, 0, 0, 0, 0, 0 };		// 所有玩家的赢数
		int loses[6] = { 0, 0, 0, 0, 0, 0 };	// 所有玩家的输数
		int score = 0;
		int ret = 0;
		bool qiPai1 = false;
		bool qiPai2 = false;
		BiJiAvatar* avatar1 = nullptr;
		BiJiAvatar* avatar2 = nullptr;
		std::string str1;
		std::string str2;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar1 == nullptr || !avatar1->isJoinRound())
				continue;
			qiPai1 = avatar1->isGiveUp();
			const PokerGenre& genre1 = avatar1->getDun(dun);
			for (int j = i + 1; j < getMaxPlayerNums(); j++) {
				avatar2 = dynamic_cast<BiJiAvatar*>(getAvatar(j).get());
				if (avatar2 == nullptr || !avatar2->isJoinRound())
					continue;

				ret = 0;
				qiPai2 = avatar2->isGiveUp();
				const PokerGenre& genre2 = avatar2->getDun(dun);
				if (qiPai1) {
					if (qiPai2) {
						if (avatar1->getQiPaiOrder() > avatar2->getQiPaiOrder())
							ret = 1;	// 先弃牌的输分
						else
							ret = 2;
					}
					else
						ret = 2;
				}
				else if (qiPai2)
					ret = 1;
				else
					ret = _rule->compareGenre(genre1, genre2);
				if (ret == 1) {
					wins[i]++;
					loses[j]++;
				}
				else if (ret == 2) {
					loses[i]++;
					wins[j]++;
				}
				else {
					genre1.card2String(str1);
					genre2.card2String(str2);
					ErrorS << "逻辑错误，牌型(" << str1 << ")与牌型(" << str2 << ")比较结果不正确";
					return;
				}
			}
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar1 == nullptr || !avatar1->isJoinRound())
				continue;

			if (loses[i] == 0) {
				score = 0;
				for (int j = 0; j < wins[i]; j++)
					score += (j + 1);
			}
			else
				score = -loses[i];
			avatar1->setDunScore(dun, score);
		}
	}

	void BiJiRoom::doAggregate() {
		int rewards[6] = { 0, 0, 0, 0, 0, 0 };
		int score = 0;
		int rewardType = 0;
		BiJiAvatar* avatar1 = nullptr;
		BiJiAvatar* avatar2 = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar1 == nullptr || !avatar1->isJoinRound() || avatar1->isGiveUp())
				continue;

			score = 0;
			avatar1->detectRewardType(_rule);
			rewardType = avatar1->getRewardType();
			if ((rewardType & static_cast<int>(BiJiRewardType::TongGuan)) != 0)				// 通关
				score += 2;
			if ((rewardType & static_cast<int>(BiJiRewardType::QuanSanTiao)) != 0)			// 全三条
				score += 1;
			if ((rewardType & static_cast<int>(BiJiRewardType::QuanShuanZi)) != 0)			// 全顺子
				score += 1;
			if ((rewardType & static_cast<int>(BiJiRewardType::QuanHeiSe)) != 0)			// 全黑色
				score += 1;
			if ((rewardType & static_cast<int>(BiJiRewardType::QuanHongSe)) != 0)			// 全红色
				score += 1;
			if ((rewardType & static_cast<int>(BiJiRewardType::ShuangTongHuaShun)) != 0)	// 双同花顺
				score += 1;
			if (score == 0)
				continue;
			for (int j = 0; j < getMaxPlayerNums(); j++) {
				if (i == j)
					continue;
				avatar2 = dynamic_cast<BiJiAvatar*>(getAvatar(j).get());
				if (avatar2 == nullptr || !avatar2->isJoinRound() || avatar2->isGiveUp())
					continue;
				rewards[i] += score;
				rewards[j] -= score;
			}
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar1 != nullptr && avatar1->isJoinRound())
				avatar1->setReward(rewards[i]);
		}
		// 纳税额(抽水)
		double tax = 0.0;
		double delta = 0.0;
		// 纳税(抽水)比例，固定为1%
		const double rate = 0.01;
		int64_t winGold = 0;
		int64_t goldNeed = getCashPledge();
		int64_t diamond = 0;
		int64_t diamondNeed = 0;
		int64_t cashPledge = 0LL;
		if (_mode == 0)
			diamondNeed = getDiamondNeed();
		bool test = true;
		GameAvatar::Ptr ptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			ptr = getAvatar(i);
			avatar1 = dynamic_cast<BiJiAvatar*>(ptr.get());
			if (avatar1 == nullptr || !avatar1->isJoinRound())
				continue;
			score = avatar1->getTotal();
			winGold = score * _diZhu;
			test = true;
			if (winGold != 0LL) {
				if (_mode != 0 && winGold > 0LL) {
					delta = static_cast<double>(winGold);
					// 抽水
					tax = rate * delta;
					delta = delta - tax;
					// 四舍五入
					delta = floor(delta + 0.5);
					// 将税收(抽水)的50%奖励给玩家的代理玩家
					rewardAgency(avatar1->getPlayerId(), tax * 0.5, winGold);
					winGold = static_cast<int64_t>(delta);
				}
				avatar1->setWinGold(winGold);
				cashPledge = avatar1->getCashPledge();
				cashPledge += winGold;
				if (cashPledge < goldNeed) {
					// 押金不足，尝试从玩家金币中扣除
					test = deductCashPledge(ptr);
				}
				else {
					// 将当前押金数额保存到数据库
					updateCashPledge(avatar1->getPlayerId(), cashPledge);
				}
			}
			if (!test) {
				// 玩家剩余金币已经低于房间最低限制，踢出
				_kicks[i] = true;
			}
			else if (avatar1->getOfflines() > 2) {
				// 已经离线玩了3局，将玩家踢出房间
				_kicks[i] = true;
			}
			if (!_kicks[i] && (_mode == 0)) {
				// 扣钻模式，检查玩家钻石是否还够下局扣除
				std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(avatar1->getPlayerId());
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

	void BiJiRoom::notifyDunResult(int dun, const std::string& playerId, bool animate) const {
		if (dun < 0 || dun > 2)
			return;

		MsgBiJiDunResult msg;
		msg.dun = dun;
		msg.animate = animate;
		unsigned int nums = 0;
		DunResult dr;
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr || !avatar->isJoinRound())
				continue;

			dr.seat = i;
			if (!avatar->isGiveUp()) {
				const PokerGenre& genre = avatar->getDun(dun);
				dr.genre = genre.getGenre();
				const CardArray& cards = genre.getCards();
				nums = static_cast<unsigned int>(cards.size());
				if (nums > 3)
					nums = 3;
				for (unsigned int j = 0; j < nums; j++)
					dr.cards[j] = cards[j].toInt32();
			}
			dr.score = avatar->getDunScore(dun);
			msg.results.push_back(dr);
		}
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void BiJiRoom::notifyAggregate(const std::string& playerId, bool animate) const {
		MsgBiJiAggregate msg;
		msg.animate = animate;
		BiJiScore score;
		BiJiAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr || !avatar->isJoinRound())
				continue;
			score.seat = i;
			score.reward = avatar->getReward();
			score.total = avatar->getTotal();
			score.rewardType = avatar->getRewardType();
			msg.scores.push_back(score);
		}
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void BiJiRoom::notifySettlement() {
		MsgBiJiSettlement msg;
		BiJiSettlement settlement;
		BiJiAvatar* avatar = nullptr;
		unsigned int nums = 0;
		std::shared_ptr<GetCapitalTask> task;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr || !avatar->isJoinRound())
				continue;
			settlement.seat = i;
			for (int j = 0; j < 3; j++)
				settlement.dunScores[j] = avatar->getDunScore(j);
			settlement.totalScore = avatar->getTotal();
			settlement.winGold = avatar->getWinGold();
			settlement.gold = avatar->getCashPledge();
			task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
			MysqlPool::getSingleton().syncQuery(task);
			if (task->getSucceed() && task->getRows() > 0)
				settlement.gold += task->getGold();
			settlement.qiPai = avatar->isGiveUp();
			if (!settlement.qiPai) {
				settlement.rewardType = avatar->getRewardType();
				for (unsigned int j = 0; j < 3; j++) {
					const PokerGenre& genre = avatar->getDun(j);
					const CardArray& cards = genre.getCards();
					nums = static_cast<unsigned int>(cards.size());
					if (nums > 3)
						nums = 3;
					for (unsigned int k = 0; k < nums; k++)
						settlement.cards[j * 3 + k] = cards[k].toInt32();
					settlement.genres[j] = genre.getGenre();
				}
			}
			msg.settlements.push_back(settlement);
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<BiJiAvatar*>(getAvatar(i).get());
			if (avatar == nullptr || !avatar->isJoinRound())
				continue;
			msg.kick = _kicks[i];
			msg.send(avatar->getSession());
		}
	}

	void BiJiRoom::onPlayerReady(const NetMessage::Ptr& netMsg) {
		if (_gameState != GameState::Waiting &&
			_gameState != GameState::Resting)
			return;
		MsgPlayerReady* inst = dynamic_cast<MsgPlayerReady*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		BiJiAvatar* avatar = dynamic_cast<BiJiAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		avatar->setReady(true);
		notifyReady(avatar->getSeat());
		if ((_gameState == GameState::Resting) && isAllReady()) {
			// 所有人都已经准备好，开始一局
			beginDeal();
		}
	}

	void BiJiRoom::notifyReady(int seat) const {
		MsgPlayerReadyResp msg;
		msg.seat = seat;
		sendMessageToAll(msg);
	}
}