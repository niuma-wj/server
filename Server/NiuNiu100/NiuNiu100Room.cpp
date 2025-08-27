// NiuNiu100Room.cpp

#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "Timer/TimerManager.h"
#include "MySql/MysqlPool.h"
#include "Game/RobotManager.h"
#include "Player/PlayerManager.h"
#include "Player/LoadPlayerTask.h"
#include "NiuNiuRule.h"
#include "NiuNiu100Avatar.h"
#include "NiuNiu100Room.h"
#include "NiuNiu100Messages.h"
#include "../GameDefines.h"

#include <boost/locale.hpp>
#include <sstream>

namespace NiuMa
{
	class NiuNiu100Filter : public DealFilter
	{
	public:
		NiuNiu100Filter() {}
		virtual ~NiuNiu100Filter() {}

	public:
		virtual bool isOk(const PokerCard& c) const override {
			// 不发王牌
			if (c.getPoint() == static_cast<int>(PokerPoint::Joker))
				return false;
			return true;
		}
	};

	// 6种筹码金额
	const int NiuNiu100Room::CHIP_AMOUNTS[6] = { 100, 500, 1000, 2000, 5000, 10000 };

	NiuNiu100Room::NiuNiu100Room(const std::shared_ptr<NiuNiuRule> rule,
		const std::string& venueId,
		const std::string& number,
		const std::string& bankerId,
		int64_t deposit,
		bool demo) : GameRoom(venueId, static_cast<int>(GameType::NiuNiu100), 0, RoomCategory::RoomCategoryB)
		, _rule(rule)
		, _bankerId(bankerId)
		, _gameState(GameState::None)
		, _stateTime(0LL)
		, _deposit(deposit)
		, _dealer(rule)
		, _bankerDisband(false)
		, _kickOffline(false)
		, _demoTable(demo)
		, _nextAddRobotTick(0)
		, _lastBetRobotTick(0)
	{
		_filter = std::make_shared<NiuNiu100Filter>();
		clean();
	}

	NiuNiu100Room::~NiuNiu100Room()
	{}

	GameAvatar::Ptr NiuNiu100Room::createAvatar(const std::string& playerId, int seat, bool robot) const {
		return std::make_shared<NiuNiu100Avatar>(playerId, seat, robot);
	}

	bool NiuNiu100Room::checkEnter(const std::string& playerId, std::string& errMsg, bool robot) const {
		return true;
	}

	int NiuNiu100Room::checkLeave(const std::string& playerId, std::string& errMsg) const {
		NiuNiu100Avatar* avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(playerId).get());
		if (avatar == nullptr)
			return 0;
		if (_gameState == GameState::Betting ||
			_gameState == GameState::DealCard ||
			_gameState == GameState::Compare) {
			if (avatar->getBetAmount() > 0) {
				errMsg = "当前已下注，请在本局结束之后再离开房间";
				return 1;
			}
		}
		return 0;
	}

	void NiuNiu100Room::onAvatarLeaved(int seat, const std::string& playerId) {
		// 在比牌状态时玩家离开不会改变排行榜座位
		if (_kickOffline || _gameState == GameState::Compare)
			return;
		bool test = false;
		for (int i = 0; i < 6; i++) {
			if (_rankIds[i] == playerId) {
				test = true;
				break;
			}
		}
		if (test) {
			// 排行榜座位上的玩家离开，重新排序更新排行榜座位
			refreshRank(true);
			sendRank(BaseUtils::EMPTY_STRING);
		}
	}

	void NiuNiu100Room::onTimer() {
		GameRoom::onTimer();

		if (_gameState == GameState::None) {
			beginWaitNext();
			return;
		}
		updateDemo();
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int deltaTicks = static_cast<int>(nowTick - _stateTime);
		if (GameState::WaitNext == _gameState) {
			// 等待10秒
			if (deltaTicks >= 10000) {
				// 每局开始前踢出所有不在线玩家
				_kickOffline = true;
				kickOfflinePlayers();
				removeRobot();
				_kickOffline = false;
				pushBets();
				beginBetting();
			}
		}
		else if (GameState::Betting == _gameState) {
			// 等待15秒
			if (deltaTicks >= 15000)
				beginDealCard();
		}
		else if (GameState::DealCard == _gameState) {
			// 等待4秒
			if (deltaTicks >= 4000)
				beginCompare();
		}
		else if (GameState::Compare == _gameState) {
			// 等待10秒
			if (deltaTicks >= 10000) {
				sendSettlement();
				beginWaitNext();
			}
		}
	}

	bool NiuNiu100Room::onMessage(const NetMessage::Ptr& netMsg) {
		if (GameRoom::onMessage(netMsg))
			return true;

		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgNiu100Sync::TYPE)
			onSyncTable(netMsg);
		else if (msgType == MsgNiu100Bet::TYPE)
			onBet(netMsg);
		else if (msgType == MsgNiu100GetRankList::TYPE)
			onRankList(netMsg);
		else if (msgType == MsgNiu100GetTrend::TYPE)
			onTrend(netMsg);
		else if (msgType == MsgDisbandRequest::TYPE)
			onBankerDisband(netMsg);
		else
			ret = false;

		return ret;
	}

	void NiuNiu100Room::clean() {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 6; j++)
				_chipNums[i][j] = 0;
			_betTotals[i] = 0LL;
			_bankerScores[i] = 0LL;
			_multiples[i] = 1;
		}
		for (int i = 0; i < 6; i++)
			_rankIds[i] = BaseUtils::EMPTY_STRING;
		_lastBetRobotTick = 0L;

		NiuNiu100Avatar* avatar = nullptr;
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if (avatar == nullptr) {
				++it;
				continue;
			}
			avatar->clear();
			++it;
		}
	}

	void NiuNiu100Room::pushBets() {
		NiuNiu100Avatar* avatar = nullptr;
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if (avatar != nullptr)
				avatar->pushBet();
			++it;
		}
	}

	void NiuNiu100Room::pushTrends() {
		int val = 0;
		int tmp = 0;
		for (int i = 0; i < 4; i++) {
			if (_multiples[i] < 0)
				tmp = (1 << i);
			else
				tmp = 0;
			val |= tmp;
		}
		_trends.push_back(val);
		if (_trends.size() > 20)
			_trends.erase(_trends.begin());
	}

	void NiuNiu100Room::setState(GameState s) {
		_gameState = s;
		_stateTime = BaseUtils::getCurrentMillisecond();
	}

	void NiuNiu100Room::notifyGameState(int ms, const std::string& playerId) {
		MsgNiu100GameState msg;
		msg.gameState = static_cast<int>(_gameState);
		msg.elapsed = ms;
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void NiuNiu100Room::beginWaitNext() {
		if (_bankerDisband) {
			// 房主已请求解散房间
			disbandRoom();
			return;
		}
		setState(GameState::WaitNext);
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void NiuNiu100Room::beginBetting() {
		setState(GameState::Betting);
		notifyGameState(0, BaseUtils::EMPTY_STRING);

		// 清理牌桌数据
		clean();
		refreshRank(true);
		sendRank(BaseUtils::EMPTY_STRING);
	}

	void NiuNiu100Room::beginDealCard() {
		setState(GameState::DealCard);
		notifyGameState(0, BaseUtils::EMPTY_STRING);
	}

	void NiuNiu100Room::beginCompare() {
		setState(GameState::Compare);

		_dealer.shuffle();
		CardArray cards;
		cards.reserve(5);
		int multiples[5] = { 0, 0, 0, 0, 0 };
		for (unsigned int i = 0; i < 5; i++) {
			cards.clear();
			if (!_dealer.handOutCards(cards, 5, _filter)) {
				ErrorS << "牌桌(ID:" << getId() << ")发牌失败!";
				return;
			}
			_genres[i].setCards(cards, _rule);
			multiples[i] = getGenreMultiple(_genres[i].getGenre());
		}
		int ret = 0;
		for (unsigned int i = 1; i < 5; i++) {
			ret = _rule->compareGenre(_genres[0], _genres[i]);
			if (ret == 1)
				_multiples[i - 1] = multiples[0] * -1;
			else if (ret == 2)
				_multiples[i - 1] = multiples[i];
		}
		pushTrends();
		int64_t bet = 0LL;
		int64_t betTotal = 0LL;
		int64_t score = 0LL;
		int64_t scoreTotal = 0LL;
		double tax = 0.0;
		double rate = 0.01;
		double delta = 0.0;
		int64_t cashPledge = 0LL;
		NiuNiu100Avatar* avatar = nullptr;
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if (avatar == nullptr) {
				++it;
				continue;
			}
			betTotal = 0;
			scoreTotal = 0;
			for (int i = 0; i < 4; i++) {
				bet = avatar->getBetAmount(i);
				betTotal += bet;
				score = bet * _multiples[i];
				scoreTotal += score;
				avatar->setScore(i, static_cast<int>(score));
			}
			avatar->pushWin(scoreTotal > 0);
			if (scoreTotal != 0) {
				if (scoreTotal > 0) {
					delta = static_cast<double>(scoreTotal);
					tax = rate * delta;
					delta = delta - tax;
					// 四舍五入
					delta = floor(delta + 0.5);
					if (!avatar->isRobot()) {
						// 将税收(抽水)的50%奖励给玩家的代理玩家
						rewardAgency(avatar->getPlayerId(), tax * 0.5, scoreTotal);
					}
					scoreTotal = static_cast<int64_t>(delta);
				}
				cashPledge = avatar->getCashPledge();
				cashPledge += scoreTotal;
				avatar->setCashPledge(cashPledge);
				// 将当前押金数额保存到数据库
				updateCashPledge(avatar->getPlayerId(), cashPledge);
			}
			++it;
		}
		scoreTotal = 0LL;
		for (int i = 0; i < 4; i++) {
			score = _betTotals[i] * _multiples[i];
			_bankerScores[i] = -score;
			scoreTotal += _bankerScores[i];
		}
		if (scoreTotal != 0LL)
			bankerPay(scoreTotal);
		notifyCompare(0, BaseUtils::EMPTY_STRING);
	}

	int NiuNiu100Room::getGenreMultiple(int genre) const {
		int mul = 1;
		if (genre == static_cast<int>(NiuNiuGenre::Niu7) ||
			genre == static_cast<int>(NiuNiuGenre::Niu8) ||
			genre == static_cast<int>(NiuNiuGenre::Niu9))
			mul = 2;
		else if (genre == static_cast<int>(NiuNiuGenre::NiuNiu))
			mul = 3;
		else if (genre == static_cast<int>(NiuNiuGenre::ZhaDan) ||
			genre == static_cast<int>(NiuNiuGenre::WuHua))
			mul = 4;
		return mul;
	}

	void NiuNiu100Room::bankerPay(int64_t gold) {
		if (gold > 0) {
			// 抽水
			double tax = 0.0;
			double rate = 0.01;
			double delta = static_cast<double>(gold);
			tax = rate * delta;
			delta = delta - tax;
			// 四舍五入
			delta = floor(delta + 0.5);
			if (!_demoTable) {
				// 将税收(抽水)的50%奖励给玩家的代理玩家
				rewardAgency(_bankerId, tax * 0.5, gold);
			}
			gold = static_cast<int64_t>(delta);
		}
		_deposit += gold;
		if (!_demoTable) {
			// 更新数据库中房间的押金数
			std::stringstream ss;
			ss << "update `game_niu_niu_100` set `deposit` = " << _deposit << " where `venue_id` = \'" << getId() << "\'";
			std::string sql = ss.str();
			std::shared_ptr<MysqlCommonTask> task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			MysqlPool::getSingleton().asyncQuery(task);
		}
		if (_deposit < 100000) {
			if (_demoTable) {
				// 演示房自动增加押金
				_deposit += BaseUtils::randInt(3000000, 20000001);
			}
			else
				_bankerDisband = true;	// 奖池押金已不足10万金币，自动解散
		}
	}

	void NiuNiu100Room::notifyCompare(int ms, const std::string& playerId) {
		MsgNiu100Compare msg;
		msg.elapsed = ms;
		msg.deposit = _deposit;
		for (int i = 0; i < 4; i++) {
			msg.multiples[i] = _multiples[i];
			msg.bankerScores[i] = _bankerScores[i];
		}
		NiuNiu100Avatar* avatar = nullptr;
		for (int i = 0; i < 6; i++) {
			if (_rankIds[i].empty())
				break;
			avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(_rankIds[i]).get());
			if (avatar == nullptr)
				break;
			if (i == 0) {
				for (int j = 0; j < 4; j++)
					msg.rankScores1[j] = avatar->getScore(j);
			}
			else if (i == 1) {
				for (int j = 0; j < 4; j++)
					msg.rankScores2[j] = avatar->getScore(j);
			}
			else if (i == 2) {
				for (int j = 0; j < 4; j++)
					msg.rankScores3[j] = avatar->getScore(j);
			}
			else if (i == 3) {
				for (int j = 0; j < 4; j++)
					msg.rankScores4[j] = avatar->getScore(j);
			}
			else if (i == 4) {
				for (int j = 0; j < 4; j++)
					msg.rankScores5[j] = avatar->getScore(j);
			}
			else if (i == 5) {
				for (int j = 0; j < 4; j++)
					msg.rankScores6[j] = avatar->getScore(j);
			}
			msg.rankGolds[i] = avatar->getGold() + avatar->getCashPledge();
		}
		for (int i = 0; i < 5; i++)
			msg.genres[i] = _genres[i];
		if (playerId.empty()) {
			const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
			std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
			while (it != allAvatars.end()) {
				avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
				if (avatar == nullptr || avatar->isRobot()) {
					++it;
					continue;
				}
				msg.gold = avatar->getCashPledge() + avatar->getGold();
				for (int i = 0; i < 4; i++)
					msg.scores[i] = avatar->getScore(i);
				msg.send(avatar->getSession());
				++it;
			}
			
		}
		else {
			avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(playerId).get());
			if (avatar == nullptr)
				return;
			msg.gold = avatar->getCashPledge() + avatar->getGold();
			for (int i = 0; i < 4; i++)
				msg.scores[i] = avatar->getScore(i);
			msg.send(avatar->getSession());
		}
	}

	void NiuNiu100Room::onSyncTable(const NetMessage::Ptr& netMsg) {
		MsgNiu100Sync* inst = dynamic_cast<MsgNiu100Sync*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		NiuNiu100Avatar* avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		MsgNiu100SyncResp msg;
		msg.demo = _demoTable;
		msg.gameState = static_cast<int>(_gameState);
		msg.bankerId = _bankerId;
		msg.deposit = _deposit;
		msg.gold = avatar->getGold() + avatar->getCashPledge();
		for (int i = 0; i < 4; i++) {
			msg.betTotals[i] = _betTotals[i];
			msg.myBetAmounts[i] = avatar->getBetAmount(i);
			for (int j = 0; j < 6; j++)
				msg.chipNums.push_back(_chipNums[i][j]);
		}
		if (_demoTable) {
			msg.bankerName = "百人牛牛";
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			msg.bankerName = boost::locale::conv::to_utf<char>(msg.bankerName, std::string("gb2312"));
#endif
			msg.bankerHeadImgUrl = "https://img1.baidu.com/it/u=135208876,3661679439&fm=253&fmt=auto&app=120&f=JPEG?w=500&h=500";
		}
		else {
			Player::Ptr banker = PlayerManager::getSingleton().getPlayer(_bankerId);
			if (banker) {
				msg.bankerName = banker->getNickname();
				msg.bankerHeadImgUrl = banker->getAvatar();
			}
			else {
				std::shared_ptr<LoadPlayerTask> task1 = std::make_shared<LoadPlayerTask>(_bankerId);
				MysqlPool::getSingleton().syncQuery(task1);
				if (task1->getSucceed()) {
					msg.bankerName = task1->_nickname;
					msg.bankerHeadImgUrl = task1->_avatar;
				}
			}
		}
		msg.send(netMsg->getSession());

		// 同步排行榜座位上的玩家
		sendRank(inst->getPlayerId());

		// 同步游戏状态
		if (GameState::None == _gameState)
			return;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int deltaTicks = static_cast<int>(nowTick - _stateTime);
		if (GameState::Compare == _gameState) {
			// 先发牌再比牌
			notifyCompare(deltaTicks, inst->getPlayerId());
		}
		else
			notifyGameState(deltaTicks, inst->getPlayerId());
	}

	void NiuNiu100Room::onBet(const NetMessage::Ptr& netMsg) {
		MsgNiu100Bet* inst = dynamic_cast<MsgNiu100Bet*>((netMsg->getMessage()).get());
		if (inst == nullptr)
			return;
		doBet(inst->getPlayerId(), inst->zone, inst->chip, netMsg->getSession());
	}

	void NiuNiu100Room::doBet(const std::string& playerId, int zone, int chip, const Session::Ptr& session) {
		if (zone < 0 || zone > 3)	// 下注区域不正确
			return;
		if (chip < 0 || chip > 5)	// 筹码类型不正确
			return;
		GameAvatar::Ptr ptr = getAvatar(playerId);
		NiuNiu100Avatar* avatar = dynamic_cast<NiuNiu100Avatar*>(ptr.get());
		if (avatar == nullptr)
			return;
		MsgNiu100BetFailed msg;
		if (_demoTable)
			msg.errMsg = "演示房不能下注";
		else if (GameState::Betting != _gameState)
			msg.errMsg = "当前不是下注状态";
		if (playerId == _bankerId)
			msg.errMsg = "庄家不能下注";
		else {
			int64_t cashPledge = avatar->getCashPledge();
			int64_t goldNeed = CHIP_AMOUNTS[chip] + avatar->getBetAmount();
			goldNeed *= 4;
			bool test = true;
			if (cashPledge < goldNeed) {
				// 当前押金数量不足，尝试从玩家金币中补充扣除
				test = deductCashPledge(ptr, goldNeed, false);
			}
			if (test) {
				goldNeed = CHIP_AMOUNTS[chip];
				for (int i = 0; i < 4; i++)
					goldNeed += _betTotals[i];
				goldNeed *= 4;
				if (_deposit < goldNeed)
					msg.errMsg = "总下注金额超出庄家所能赔付";
			}
			else
				msg.errMsg = "下注金额超出所能赔付，携带金币不能小于下注金额4倍";
		}
		if (msg.errMsg.empty()) {
			_chipNums[zone][chip] += 1;
			_betTotals[zone] += CHIP_AMOUNTS[chip];
			avatar->addBet(zone, CHIP_AMOUNTS[chip]);
			MsgNiu100BetSucess msg1;
			msg1.playerId = avatar->getPlayerId();
			msg1.zone = zone;
			msg1.chip = chip;
			sendMessageToAll(msg1);
		}
		else if (session) {
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			msg.errMsg = boost::locale::conv::to_utf<char>(msg.errMsg, std::string("gb2312"));
#endif
			msg.send(session);
		}
	}

	void NiuNiu100Room::sendSettlement() {
		MsgNiu100Settlement msg;
		for (int i = 0; i < 4; i++)
			msg.bankerScore += _bankerScores[i];
		bool test = false;
		NiuNiu100Avatar* avatar = nullptr;
		int score = 0;
		std::vector<int> socres;
		std::vector<std::string> ids;
		std::vector<int>::iterator it1;
		std::vector<std::string>::iterator it2;
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if ((avatar == nullptr) || (avatar->getBetAmount() == 0)) {
				++it;
				continue;
			}
			score = avatar->getScore();
			test = false;
			it1 = socres.begin();
			it2 = ids.begin();
			while (it1 != socres.end()) {
				if (score > *it1) {
					socres.insert(it1, score);
					ids.insert(it2, avatar->getPlayerId());
					test = true;
					break;
				}
				++it1;
				++it2;
			}
			if (!test) {
				socres.push_back(score);
				ids.push_back(avatar->getPlayerId());
			}
			++it;
		}
		int tmp = 0;
		for (std::size_t i = 0; i < socres.size(); i++) {
			if (ids[i] == _bankerId)	// 除去庄家
				continue;
			msg.winnerIds[tmp] = ids[i];
			msg.winnerScores[tmp] = socres[i];
			avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(ids[i]).get());
			if (avatar != nullptr) {
				msg.winnerNames[tmp] = avatar->getNickname();
				msg.winnerHeadImgUrls[tmp] = avatar->getHeadUrl();
			}
			tmp++;
			if (tmp > 3)
				break;
		}
		it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if ((avatar == nullptr) || avatar->isRobot() || (avatar->getBetAmount() == 0)) {
				++it;
				continue;
			}
			msg.score = avatar->getScore();
			msg.send(avatar->getSession());
			++it;
		}
	}

	void NiuNiu100Room::refreshRank(bool updateSeat) {
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		if (allAvatars.empty())
			return;

		if (updateSeat) {
			for (int i = 0; i < 6; i++)
				_rankIds[i] = BaseUtils::EMPTY_STRING;
		}
		_rank.clear();

		std::string maxId;
		int maxWins = 0;
		bool test = true;
		NiuNiu100Avatar* avatar = nullptr;
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if ((it->first == _bankerId) || (avatar == nullptr)) {
				++it;
				continue;
			}
			if (test || (maxWins < avatar->getAccWins20())) {
				test = false;
				maxWins = avatar->getAccWins20();
				maxId = it->first;
			}
			++it;
		}
		if (maxWins > 0) {
			if (updateSeat)
				_rankIds[0] = maxId;
			_rank.push_back(maxId);
		}
		else
			_rank.push_back(BaseUtils::EMPTY_STRING);
		int64_t bet = 0LL;
		std::vector<int64_t> bets;
		std::vector<std::string> ids;
		std::vector<int64_t>::iterator it1;
		std::vector<std::string>::iterator it2;
		it = allAvatars.begin();
		while (it != allAvatars.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
			if ((it->first == _rank[0]) || (it->first == _bankerId) || (avatar == nullptr)) {
				// 房主(庄家)和神算子不再参与排序
				++it;
				continue;
			}
			bet = avatar->getAccBets20();
			if (bet == 0LL) {
				++it;
				continue;
			}
			test = false;
			it1 = bets.begin();
			it2 = ids.begin();
			while (it1 != bets.end()) {
				if (bet > *it1) {
					bets.insert(it1, bet);
					ids.insert(it2, avatar->getPlayerId());
					test = true;
					break;
				}
				++it1;
				++it2;
			}
			if (!test) {
				bets.push_back(bet);
				ids.push_back(avatar->getPlayerId());
			}
			++it;
		}
		int tmp = 1;
		for (std::size_t i = 0; i < ids.size(); i++) {
			if (updateSeat && (tmp < 6)) {
				_rankIds[tmp] = ids[i];
				tmp++;
			}
			_rank.push_back(ids[i]);
		}
	}

	void NiuNiu100Room::sendRank(const std::string& playerId) {
		MsgNiu100Rank msg;
		NiuNiu100Avatar* avatar = nullptr;
		for (int i = 0; i < 6; i++) {
			if (_rankIds[i].empty())
				break;
			avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(_rankIds[i]).get());
			if (avatar == nullptr)
				continue;
			msg.rankIds[i] = _rankIds[i];
			msg.golds[i] = avatar->getGold() + avatar->getCashPledge();
			msg.names[i] = avatar->getNickname();
			msg.headImgUrls[i] = avatar->getHeadUrl();
		}
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void NiuNiu100Room::onRankList(const NetMessage::Ptr& netMsg) {
		MsgNiu100GetRankList* inst = dynamic_cast<MsgNiu100GetRankList*>((netMsg->getMessage()).get());
		if (inst == nullptr)
			return;
		refreshRank(false);
		if (_rank.empty())
			return;
		MsgNiu100RankList msg;
		std::set<std::string> ids;
		NiuNiu100Avatar* avatar = nullptr;
		Niu100RankItem item;
		int cnt = 0;
		std::size_t nums = _rank.size();
		for (std::size_t i = 0; i < nums; i++) {
			if (_rank[i].empty())
				avatar = nullptr;
			else
				avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(_rank[i]).get());
			if (avatar == nullptr) {
				item.playerId = "";
				item.nickname = "";
				item.headImgUrl = "";
				item.gold = 0LL;
				item.accWins20 = 0;
				item.accBets20 = 0LL;
			}
			else {
				item.playerId = _rank[i];
				item.nickname = avatar->getNickname();
				item.headImgUrl = avatar->getHeadUrl();
				item.gold = avatar->getGold() + avatar->getCashPledge();
				item.accWins20 = avatar->getAccWins20();
				item.accBets20 = avatar->getAccBets20();
			}
			ids.insert(_rank[i]);
			msg.items.push_back(item);
			cnt++;
			if (cnt > 49)
				break;
		}
		if (cnt < 50) {
			// 最多只发送50个玩家
			bool test = false;
			std::set<std::string>::const_iterator it1;
			const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
			std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
			while (it != allAvatars.end()) {
				if (it->first == _bankerId)	// 跳过庄家
					test = true;
				else {
					it1 = ids.find(it->first);
					if (it1 != ids.end())
						test = true;
				}
				if (!test) {
					avatar = dynamic_cast<NiuNiu100Avatar*>((it->second).get());
					if (avatar == nullptr)
						test = true;
				}
				if (test) {
					test = false;
					++it;
					continue;
				}
				item.playerId = it->first;
				item.accWins20 = avatar->getAccWins20();
				item.accBets20 = avatar->getAccBets20();
				item.gold = avatar->getGold() + avatar->getCashPledge();
				item.nickname = avatar->getNickname();
				item.headImgUrl = avatar->getHeadUrl();
				msg.items.push_back(item);
				cnt++;
				if (cnt > 49)
					break;
				++it;
			}
		}
		msg.send(netMsg->getSession());
	}

	void NiuNiu100Room::onTrend(const NetMessage::Ptr& netMsg) {
		MsgNiu100GetTrend* inst = dynamic_cast<MsgNiu100GetTrend*>((netMsg->getMessage()).get());
		if (inst == nullptr)
			return;
		MsgNiu100Trend msg;
		int nums = 0;
		std::vector<int>::const_reverse_iterator it = _trends.rbegin();
		while (it != _trends.rend()) {
			msg.trends.push_back(*it);
			++nums;
			if (nums > 6)
				break;
			++it;
		}
		msg.send(netMsg->getSession());
	}

	void NiuNiu100Room::onBankerDisband(const NetMessage::Ptr& netMsg) {
		MsgDisbandRequest* inst = dynamic_cast<MsgDisbandRequest*>((netMsg->getMessage()).get());
		if (inst == nullptr)
			return;
		if (inst->getPlayerId() != _bankerId)	// 非房主无权利解散
			return;
		bool old = _bankerDisband;
		_bankerDisband = true;
		if (_gameState == GameState::WaitNext) {
			// 当前在等待下局开始状态，直接解散
			disbandRoom();
			return;
		}
		MsgNiu100BankerDisband msg;
		if (old)	// 已请求解散，仅提示房主本人
			msg.send(netMsg->getSession());
		else
			sendMessageToAll(msg);
	}

	void NiuNiu100Room::disbandRoom() {
		// 通知所有玩家房间已解散
		MsgDisband msg;
		sendMessageToAll(msg);
		// 将数据库中房间的押金数量置零
		std::stringstream ss;
		ss << "update `game_niu_niu_100` set `deposit` = 0 where `venue_id` = \'" << getId() << "\'";
		std::string sql = ss.str();
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
		MysqlPool::getSingleton().asyncQuery(task);
		if (!_demoTable && (_deposit > 0)) {
			// 将房间剩余押金归还给庄家(房主)
			ss.str("");
			ss << "update `capital` set `gold` = `gold` + " << _deposit << " where `player_id` = \'" << _bankerId << "\'";
			sql = ss.str();
			task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
			MysqlPool::getSingleton().asyncQuery(task);
			_deposit = 0;
		}
		setState(GameState::None);
		// 删除所有玩家
		kickAllAvatars();
		// 结束游戏
		gameOver();
	}

	void NiuNiu100Room::kickOfflinePlayers() {
		std::vector<std::string> ids;
		const std::unordered_map<std::string, GameAvatar::Ptr>& allAvatars = getAllAvatars();
		std::unordered_map<std::string, GameAvatar::Ptr>::const_iterator it = allAvatars.begin();
		while (it != allAvatars.end()) {
			const GameAvatar::Ptr& avatar = (it->second);
			if (avatar->isRobot()) {
				++it;
				continue;
			}
			if (avatar->isOffline())
				ids.push_back(it->first);
			++it;
		}
		std::vector<std::string>::const_iterator it1 = ids.begin();
		while (it1 != ids.end()) {
			GameAvatar::Ptr avatar = getAvatar(*it1);
			kickAvatar(avatar);
			++it1;
		}
	}

	void NiuNiu100Room::updateDemo() {
		if (!_demoTable)
			return;
		addRobot();
		robotBet();
	}

	void NiuNiu100Room::addRobot() {
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		if (_nextAddRobotTick == 0)
			_nextAddRobotTick = nowTick + BaseUtils::randInt(3000, 50001);
		else if (nowTick >= _nextAddRobotTick) {
			// 5~10秒后再次添加机器人
			_nextAddRobotTick = nowTick + BaseUtils::randInt(5000, 10001);
			if (_robots.size() < 25) {
				// 添加机器人，最多仅添加25个机器人
				std::string playerId;
				if (!RobotManager::getSingleton().request(playerId))
					return;
				std::string errMsg;
				bool ret = onEnter(playerId, BaseUtils::EMPTY_STRING, errMsg);
				if (ret) {
					_robots.push_back(playerId);
					NiuNiu100Avatar* avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(playerId).get());
					if (avatar != nullptr)
						avatar->setJoinTick(nowTick);
				}
				else
					RobotManager::getSingleton().free(playerId);
			}
		}
	}

	void NiuNiu100Room::removeRobot() {
		if (!_demoTable)
			return;

		int64_t gold = 0LL;
		GameAvatar::Ptr ptr;
		std::vector<std::string>::iterator it = _robots.begin();
		while (it != _robots.end()) {
			ptr = getAvatar(*it);
			if (!ptr) {
				it = _robots.erase(it);
				continue;
			}
			if (gold < 2000) {
				// 金币不足
				kickAvatar(ptr);
				RobotManager::getSingleton().free(*it);
				it = _robots.erase(it);
			}
			else
				++it;
		}
		int nums = static_cast<int>(_robots.size());
		if (nums < 10)
			return;
		std::size_t pos = BaseUtils::randInt(0, nums);
		it = _robots.begin();
		it = it + pos;
		ptr = getAvatar(*it);
		NiuNiu100Avatar* avatar = dynamic_cast<NiuNiu100Avatar*>(ptr.get());
		if (avatar == nullptr) {
			_robots.erase(it);
			return;
		}
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		int deltaTicks = static_cast<int>(nowTick - avatar->getJoinTick());
		if (deltaTicks > 300000) {
			kickAvatar(ptr);
			RobotManager::getSingleton().free(avatar->getPlayerId());
			_robots.erase(it);
		}
	}

	void NiuNiu100Room::robotBet() {
		if (GameState::Betting != _gameState)
			return;

		// 每300毫秒驱动机器人下注一次
		int deltaTicks = 301;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		if (_lastBetRobotTick != 0)
			deltaTicks = static_cast<int>(nowTick - _lastBetRobotTick);
		if (deltaTicks < 300)
			return;
		_lastBetRobotTick = nowTick;
		bool test = false;
		time_t nextTick = 0;
		NiuNiu100Avatar* avatar = nullptr;
		std::vector<std::string>::iterator it = _robots.begin();
		while (it != _robots.end()) {
			avatar = dynamic_cast<NiuNiu100Avatar*>(getAvatar(*it).get());
			if (avatar == nullptr) {
				++it;
				continue;
			}
			nextTick = avatar->getNextBetTick();
			if (nextTick == 0)
				test = true;
			else if (nowTick >= nextTick) {
				// 机器人下注
				robotBet(*it);
				test = true;
			}
			if (test) {
				test = false;
				// 1~5秒后再下注
				deltaTicks = BaseUtils::randInt(1000, 5001);
				nextTick = nowTick + deltaTicks;
				avatar->setNextBetTick(nextTick);
			}
			++it;
		}
	}

	void NiuNiu100Room::robotBet(const std::string& playerId) {
		// 筹码越大，投注概率越低
		const int chances[6] = { 1600, 2200, 2400, 2500, 2550, 2570 };
		int chip = -1;
		int ret = BaseUtils::randInt(0, 2571);
		for (int i = 0; i < 6; i++) {
			if (ret <= chances[i]) {
				chip = i;
				break;
			}
		}
		if (chip < 0 || chip > 5)
			return;
		// 注：这里不需要判断机器人能否下注成功，因为内部已经做了相关判断，如果下注失败只是简单返回
		// 不会有任何影响
		int zone = BaseUtils::randInt(0, 4);
		doBet(playerId, zone, chip, Session::Ptr());
	}
}