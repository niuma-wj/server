// StandardMahjongRoom.cpp

#include "Base/BaseUtils.h"
#include "Base/Log.h"
#include "Game/GetCapitalTask.h"
#include "MySql/MysqlPool.h"
#include "Venue/VenueInnerHandler.h"
#include "StandardMahjongAvatar.h"
#include "StandardMahjongRoom.h"
#include "StandardMahjongMessages.h"
#include "StandardMahjongPlayback.h"
#include "StandardMahjongRecordTask.h"
#include "Game/GameMessages.h"
#include "Game/DebtLiquidation.h"
#include "Player/PlayerManager.h"
#include "jsoncpp/include/json/json.h"

#include <sstream>
#include <zlib.h>

#include <mysql/jdbc.h>

namespace NiuMa
{
	const int StandardMahjongRoom::RULER_TABLE[6] = { 0x01, 0x02, 0x03, 0x12, 0x13, 0x23 };

	StandardMahjongRoom::StandardMahjongRoom(const MahjongRule::Ptr& rule, const std::string& venueId, const std::string& number, int mode, int diZhu, int config)
		: MahjongRoom(rule, venueId, static_cast<int>(GameType::Mahjong))
		, _number(number)
		, _mode(mode)
		, _diZhu(diZhu)
		, _config(config)
		, _roundState(StageState::NotStarted)
		, _disbandState(StageState::NotStarted)
		, _roundNo(0)
		, _backupBanker(0)
		, _disbander(0)
		, _disbandTick(0)
	{
		_anGangVisible = true;
		_chi = ((_config & 0x01) != 0);
		_dianPao = ((_config & 0x02) != 0);
		_anGangVisible = true; // ((_config & 0x04) != 0);

		for (int i = 0; i < 6; i++)
			_distances[i] = -1;
		for (int i = 0; i < 4; i++) {
			_disbandChoices[i] = 0;
			_kicks[i] = false;
		}
		// 押金数额为底注的50倍
		setCashPledge(_diZhu * 50);
		if (_mode == 0) {
			// 扣钻模式，每局需扣除4个钻石
			setDiamondNeed(4);
		}
	}

	StandardMahjongRoom::~StandardMahjongRoom()
	{}

	GameAvatar::Ptr StandardMahjongRoom::createAvatar(const std::string& playerId, int seat, bool robot) const {
		return std::make_shared<StandardMahjongAvatar>(playerId, seat, robot);
	}

	void StandardMahjongRoom::onAvatarLeaved(int seat, const std::string& playerId) {
		clearDistances(seat, _distances);
		if (getAvatarCount() == 0)
			gameOver();
	}

	bool StandardMahjongRoom::checkEnter(const std::string& playerId, std::string& errMsg, bool robot) const {
		if (_roundState == StageState::Underway) {
			errMsg = "游戏正在进行中，不能进入房间";
			return false;
		}
		return true;
	}

	int StandardMahjongRoom::checkLeave(const std::string& playerId, std::string& errMsg) const {
		if (_roundState == StageState::Underway) {
			errMsg = "游戏正在进行中，不能离开房间";
			return 1;
		}
		return 0;
	}

	void StandardMahjongRoom::getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const {
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
			if (session)
				tmp["ip"] = session->getRemoteIp();
		}
		std::string json = tmp.toStyledString();
		BaseUtils::encodeBase64(base64, json.data(), static_cast<int>(json.size()));
	}

	void StandardMahjongRoom::clean() {
		MahjongRoom::clean();

		for (int i = 0; i < 4; i++)
			_kicks[i] = false;
	}

	bool StandardMahjongRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (MahjongRoom::onMessage(netMsg))
			return true;

		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgMahjongSync::TYPE)
			onSyncMahjong(netMsg);
		else if (msgType == MsgPlayerReady::TYPE)
			onPlayerReady(netMsg);
		else if (msgType == MsgDisbandRequest::TYPE)
			onDisbandRequest(netMsg);
		else if (msgType == MsgDisbandChoose::TYPE)
			onDisbandChoose(netMsg);
		else
			ret = false;

		return ret;
	}

	void StandardMahjongRoom::onTimer() {
		if (_disbandState == StageState::Underway) {
			time_t nowTick = BaseUtils::getCurrentMillisecond();
			int deltaTicks = static_cast<int>(nowTick - _disbandTick);
			if (deltaTicks > 300000) {
				// 超过300秒(5分钟)不选择默认同意解散
				for (int i = 0; i < 4; i++) {
					if (_disbandChoices[i] == 0)
						doDisbandChoose(i, 1);
				}
			}
		}
	}

	void StandardMahjongRoom::onSyncMahjong(const NetMessage::Ptr& netMsg) {
		MsgMahjongSync* inst = dynamic_cast<MsgMahjongSync*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		StandardMahjongAvatar* avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		std::shared_ptr<GetCapitalTask> task = std::make_shared<GetCapitalTask>(inst->getPlayerId());
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed() || (task->getRows() < 1)) {
			ErrorS << "查询玩家资产失败，场地Id: " << getId() << ", 玩家Id: " << inst->getPlayerId();
			return;
		}
		MsgMahjongSyncResp msg;
		msg.number = _number;
		msg.gold = task->getGold();
		msg.diamond = task->getDiamond();
		msg.mode = _mode;
		msg.diZhu = _diZhu;
		msg.chi = _chi;
		msg.dianPao = _dianPao;
		msg.seat = avatar->getSeat();
		msg.roundState = static_cast<int>(_roundState);
		msg.disbandState = static_cast<int>(_disbandState);
		msg.banker = _banker;
		msg.leftTiles = _dealer.getTileLeft();
		StandardMahjongAvatar* tmpAvatar = NULL;
		if (_roundState == StageState::Underway) {
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				tmpAvatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
				if (tmpAvatar == NULL)
					continue;
				if (i == avatar->getSeat()) {
					msg.handTiles = tmpAvatar->getTiles();
					if ((i == _actor) && !_actions.empty()) {
						const MahjongAction& ma = _actions.back();
						if (ma.getType() == MahjongAction::Type::Fetch) {
							// 最新的动作为摸牌
							int tileId = tmpAvatar->getFetchedTileId();
							msg.hasFetch = true;
							msg.fetchTile.setId(tileId);
							_dealer.getTileById(msg.fetchTile);
						}
					}
				}
				msg.handTileNums[i] = tmpAvatar->getTileNums();
				tmpAvatar->getPlayedTilesNoAction(msg.playedTiles[i]);
				msg.chapters[i] = tmpAvatar->getChapters();
			}
		}
		msg.send(netMsg->getSession());
		sendAvatars(netMsg->getSession());
		if (_roundState == StageState::Underway) {
			notifyActorUpdated(inst->getPlayerId());
			if ((_state == StateMachine::Action) || (_state == StateMachine::Play))
				notifyWaitingAction(inst->getPlayerId());	// 通知牌桌处于等待状态
			if (avatar->hasActionOption())
				notifyActionOptions(avatar);
			notifyTingTile(avatar);
			if (_disbandState == StageState::Underway)
				notifyDisbandVote(inst->getPlayerId());
		}
	}

	void StandardMahjongRoom::onPlayerReady(const NetMessage::Ptr& netMsg) {
		if (_roundState != StageState::NotStarted)
			return;
		MsgPlayerReady* inst = dynamic_cast<MsgPlayerReady*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		StandardMahjongAvatar* avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == nullptr)
			return;
		avatar->setReady(true);
		MsgPlayerReadyResp msg;
		msg.playerId = avatar->getPlayerId();
		msg.seat = avatar->getSeat();
		sendMessageToAll(msg);
		if (isFull() && isAllReady()) {
			// 所有人都已经准备好，开始一局
			startRound();
		}
	}

	void StandardMahjongRoom::startRound() {
		// 开始一局
		_roundState = StageState::Underway;
		_backupBanker = _banker;

		clean();

		// 扣除钻石
		int64_t diamonds[4] = { 0LL };
		deductDiamond(diamonds);

		MsgMahjongStartRound msg;
		msg.banker = _banker;
		std::ostringstream os;
		os << "麻将牌桌(Id:" << getId() << ")开局，各玩家Id: ";
		StandardMahjongAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			if (i > 0)
				os << "、";
			os << avatar->getPlayerId();
			if (_mode == 0)
				msg.diamond = static_cast<int>(diamonds[i]);
			msg.send(avatar->getSession());
		}
		LOG_INFO(os.str());

		dealTiles();

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
					ss << "select max(`round_no`) from `game_mahjong_record` where `venue_id` = \"" << _venueId << "\"";
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

	void StandardMahjongRoom::deductDiamond(int64_t diamonds[4]) {
		if (_mode != 0)
			return;
		StandardMahjongAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar != NULL)
				GameRoom::deductDiamond(avatar->getPlayerId(), diamonds[i]);
		}
	}

	double* StandardMahjongRoom::getDistances() {
		return _distances;
	}

	void StandardMahjongRoom::getDistances(std::vector<int>& distances) const {
		for (int i = 0; i < 6; i++)
			distances.push_back(static_cast<int>(_distances[i]));
	}

	int StandardMahjongRoom::getDistanceIndex(int seat1, int seat2) const {
		int i1 = seat1;
		int i2 = seat2;
		if (seat1 > seat2) {
			i1 = seat2;
			i2 = seat1;
		}
		int i3 = (i1 << 4) | i2;
		int ret = -1;
		for (int j = 0; j < 6; j++) {
			if (i3 == RULER_TABLE[j]) {
				ret = j;
				break;
			}
		}
		return ret;
	}

	void StandardMahjongRoom::calcHuScore() const {
		// 流局不算分，即便是杠分
		if (!_hu)
			return;
		int seat = 0;
		int score = 0;
		int scores[4] = { 0, 0, 0, 0 };
		StandardMahjongAvatar* avatar1 = nullptr;
		StandardMahjongAvatar* avatar2 = nullptr;
		MahjongChapter::Type chapterType = MahjongChapter::Type::Invalid;
		MahjongChapterArray::const_iterator it;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar1 == nullptr)
				continue;
			// 算胡分
			if (avatar1->isHu()) {
				score = avatar1->calcHuScore();
				if (avatar1->isDianPao()) {
					// 点炮
					avatar2 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(_actor).get());
					if (avatar2 != nullptr) {
						avatar1->addLoseScore(_actor, -score);
						avatar2->addLoseScore(i, score);
					}
				} else {
					// 自摸
					for (int j = 0; j < getMaxPlayerNums(); j++) {
						if (i == j)
							continue;
						avatar2 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(j).get());
						if (avatar2 == nullptr)
							continue;
						avatar1->addLoseScore(j, -score);
						avatar2->addLoseScore(i, score);
					}
				}
			}
			// 算杠分
			const MahjongChapterArray& lstChapters = avatar1->getChapters();
			it = lstChapters.begin();
			while (it != lstChapters.end()) {
				if (it->isVetoed()) {
					++it;
					continue;
				}
				chapterType = it->getType();
				if (chapterType == MahjongChapter::Type::ZhiGang) {
					seat = it->getTargetPlayer();
					avatar2 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(seat).get());
					if (avatar2 != nullptr) {
						avatar1->addLoseScore(seat, -1);
						avatar2->addLoseScore(i, 1);
					}
				}
				else if (chapterType == MahjongChapter::Type::JiaGang) {
					for (int j = 0; j < getMaxPlayerNums(); j++) {
						if (i == j)
							continue;
						avatar2 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(j).get());
						if (avatar2 == nullptr)
							continue;
						avatar1->addLoseScore(j, -1);
						avatar2->addLoseScore(i, 1);
					}
				}
				else if (chapterType == MahjongChapter::Type::AnGang) {
					for (int j = 0; j < getMaxPlayerNums(); j++) {
						if (i == j)
							continue;
						avatar2 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(j).get());
						if (avatar2 == NULL)
							continue;
						avatar1->addLoseScore(j, -2);
						avatar2->addLoseScore(i, 2);
					}
				}
				++it;
			}
		}
		int loseScores[4] = { 0 };
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar1 == NULL)
				continue;
			avatar1->getLoseScores(loseScores);
			for (int j = 0; j < 4; j++) {
				if (i != j)
					scores[j] += loseScores[j];
			}
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar1 != NULL)
				avatar1->setScore(scores[i]);
		}
		bool test = false;
		double diZhu = _diZhu;
		double capital = 0.0f;
		DebtNode* node = NULL;
		std::unordered_map<int, DebtNode*> debtNet;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar1 == NULL)
				continue;
			test = false;
			avatar1->getLoseScores(loseScores);
			for (int j = 0; j < 4; j++) {
				if ((i == j) || (loseScores[j] == 0))
					continue;
				test = true;
				break;
			}
			if (!test)
				continue;
			// 与其他玩家存在赔付关系
			capital = static_cast<double>(avatar1->getCashPledge());
			node = new DebtNode(i, capital);
			debtNet.insert(std::make_pair(i, node));
		}
		std::unordered_map<int, DebtNode*>::const_iterator it1 = debtNet.begin();
		std::unordered_map<int, DebtNode*>::const_iterator it2;
		while (it1 != debtNet.end()) {
			node = it1->second;
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(it1->first).get());
			avatar1->getLoseScores(loseScores);
			for (int j = 0; j < 4; j++) {
				if (((it1->first) == j) || (loseScores[j] == 0))
					continue;
				it2 = debtNet.find(j);
				if (it2 != debtNet.end())
					node->tally((it2->second), diZhu * loseScores[j]);
			}
			++it1;
		}
		std::string logDebt;
		DebtLiquidation dl;
		dl.printDebtNet(debtNet, logDebt);
		if (!dl(debtNet)) {
			dl.releaseDebtNet(debtNet);
			ErrorS << "清算结果不正确，原始债务网：" << logDebt;
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
			ErrorS << "清算之后存在负数结果，原始债务网：" << logDebt;
			return;
		}
		double winGold = 0.0;
		it1 = debtNet.begin();
		while (it1 != debtNet.end()) {
			avatar1 = dynamic_cast<StandardMahjongAvatar*>(getAvatar(it1->first).get());
			node = it1->second;
			// 清算之后的剩余金币数量
			capital = node->getCapital();
			winGold = capital - static_cast<double>(avatar1->getCashPledge());
			avatar1->setWinGold(winGold);
			++it1;
		}
		dl.releaseDebtNet(debtNet);
	}

	void StandardMahjongRoom::doJieSuan() {
		_roundState = StageState::NotStarted;

		MsgMahjongSettlement msg;
		getSettlementData(&(msg.data));

		// 纳税额(抽水)
		double tax = 0.0;
		double delta = 0.0;
		// 纳税(抽水)比例，固定为1%
		const double rate = 0.01;
		int64_t tmp = 0LL;
		int64_t diamond = 0;
		int64_t cashPledge = 0LL;
		int64_t goldNeed = getCashPledge();
		bool test = true;
		GameAvatar::Ptr ptr;
		StandardMahjongAvatar* avatar = NULL;
		std::shared_ptr<GetCapitalTask> task;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			ptr = getAvatar(i);
			avatar = dynamic_cast<StandardMahjongAvatar*>(ptr.get());
			if (avatar == NULL)
				continue;
			delta = avatar->getWinGold();
			if (_mode != 0 && delta > 0.0f) {
				tmp = static_cast<int64_t>(delta);
				// 每局获利抽水1%
				tax = delta * rate;
				delta = delta - tax;
				// 四舍五入
				delta = floor(delta + 0.5);
				avatar->setWinGold(delta);
				// 将税收(抽水)的50%奖励给玩家的代理玩家
				rewardAgency(avatar->getPlayerId(), tax * 0.5, tmp);
			}
			msg.winGolds[i] = static_cast<int>(delta);
			cashPledge = avatar->getCashPledge();
			cashPledge += msg.winGolds[i];
			test = true;
			if (msg.winGolds[i] != 0) {
				if (cashPledge < goldNeed) {
					// 押金不足，尝试从玩家金币中补充扣除
					test = deductCashPledge(ptr);
				}
				else {
					// 将当前押金数额保存到数据库
					updateCashPledge(avatar->getPlayerId(), cashPledge);
				}
			}
			task = std::make_shared<GetCapitalTask>(avatar->getPlayerId());
			MysqlPool::getSingleton().syncQuery(task);
			if (task->getSucceed() && task->getRows() > 0)
				msg.golds[i] = task->getGold() + avatar->getCashPledge();
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
				if (getDiamondNeed() > diamond)
					_kicks[i] = true;	// 玩家剩余钻石已经不足下局扣除，踢出
			}
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar == NULL)
				continue;
			msg.kick = _kicks[i];
			msg.send(avatar->getSession());
		}
	}

	void StandardMahjongRoom::afterHu() {
		saveRoundRecord();

		GameAvatar::Ptr avatar;
		for (int i = 0; i < 4; i++) {
			avatar = getAvatar(i);
			if (!avatar)
				continue;
			if (_kicks[i])
				kickAvatar(avatar);	// 将玩家踢出房间
			else
				avatar->setReady(false);
		}
	}

	void StandardMahjongRoom::onDisbandRequest(const NetMessage::Ptr& netMsg) {
		MsgDisbandRequest* inst = dynamic_cast<MsgDisbandRequest*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if (_disbandState == StageState::Underway)
			return;
		StandardMahjongAvatar* avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar == NULL)
			return;
		_disbander = avatar->getSeat();
		for (int i = 0; i < 4; i++) {
			if (i == _disbander)
				_disbandChoices[i] = 1;
			else
				_disbandChoices[i] = 0;
		}
		_disbandState = StageState::Underway;
		_disbandTick = BaseUtils::getCurrentMillisecond();
		notifyDisbandVote(std::string(""));
	}

	void StandardMahjongRoom::notifyDisbandVote(const std::string& playerId) {
		MsgMahjongDisbandVote msg;
		msg.disbander = _disbander;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		msg.elapsed = static_cast<int>((nowTick - _disbandTick) / 1000LL);

		for (int i = 0; i < 4; i++)
			msg.choices[i] = _disbandChoices[i];
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void StandardMahjongRoom::onDisbandChoose(const NetMessage::Ptr& netMsg) {
		MsgDisbandChoose* inst = dynamic_cast<MsgDisbandChoose*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		if ((inst->choice != 1) && (inst->choice != 2))
			return;
		StandardMahjongAvatar* avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(inst->getPlayerId()).get());
		if (avatar != NULL)
			doDisbandChoose(avatar->getSeat(), inst->choice);
	}

	void StandardMahjongRoom::doDisbandChoose(int seat, int choice) {
		if (seat < 0 || seat >= getMaxPlayerNums())
			return;
		int half = (getMaxPlayerNums() >> 1);
		int nums1 = 0;
		int nums2 = 0;
		_disbandChoices[seat] = choice;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (_disbandChoices[i] == 1)
				nums1++;
			else if (_disbandChoices[i] == 2)
				nums2++;
		}
		MsgDisbandChoice msg;
		msg.seat = seat;
		msg.choice = choice;
		sendMessageToAll(msg);

		if (nums1 > half)
			disbandRoom();		// 超过一半的玩家同意解散，解散
		else if (nums2 >= half)
			disbandObsolete();	// 有一半及以上的玩家不同意解散，解散失败
	}

	void StandardMahjongRoom::disbandRoom() {
		// 房间解散
		_disbandState = StageState::Finished;

		MsgDisband msg;
		sendMessageToAll(msg);

		_roundState = StageState::NotStarted;

		kickAllAvatars();

		// 在解散房间后游戏结束
		gameOver();
	}

	void StandardMahjongRoom::disbandObsolete() {
		// 取消解散
		_disbandState = StageState::NotStarted;

		MsgDisbandObsolete msg;
		sendMessageToAll(msg);
	}

	void StandardMahjongRoom::saveRoundRecord() {
		std::shared_ptr<StandardMahjongRecordTask> task = std::make_shared<StandardMahjongRecordTask>();
		task->_venueId = getId();
		task->_roundNo = _roundNo;
		task->_banker = _backupBanker;
		StandardMahjongPlaybackData data;
		getSettlementData(&(data.settlement));
		getPlaybackData(data);
		StandardMahjongAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<StandardMahjongAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			task->_playerIds[i] = avatar->getPlayerId();
			task->_scores[i] = avatar->getScore();
			task->_winGolds[i] = static_cast<int>(avatar->getWinGold());
			data.winGolds[i] = task->_winGolds[i];
		}
		msgpack::sbuffer sbuf;
		msgpack::pack(sbuf, data);
		uLongf srcLen = static_cast<uLongf>(sbuf.size());
		std::string base64;
		if (srcLen > 0) {
			uLongf dstLen = srcLen + 100;
			unsigned char* dstBuf = new unsigned char[dstLen];
			int ret = compress(dstBuf, &dstLen, reinterpret_cast<const unsigned char*>(sbuf.data()), srcLen);
			if (ret != Z_OK)
				LOG_ERROR("牌局回放数据压缩失败");
			else {
				std::stringstream ss;
#if defined(DEBUG) || defined(_DEBUG)
				ss << "游戏(Id: " << getId() << ")牌局回放数据压缩完成，原始大小: " << srcLen << "，压缩后大小: " << dstLen;
				LOG_DEBUG(ss.str());
#endif
				if (!BaseUtils::encodeBase64(base64, reinterpret_cast<const char*>(dstBuf), static_cast<int>(dstLen))) {
					ss.str("");
					ss << "游戏(Id: " << getId() << ")牌局回放数据打包Base64失败";
					LOG_ERROR(ss.str());
				}
			}
			delete[] dstBuf;
		}
		task->_playback = base64;
		// 异步保存到数据库
		MysqlPool::getSingleton().asyncQuery(task);
	}
}