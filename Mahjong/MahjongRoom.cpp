// MahjongTable.cpp 

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "MahjongRoom.h"
#include "MahjongRule.h"
#include "MahjongMessages.h"

#include <assert.h>
#include <sstream>

namespace NiuMa
{
	MahjongRoom::MahjongRoom(const MahjongRule::Ptr& rule, const std::string& venueId, int gameType, int maxPlayerNums)
		: GameRoom(venueId, gameType, maxPlayerNums)
		, _rule(rule)
		, _dealer(rule->hasFlower())
		, _acOpIdAlloc(0)
		, _banker(0)
		, _actor(0)
		, _gangHu(-1)
		, _tilesLeft(0)
		, _playedTileId(MahjongTile::INVALID_ID)
		, _huTileId(MahjongTile::INVALID_ID)
		, _waitingTick(0LL)
		, _state(StateMachine::Null)
		, _chi(false)
		, _dianPao(true)
		, _mutiDianPao(true)
		, _allDianPao(true)
		, _waitingQiangGang(false)
		, _hu(false)
		, _anGangVisible(false)
		, _gangShangPaoVetoed(false)
		, _delayJiaGang(true)
	{}

	MahjongRoom::~MahjongRoom() {}

	bool MahjongRoom::onMessage(const NetMessage::Ptr& netMsg) {
		if (GameRoom::onMessage(netMsg))
			return true;
		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (msgType == MsgDoActionOption::TYPE)
			onActionOption(netMsg);
		else if (msgType == MsgPassActionOption::TYPE)
			onPassActionOption(netMsg);
		else if (msgType == MsgNextTile::TYPE)
			onNextTile(netMsg);
		else
			ret = false;
		return ret;
	}

	void MahjongRoom::clean() {
		MahjongAvatar* avatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (avatar != nullptr)
				avatar->clear();
		}
		_actions.clear();
		_actors.clear();
		_acOpIdAlloc.recycleAll();
		for (unsigned int i = 0; i < 4; i++) {
			_acOps1[i].clear();
			_acOps2[i].clear();
		}
		_gangHu = -1;
		_playedTileId = MahjongTile::INVALID_ID;
		_huTileId = MahjongTile::INVALID_ID;
		_state = StateMachine::Null;
		_waitingQiangGang = false;
		_hu = false;

		GameRoom::clean();
	}

	void MahjongRoom::updateCurrentActor()
	{
		const MahjongActor& ma = _actors.back();
		_actor = ma.getPlayer();
		_actor++;
		if (_actor >= getMaxPlayerNums())
			_actor -= getMaxPlayerNums();

		MahjongActor ma1(_actor, static_cast<int>(_actions.size()));
		_actors.push_back(ma1);

		notifyActorUpdated(BaseUtils::EMPTY_STRING);
	}

	void MahjongRoom::updateCurrentActor(int player)
	{
		_actor = player;
		MahjongActor ma(static_cast<int>(_actor), static_cast<int>(_actions.size()));
		_actors.push_back(ma);

		notifyActorUpdated(BaseUtils::EMPTY_STRING);
	}

	void MahjongRoom::changeState(StateMachine eNewState)
	{
		StateMachine eOldState = _state;
		_state = eNewState;

		notifyStateChanged(eOldState);
	}

	void MahjongRoom::dealTiles()
	{
		MahjongAvatar* pAvatar = nullptr;
		MahjongTile mt;
		bool bTest = true;
#if 0
		bTest = false;
		// 初始化固定手牌，测试麻将相关算法的正确性
		MahjongTile::Tile tile;
		const std::string tiles1[13] = { "一筒", "一筒", "一筒", "五条", "五条", "五条", "八条", "八条", "八条", "三万", "四万", "东", "东" };
		const std::string tiles2[13] = { "二筒", "二筒", "二筒", "三条", "三条", "三条", "二万", "二万", "二万", "白", "白", "白", "南" };
		const std::string tiles3[13] = { "九筒", "九筒", "九筒", "一条", "一条", "一条", "六万", "六万", "六万", "南", "南", "南", "西" };
		const std::string tiles4[13] = { "六筒", "六筒", "六筒", "六筒", "四条", "四条", "四条", "四条", "五万", "五万", "五万", "五万", "北"};
		std::string tileName;
		std::map<int, int> initTiles;
		std::map<int, int>::iterator it;
		for (int j = 0; j < getMaxPlayerNums(); j++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(j).get());
			MahjongTileArray& lstTiles = pAvatar->getTiles();
			lstTiles.clear();
			for (int i = 0; i < 13; i++) {
				if (j == 0)
					tileName = tiles1[i];
				else if (j == 1)
					tileName = tiles2[i];
				else if (j == 2)
					tileName = tiles3[i];
				else
					tileName = tiles4[i];
				tile = MahjongTile::Tile::fromString(tileName);
				mt.setTile(tile);
				if (!MahjongDealer::getIdByTile(mt)) {
					bTest = true;
					break;
				}
				it = initTiles.find(mt.getId());
				if (it == initTiles.end())
					initTiles.insert(std::make_pair(mt.getId(), 1));
				else {
					mt.setId(mt.getId() + (it->second));
					(it->second) = (it->second) + 1;
					if ((it->second) > 4) {
						bTest = true;
						break;
					}
				}
				lstTiles.push_back(mt);
			}
			if (bTest)
				break;
			pAvatar->sortTiles();
			pAvatar->backupDealedTiles();
			_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
		}
		if (bTest) {
			std::stringstream ss;
			tile.toString(tileName);
			ss << "初始化手牌出错:" << tileName << "为非法牌或其的总数超过4张";
			LOG_ERROR(ss.str());
		}
		else
			_dealer.shuffle(initTiles);
#endif
		if (bTest) {
			_dealer.shuffle();
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
				if (pAvatar == nullptr)
					continue;
				MahjongTileArray& lstTiles = pAvatar->getTiles();
				lstTiles.clear();
				for (unsigned int j = 0; j < 13; j++) {
					_dealer.fetchTile(mt);
					lstTiles.push_back(mt);
				}
				pAvatar->sortTiles();
				pAvatar->backupDealedTiles();
				_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
			}
		}
		notifyDealTiles();
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;
			notifyTingTile(pAvatar);
		}
		// 每人发13张牌，发完之后庄家摸第一张牌
		updateCurrentActor(_banker);
		fetchTile();
	}

	bool MahjongRoom::fetchTile(bool bBack) {
		if (earlyTermination()) {
			noMoreTile();
			return false;
		}
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		if (pAvatar == nullptr)
			return false;

		MahjongTile mt;
		bool bTest = false;
		const std::string& str = pAvatar->getNextTile();
		if (!str.empty()) {
			bTest = _dealer.fetchTile(mt, str);
			pAvatar->setNextTile(std::string(""));
		}
		if (!bTest) {
			if (bBack)
				_dealer.fetchTile1(mt);
			else
				_dealer.fetchTile(mt);
		}
		// 进入取牌后状态
		changeState(StateMachine::Fetched);

		if (pAvatar->canHu(mt)) {
			int id = _acOpIdAlloc.askForId();
			_acOpPool[id].setType(MahjongAction::Type::ZiMo);
			_acOpPool[id].setId(id);
			_acOpPool[id].setPlayer(_actor);
			_acOpPool[id].setTileId1(mt.getId());
			_acOps1[0].push_back(id);
			pAvatar->addActionOption(id);
		}
		pAvatar->fetchTile(mt);

		MahjongAction ma(MahjongAction::Type::Fetch, (static_cast<int>(_actors.size()) - 1), mt.getId());
		_actions.push_back(ma);

		// 通知摸牌
		notifyFetchTile(pAvatar, bBack);
		// 摸牌之后通知出牌或杠
		afterFetchChiPeng(pAvatar, mt.getId());

		return true;
	}

	bool MahjongRoom::earlyTermination() const {
		if (_tilesLeft >= _dealer.getTileLeft())
			return true;

		return false;
	}

	void MahjongRoom::onActionOption(const NetMessage::Ptr& netMsg) {
		MsgDoActionOption* inst = dynamic_cast<MsgDoActionOption*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		doActionOption(inst->getPlayerId(), inst->actionId, inst->tileId);
	}

	void MahjongRoom::doActionOption(const std::string& playerId, int actionId, int tileId) {
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(playerId).get());
		if ((pAvatar == nullptr) || !(pAvatar->hasActionOption(actionId)))
			return;

		std::stringstream ss;
		MahjongAction::Type actionType = _acOpPool[actionId].getType();
		if (actionType == MahjongAction::Type::Play) {
			if (_state != StateMachine::Play) {
				ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")出牌，但牌桌当前不处于等待出牌状态!";
				LOG_ERROR(ss.str());
				return;
			}
			if (pAvatar->getSeat() != _actor) {
				ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")出牌，但该玩家并非当前活动玩家!";
				LOG_ERROR(ss.str());
				return;
			}
			executePlay(tileId);
			return;
		}
		if (_state != StateMachine::Action) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")选择了一个动作选项，但牌桌当前不处于等待动作选项状态!";
			LOG_ERROR(ss.str());
			return;
		}
		int index = -1;	
		if (actionType == MahjongAction::Type::DianPao || actionType == MahjongAction::Type::ZiMo)
			index = 0;
		else if (actionType == MahjongAction::Type::ZhiGang || actionType == MahjongAction::Type::JiaGang || actionType == MahjongAction::Type::AnGang)
			index = 1;
		else if (actionType == MahjongAction::Type::Peng)
			index = 2;
		else if (actionType == MahjongAction::Type::Chi)
			index = 3;
		else {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")的动作选项不正确!";
			LOG_ERROR(ss.str());
			return;
		}
		std::vector<int>& acOps1 = _acOps1[index];
		std::vector<int>::const_iterator it = acOps1.begin();
		while (it != acOps1.end()) {
			if (*it == actionId) {
				acOps1.erase(it);
				break;
			}
			++it;
		}
		_acOps2[index].push_back(actionId);
		pAvatar->removeActionOption(actionId);
		clearActionOptions(pAvatar);
		if (!executeActionOptions()) {
			bool bTest = false;
			for (unsigned int i = 0; i < 4; i++) {
				if (!_acOps1[i].empty()) {
					bTest = true;
					break;
				}
			}
			if (bTest)
				notifyActionOptionsWaiting(pAvatar);
			else
				LOG_ERROR("逻辑错误，动作选项列表中已经没有任何动作，游戏逻辑将无法继续进行，此类严重错误要严格测试！");
		}
	}

	void MahjongRoom::onPassActionOption(const NetMessage::Ptr& netMsg) {
		MsgPassActionOption* inst = dynamic_cast<MsgPassActionOption*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		passActionOption(inst->getPlayerId());
	}

	void MahjongRoom::passActionOption(const std::string& playerId) {
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(playerId).get());
		if ((pAvatar == nullptr) || !(pAvatar->hasActionOption()))
			return;
		MahjongTile mt;
		const std::vector<int>& lstAcOps = pAvatar->getAllActionOptions();
		std::vector<int>::const_iterator it = lstAcOps.begin();
		while (it != lstAcOps.end()) {
			const MahjongActionOption& mao = _acOpPool[*it];
			if (mao.getType() == MahjongAction::Type::DianPao) {
				mt.setId(mao.getTileId1());
				if (_dealer.getTileById(mt))
					pAvatar->passDianPao(mt);
			}
			else if (mao.getType() == MahjongAction::Type::Peng) {
				mt.setId(mao.getTileId1());
				if (_dealer.getTileById(mt))
					pAvatar->passPeng(mt);
			}
			++it;
		}
		clearActionOptions(pAvatar);
		bool bRet = executeActionOptions();
		if (bRet)
			return;
		if (_waitingQiangGang) {
			// 继续等待其他玩家抢杠
			if (!_acOps1[0].empty())
				return;

			// 其他玩家取消抢杠，当前活动玩家继续完成加杠连贯动作
			const MahjongAction& ma = _actions.back();
			if (ma.getType() == MahjongAction::Type::JiaGang) {
#if defined(DEBUG) || defined(_DEBUG)
				LOG_DEBUG("取消抢杠测试");
#endif
				_waitingQiangGang = false;
				// 杠后补牌
				fetchTile(true);
			}
			else
				LOG_ERROR("逻辑错误，当前正等待玩家抢杠，然而动作列表中最后一个动作却不是加杠！");
		} else {
			bool bTest = false;
			for (unsigned int i = 0; i < 4; i++) {
				if (!_acOps1[i].empty()) {
					bTest = true;
					break;
				}
			}
			if (!bTest) {
				if (pAvatar->getSeat() == _actor) {
					// 通知出牌
					int id = _acOpIdAlloc.askForId();
					if (id >= ACTION_OPTION_POOL_SIZE) {
						LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
						return;
					}
					_acOpPool[id].setType(MahjongAction::Type::Play);
					_acOpPool[id].setId(id);
					_acOpPool[id].setPlayer(_actor);
					pAvatar->addActionOption(id);
					notifyActionOptions(pAvatar);

					// 进入等待出牌状态
					changeState(StateMachine::Play);
				} else {
					// 没有任何动作选项，判断是否要再次摸牌，如有的地方的规则是打出花牌之后要补花，
					// 若不要则切换到下一位玩家
					if (!fetchAgainAfterPlay())
						updateCurrentActor();
					fetchTile();
				}
			} else {
				// 再次进入等待动作选项状态
				changeState(StateMachine::Action);
			}
		}
	}

	void MahjongRoom::autoActionOption(bool bOnlyAuto) {
		// 自动动作的原则：能胡则胡->能杠则杠->能碰则碰->能吃则吃->都不能则出牌
		if (_state == StateMachine::Action) {
			// 当前状态为等待动作选项状态
			bool bAction = false;
			int acOp = 0;
			std::stringstream ss;
			MahjongAvatar* pAvatar = nullptr;
			while (true) {
				bAction = false;
				for (unsigned int i = 0; i < 4; i++) {
					if (_acOps1[i].empty())
						continue;
					for (unsigned int j = 0; j < _acOps1[i].size(); j++) {
						acOp = _acOps1[i].at(j);
						const MahjongActionOption& ma = _acOpPool[acOp];
						pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(ma.getPlayer()).get());
						if (pAvatar == nullptr) {
							ss << "逻辑错误，牌桌(Id: " << getId() << ")等待动作玩家(位置: " << ma.getPlayer() << ")为空!";
							LOG_ERROR(ss.str());
							return;
						}
						if (!bOnlyAuto || pAvatar->isAuthorize()) {
							bAction = true;
							break;
						}
					}
				}
				if (!bAction)
					break;
				doActionOption(pAvatar->getPlayerId(), acOp, 0);
				pAvatar->afterAutoAction();
			}
		}
		else if (_state == StateMachine::Play) {
			// 当前为等待出牌状态
			MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
			if (pAvatar == nullptr)
				return;
			const std::vector<int>& lstAcOps = pAvatar->getAllActionOptions();
			if (lstAcOps.empty())
				return;
			const MahjongActionOption& ma = _acOpPool[lstAcOps[0]];
			if (ma.getType() != MahjongAction::Type::Play)
				return;
			doActionOption(pAvatar->getPlayerId(), lstAcOps[0], pAvatar->autoPlayTile());
			return;
		}
	}

	void MahjongRoom::onNextTile(const NetMessage::Ptr& netMsg) {
		MsgNextTile* inst = dynamic_cast<MsgNextTile*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		doNextTile(inst->getPlayerId(), inst->tileName);
	}

	void MahjongRoom::doNextTile(const std::string& playerId, const std::string& str) {
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(playerId).get());
		if (pAvatar != nullptr)
			pAvatar->setNextTile(str);
	}

	void MahjongRoom::clearActionOptions() {
		for (unsigned int i = 0; i < 4; i++) {
			_acOps1[i].clear();
			_acOps2[i].clear();
		}
		MahjongAvatar* pAvatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;
			pAvatar->clearActionOptions();
		}
		_acOpIdAlloc.recycleAll();
	}

	void MahjongRoom::clearActionOptions(MahjongAvatar* pAvatar) {
		if (pAvatar == nullptr)
			return;

		bool bFound = false;
		const std::vector<int>& lstAcOps = pAvatar->getAllActionOptions();
		std::vector<int>::const_iterator it1 = lstAcOps.begin();
		std::vector<int>::const_iterator it2;
		while (it1 != lstAcOps.end()) {
			bFound = false;
			for (unsigned int i = 0; i < 4; i++) {
				it2 = _acOps1[i].begin();
				while (it2 != _acOps1[i].end()) {
					if (*it2 == *it1) {
						bFound = true;
						_acOps1[i].erase(it2);
						break;
					}
					++it2;
				}
				if (bFound)
					break;
			}
			_acOpIdAlloc.recycleId(*it1);
			++it1;
		}
		pAvatar->clearActionOptions();
	}

	bool MahjongRoom::executeActionOptions() {
		if (executeHu())
			return true;
		if (executeGang())
			return true;
		if (executePeng())
			return true;
		if (executeChi())
			return true;

		return false;
	}

	bool MahjongRoom::canDianPao() const {
		return _dianPao;
	}

	bool MahjongRoom::executeHu() {
		if (_acOps2[0].empty())
			return false;

		MahjongAvatar* pAvatar = nullptr;
		MahjongTile mt;
		const MahjongActionOption& acOp = _acOpPool[_acOps2[0].at(0)];
		int player = 0;
		int tileId = acOp.getTileId1();
		mt.setId(tileId);
		_dealer.getTile(mt);
		std::string passed;
		std::vector<int>::const_iterator it;
		std::stringstream ss;
		if (acOp.getType() == MahjongAction::Type::DianPao) {
			// 点炮
			if (_mutiDianPao) {
				// 允许一炮多响
				if (_allDianPao) {
					// 只要有一人胡就全部胡
					for (int i = 0; i < getMaxPlayerNums(); i++) {
						if (i == _actor)
							continue;
						pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
						if ((pAvatar == nullptr) || !canDianPao() || !(pAvatar->canDianPao(mt, passed)) || !(pAvatar->detectHuStyle(false, mt)))
							continue;
						pAvatar->addHuWay(MahjongGenre::HuWay::DianPao);
					}
				} else {
					// 所有选择“胡”以及还未选择“胡”的玩家能胡，选择了“过”的玩家将不能胡
					it = _acOps1[0].begin();
					while (it != _acOps1[0].end()) {
						if (_acOpPool[*it].getType() == MahjongAction::Type::DianPao)
							_acOps2[0].push_back(*it);
						else {
							ss.str("");
							ss << "逻辑错误，牌桌(Id: " << getId() << ")胡动作选项并不统一为点炮!";
							LOG_ERROR(ss.str());
						}
						++it;
					}
					_acOps1[0].clear();
					it = _acOps2[0].begin();
					while (it != _acOps2[0].end()) {
						player = _acOpPool[*it].getPlayer();
						pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(player).get());
						if (pAvatar == nullptr) {
							ss.str("");
							ss << "逻辑错误，牌桌(Id: " << getId() << ")可胡玩家(位置: " << player << ")为空!";
							LOG_ERROR(ss.str());
							continue;
						}
						if (!(pAvatar->detectHuStyle(false, mt))) {
							ss.str("");
							ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")未能胡牌!";
							LOG_ERROR(ss.str());
							continue;
						}
						pAvatar->addHuWay(MahjongGenre::HuWay::DianPao);
						++it;
					}
				}
			} else {
				// 不允许一炮多响，最近的玩家有最大点炮优先权
				int diff = 0;
				int min1 = getMaxPlayerNums();
				int min2 = min1;
				int id1 = 0;
				int id2 = 0;
				it = _acOps1[0].begin();
				while (it != _acOps1[0].end()) {
					diff = _acOpPool[*it].getPlayer();
					diff += getMaxPlayerNums() - _actor;
					diff %= getMaxPlayerNums();
					if (min1 > diff) {
						min1 = diff;
						id1 = *it;
					}
					++it;
				}
				it = _acOps2[0].begin();
				while (it != _acOps2[0].end()) {
					diff = _acOpPool[*it].getPlayer();
					diff += getMaxPlayerNums() - _actor;
					diff %= getMaxPlayerNums();
					if (min2 > diff) {
						min2 = diff;
						id2 = *it;
					}
					++it;
				}
				// 最近的那家还没决定是否要胡
				if (min1 < min2)
					return false;
				// 只有最近的那家能胡，其他人都不能胡
				player = _acOpPool[id2].getPlayer();
				pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(player).get());
				if ((pAvatar == nullptr) || !(pAvatar->detectHuStyle(false, mt)))
					return false;

				pAvatar->addHuWay(MahjongGenre::HuWay::DianPao);
			}
		}
		else if (acOp.getType() == MahjongAction::Type::ZiMo) {
			if ((_acOps1[0].size() + _acOps2[0].size()) > 1) {
				ss.str("");
				ss << "逻辑错误，牌桌(Id: " << getId() << ")有玩家自摸，但是能胡的玩家数却超过一人!";
				LOG_ERROR(ss.str());
				return false;
			}
			player = acOp.getPlayer();
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(player).get());
			if ((pAvatar == nullptr) || !(pAvatar->detectHuStyle(true, mt)))
				return false;

			pAvatar->addHuWay(MahjongGenre::HuWay::ZiMo);
		} else {
			ss.str("");
			ss << "逻辑错误，牌桌(Id: " << getId() << ")胡动作既不是点炮也不是自摸!";
			LOG_ERROR(ss.str());
			return false;
		}
		doHu();
		return true;
	}

	bool MahjongRoom::executeGang() {
		// 等待其他玩家选择胡动作
		if (!_acOps1[0].empty())
			return false;
		if (_acOps2[1].empty())
			return false;

		bool bQiangGang = false;
		std::string passed;
		std::stringstream ss;
		const MahjongActionOption& acOp = _acOpPool[_acOps2[1].at(0)];
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(acOp.getPlayer()).get());
		MahjongTile mt;
		MahjongAction ma;
		mt.setId(acOp.getTileId1());
		_dealer.getTile(mt);
		ma.setTile(acOp.getTileId1());
		if (acOp.getType() == MahjongAction::Type::ZhiGang) {
			if (!pAvatar->doZhiGang(mt, static_cast<int>(_actions.size()), _actor)) {
				ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")直杠失败!";
				LOG_ERROR(ss.str());
				return false;
			}
			ma.setType(MahjongAction::Type::ZhiGang);
			ma.setSlot(static_cast<int>(_actors.size()));

			MahjongAvatar* pCurAva = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
			pCurAva->setPlayedTileAction(mt.getId(), MahjongAction::Type::ZhiGang, acOp.getPlayer());
		}
		else if (acOp.getType() == MahjongAction::Type::JiaGang) {
			if (canDianPao()) {
				MahjongAvatar* pAvaTmp = nullptr;
				for (int i = 0; i < getMaxPlayerNums(); i++) {
					if (i == _actor)
						continue;
					pAvaTmp = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
					if (pAvaTmp == nullptr)
						continue;
					passed.clear();
					if (pAvaTmp->canHu(mt) && pAvaTmp->canDianPao(mt, passed)) {
						bQiangGang = true;
						break;
					}
					else if (!passed.empty())
						notifyPassTip(pAvaTmp, 1, passed);
				}
			}
			if (!pAvatar->doJiaGang(mt, static_cast<int>(_actions.size()))) {
				ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")加杠失败!";
				LOG_ERROR(ss.str());
				return false;
			}
			ma.setType(MahjongAction::Type::JiaGang);
			ma.setSlot(static_cast<int>(_actors.size() - 1));
		}
		else if (acOp.getType() == MahjongAction::Type::AnGang) {
			if (!pAvatar->doAnGang(mt, static_cast<int>(_actions.size()))) {
				ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")暗杠失败!";
				LOG_ERROR(ss.str());
				return false;
			}
			ma.setType(MahjongAction::Type::AnGang);
			ma.setSlot(static_cast<int>(_actors.size() - 1));
		} else {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")杠牌出错!";
			LOG_ERROR(ss.str());
			return false;
		}
		// 更新听牌信息
		_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
		// 清空所有动作选项
		clearActionOptions();
		// 通知所有人动作选项结束
		notifyActionOptionsFinish();
		// 通知所有人玩家杠牌
		notifyGangTile(pAvatar);
		// 通知玩家听牌
		notifyTingTile(pAvatar);

		if (acOp.getType() == MahjongAction::Type::ZhiGang)
			updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		if (bQiangGang) {
			// 通知等待其他玩家动作(等待其他玩家抢杠)
			notifyActionOptionsWaiting(pAvatar);
			// 通知其他玩家抢杠
			int tmp = 0;
			for (int i = 0; i < getMaxPlayerNums(); i++) {
				if (i == _actor)
					continue;
				pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
				if (!(pAvatar->canHu(mt)))
					continue;

				_waitingQiangGang = true;
				tmp = _acOpIdAlloc.askForId();
				if (tmp >= ACTION_OPTION_POOL_SIZE) {
					LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
					return false;
				}
				_acOpPool[tmp].setType(MahjongAction::Type::DianPao);
				_acOpPool[tmp].setId(tmp);
				_acOpPool[tmp].setPlayer(i);
				_acOpPool[tmp].setTileId1(mt.getId());
				_acOps1[0].push_back(tmp);
				pAvatar->addActionOption(tmp);
				notifyActionOptions(pAvatar);
			}
			// 进入等待动作选项状态
			changeState(StateMachine::Action);
		} else {
			// 杠后立即补牌
			fetchTile(true);
		}
		return true;
	}

	bool MahjongRoom::executePeng() {
		// 等待其他玩家选择胡动作
		if (!_acOps1[0].empty())
			return false;
		// 等待其他玩家选择杠动作
		if (!_acOps1[1].empty())
			return false;
		if (_acOps2[2].empty())
			return false;

		std::stringstream ss;
		const MahjongActionOption& acOp = _acOpPool[_acOps2[2].at(0)];
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(acOp.getPlayer()).get());
		MahjongTile mt;
		MahjongAction ma;
		mt.setId(acOp.getTileId1());
		_dealer.getTile(mt);
		ma.setType(MahjongAction::Type::Peng);
		ma.setSlot(static_cast<int>(_actors.size()));
		ma.setTile(acOp.getTileId1());
		if (acOp.getType() != MahjongAction::Type::Peng) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")碰牌出错!";
			LOG_ERROR(ss.str());
			return false;
		}
		int player = _actor;
		if (!pAvatar->doPeng(mt, static_cast<int>(_actions.size()), player)) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")碰牌失败!";
			LOG_ERROR(ss.str());
			return false;
		}
		MahjongAvatar* pCurAva = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		pCurAva->setPlayedTileAction(mt.getId(), MahjongAction::Type::Peng, acOp.getPlayer());

		updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		// 清空所有动作选项
		clearActionOptions();
		// 通知所有人动作选项结束
		notifyActionOptionsFinish();
		// 通知所有人玩家碰牌
		notifyPengChiTile(pAvatar, true);
		// 碰牌之后通知出牌或者杠
		afterFetchChiPeng(pAvatar);

		return true;
	}

	bool MahjongRoom::executeChi() {
		// 等待其他玩家选择胡动作
		if (!_acOps1[0].empty())
			return false;
		// 等待其他玩家选择杠动作
		if (!_acOps1[1].empty())
			return false;
		// 等待其他玩家选择碰动作
		if (!_acOps1[2].empty())
			return false;
		if (_acOps2[3].empty())
			return false;

		std::stringstream ss;
		const MahjongActionOption& acOp = _acOpPool[_acOps2[3].at(0)];
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(acOp.getPlayer()).get());
		MahjongTile mt;
		MahjongAction ma;
		mt.setId(_playedTileId);
		_dealer.getTile(mt);
		ma.setType(MahjongAction::Type::Chi);
		ma.setSlot(static_cast<int>(_actors.size()));
		ma.setTile(acOp.getTileId1());
		if (acOp.getType() != MahjongAction::Type::Chi) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")吃牌出错!";
			LOG_ERROR(ss.str());
			return false;
		}
		int player = _actor;
		if (!pAvatar->doChi(mt, acOp.getTileId1(), acOp.getTileId2(), static_cast<int>(_actions.size()), player)) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")吃牌失败!";
			LOG_ERROR(ss.str());
			return false;
		}
		MahjongAvatar* pCurAva = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		pCurAva->setPlayedTileAction(mt.getId(), MahjongAction::Type::Chi, acOp.getPlayer());

		updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		// 清空所有动作选项
		clearActionOptions();
		// 通知所有人动作选项结束
		notifyActionOptionsFinish();
		// 通知所有人玩家吃牌
		notifyPengChiTile(pAvatar, false);
		// 吃牌之后通知出牌或者杠
		afterFetchChiPeng(pAvatar);

		return true;
	}

	void MahjongRoom::afterFetchChiPeng(MahjongAvatar* pAvatar, int fetchedId) {
		if (pAvatar == nullptr)
			return;

		std::vector<int> lstTileIds;
		std::vector<int>::const_iterator it;
		int id = 0;
		if (pAvatar->canJiaGang(lstTileIds)) {
			it = lstTileIds.begin();
			while (it != lstTileIds.end()) {
				if (_delayJiaGang || (*it == fetchedId)) {
					id = _acOpIdAlloc.askForId();
					if (id >= ACTION_OPTION_POOL_SIZE) {
						LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
						return;
					}
					_acOpPool[id].setType(MahjongAction::Type::JiaGang);
					_acOpPool[id].setId(id);
					_acOpPool[id].setPlayer(_actor);
					_acOpPool[id].setTileId1(*it);
					_acOps1[1].push_back(id);
					pAvatar->addActionOption(id);
				}
				++it;
			}
		}
		if (pAvatar->canAnGang(lstTileIds)) {
			it = lstTileIds.begin();
			while (it != lstTileIds.end()) {
				id = _acOpIdAlloc.askForId();
				if (id >= ACTION_OPTION_POOL_SIZE) {
					LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
					return;
				}
				_acOpPool[id].setType(MahjongAction::Type::AnGang);
				_acOpPool[id].setId(id);
				_acOpPool[id].setPlayer(_actor);
				_acOpPool[id].setTileId1(*it);
				_acOps1[1].push_back(id);
				pAvatar->addActionOption(id);
				++it;
			}
		}
		if (pAvatar->hasActionOption()) {
			// 进入等待动作选项状态
			changeState(StateMachine::Action);
		} else {
			// 通知玩家出牌
			id = _acOpIdAlloc.askForId();
			if (id >= ACTION_OPTION_POOL_SIZE) {
				LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
				return;
			}
			_acOpPool[id].setType(MahjongAction::Type::Play);
			_acOpPool[id].setId(id);
			_acOpPool[id].setPlayer(_actor);
			pAvatar->addActionOption(id);

			// 进入等待出牌状态
			changeState(StateMachine::Play);
		}
		notifyActionOptions(pAvatar);
	}

	bool MahjongRoom::executePlay(int tileId) {
		// 等待其他玩家选择胡动作
		if (!_acOps1[0].empty())
			return false;
		// 等待其他玩家选择杠动作
		if (!_acOps1[1].empty())
			return false;
		// 等待其他玩家选择碰动作
		if (!_acOps1[2].empty())
			return false;
		// 等待其他玩家选择吃动作
		if (!_acOps1[3].empty())
			return false;

		std::stringstream ss;
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		if (!pAvatar->playTile(tileId)) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")出牌失败!";
			LOG_ERROR(ss.str());
			return false;
		}
		MahjongTile mt;
		mt.setId(tileId);
		_dealer.getTile(mt);
		pAvatar->addPlayedTile(mt);
		_playedTileId = tileId;
		MahjongAction ma;
		ma.setType(MahjongAction::Type::Play);
		ma.setSlot(static_cast<int>(_actors.size()) - 1);
		ma.setTile(tileId);
		_actions.push_back(ma);
		
		// 更新听牌信息
		_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
		// 清空所有动作选项
		clearActionOptions();
		// 通知所有玩家当前活动玩家出牌
		notifyPlayTile(mt);
		// 通知玩家听牌
		notifyTingTile(pAvatar);

		bool bTest = false;
		int tmp = 0;
		std::string passed;
		std::vector<std::pair<int, int> > lstPairs;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			if (i == _actor)
				continue;
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;
			passed.clear();
			if (canDianPao() && pAvatar->canHu(mt) && pAvatar->canDianPao(mt, passed)) {
				tmp = _acOpIdAlloc.askForId();
				if (tmp >= ACTION_OPTION_POOL_SIZE) {
					LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
					return false;
				}
				_acOpPool[tmp].setType(MahjongAction::Type::DianPao);
				_acOpPool[tmp].setId(tmp);
				_acOpPool[tmp].setPlayer(i);
				_acOpPool[tmp].setTileId1(tileId);
				_acOps1[0].push_back(tmp);
				pAvatar->addActionOption(tmp);
			}
			else if (!passed.empty())
				notifyPassTip(pAvatar, 1, passed);
			if (pAvatar->canZhiGang(mt)) {
				tmp = _acOpIdAlloc.askForId();
				if (tmp >= ACTION_OPTION_POOL_SIZE) {
					LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
					return false;
				}
				_acOpPool[tmp].setType(MahjongAction::Type::ZhiGang);
				_acOpPool[tmp].setId(tmp);
				_acOpPool[tmp].setPlayer(i);
				_acOpPool[tmp].setTileId1(tileId);
				_acOps1[1].push_back(tmp);
				pAvatar->addActionOption(tmp);
			}
			passed.clear();
			if (pAvatar->canPeng(mt, passed)) {
				tmp = _acOpIdAlloc.askForId();
				if (tmp >= ACTION_OPTION_POOL_SIZE) {
					LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
					return false;
				}
				_acOpPool[tmp].setType(MahjongAction::Type::Peng);
				_acOpPool[tmp].setId(tmp);
				_acOpPool[tmp].setPlayer(i);
				_acOpPool[tmp].setTileId1(tileId);
				_acOps1[2].push_back(tmp);
				pAvatar->addActionOption(tmp);
			}
			else if (!passed.empty())
				notifyPassTip(pAvatar, 0, passed);
			if (_chi && (((_actor + 1) % getMaxPlayerNums()) == i) && pAvatar->canChi(mt, lstPairs)) {
				std::vector<std::pair<int, int> >::const_iterator it = lstPairs.begin();
				while (it != lstPairs.end()) {
					tmp = _acOpIdAlloc.askForId();
					if (tmp >= ACTION_OPTION_POOL_SIZE) {
						LOG_ERROR("逻辑错误，动作id大于动作选项池大小");
						return false;
					}
					_acOpPool[tmp].setType(MahjongAction::Type::Chi);
					_acOpPool[tmp].setId(tmp);
					_acOpPool[tmp].setPlayer(i);
					_acOpPool[tmp].setTileId1(it->first);
					_acOpPool[tmp].setTileId2(it->second);
					_acOps1[3].push_back(tmp);
					pAvatar->addActionOption(tmp);
					++it;
				}
			}
			if (pAvatar->hasActionOption()) {
				bTest = true;
				notifyActionOptions(pAvatar);
			}
		}
		if (!bTest) {
			// 没有任何动作选项，判断是否要再次摸牌，如有的地方的规则是打出花牌之后要补花，
			// 若不要则切换到下一位玩家
			if (!fetchAgainAfterPlay())
				updateCurrentActor();
			fetchTile();
		} else {
			// 进入等待动作选项状态
			changeState(StateMachine::Action);
		}
		return true;
	}

	bool MahjongRoom::fetchAgainAfterPlay() const {
		return false;
	}

	bool MahjongRoom::noChiPengGang() const {
		MahjongAction::Type eType = MahjongAction::Type::Invalid;
		MahjongActionList::const_iterator it = _actions.begin();
		while (it != _actions.end()) {
			eType = it->getType();
			if (eType == MahjongAction::Type::Chi ||
				eType == MahjongAction::Type::Peng ||
				eType == MahjongAction::Type::ZhiGang ||
				eType == MahjongAction::Type::JiaGang ||
				eType == MahjongAction::Type::AnGang)
				return false;

			++it;
		}
		return true;
	}

	void MahjongRoom::doHu() {
		// 清空所有动作选项
		clearActionOptions();
		// 通知动作选项完成
		notifyActionOptionsFinish();
		// 进入结束状态
		changeState(StateMachine::End);

		int nTemp = 0;
		int nHuNums = 0;
		int banker = 0;
		bool bTemp = false;
		bool bGangShangPao = false;
		_hu = true;
		std::stringstream ss;
		MahjongAction::Type eType = MahjongAction::Type::Invalid;
		MahjongAvatar* pAvatar = nullptr;
		MahjongAvatar* pAvaFangPao = nullptr;
		MahjongAvatar* pAvaTmp = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if ((pAvatar == nullptr) || !(pAvatar->isHu()))
				continue;

			nHuNums++;
			banker = i;
			pAvatar->addHuTimes();
			if (pAvatar->isMenQing())
				pAvatar->addHuWay(MahjongGenre::HuWay::MenQing);
			if (i == _actor) {
				pAvatar->addZiMo();
				// 自摸，胡的牌为手上拿的最后一张牌
				_huTileId = pAvatar->getFetchedTileId();
				// 自摸，手上只有两张牌，则为全求人
				if (pAvatar->getTileNums() == 2)
					pAvatar->addHuWay(MahjongGenre::HuWay::QuanQiuRen);
				if (i == _banker) {
					// 庄家摸起第一张牌胡牌，天胡
					if ((pAvatar->getPlayedTileNums() == 0) && noChiPengGang())
						pAvatar->addHuWay(MahjongGenre::HuWay::TianHu);
				}
				else if ((pAvatar->getPlayedTileNums() == 0)) {
					// 闲家摸起第一张牌胡牌，并且桌面上没有任何吃碰杠，人胡
					if (noChiPengGang())
						pAvatar->addHuWay(MahjongGenre::HuWay::RenHu);
				}
				if (_tilesLeft >= _dealer.getTileLeft())
					pAvatar->addHuWay(MahjongGenre::HuWay::HaiDiLaoYue);	// 牌池已经没有剩余的牌，海底捞月
				const MahjongActor& ma = _actors.back();
				for (int j = ma.getStart(); j < static_cast<int>(_actions.size()); j++) {
					eType = _actions[j].getType();
					if (eType == MahjongAction::Type::ZhiGang) {
						if (!bTemp) {
							bTemp = true;
							const MahjongActor& ma1 = _actors.at(_actors.size() - 2);
							_gangHu = ma1.getPlayer();
							pAvatar->addHuWay(MahjongGenre::HuWay::MingGang);
						}
						nTemp++;
					}
					else if (eType == MahjongAction::Type::JiaGang) {
						if (!bTemp) {
							bTemp = true;
							MahjongTile mt;
							mt.setId(_actions[j].getTile());
							_dealer.getTile(mt);
							for (int k = 0; k < getMaxPlayerNums(); k++) {
								if (k == i)
									continue;
								pAvaTmp = dynamic_cast<MahjongAvatar*>(getAvatar(k).get());
								if (pAvaTmp == nullptr) {
									ss << "逻辑错误，牌桌(Id: " << getId() << ")胡牌，玩家(位置: " << k << ")为空!";
									LOG_ERROR(ss.str());
									return;
								}
								if (pAvaTmp->hasPlayedTilePeng(mt.getTile())) {
									_gangHu = k;
									break;
								}
							}
							pAvatar->addHuWay(MahjongGenre::HuWay::MingGang);
						}
						nTemp++;
					}
					else if (eType == MahjongAction::Type::AnGang) {
						if (!bTemp)
							bTemp = true;
						nTemp++;
					}
				}
				if (nTemp == 1)
					pAvatar->addHuWay(MahjongGenre::HuWay::GangShangHua1);
				else if (nTemp == 2)
					pAvatar->addHuWay(MahjongGenre::HuWay::GangShangHua2);
				else if (nTemp == 3)
					pAvatar->addHuWay(MahjongGenre::HuWay::GangShangHua3);
				else if (nTemp == 4)
					pAvatar->addHuWay(MahjongGenre::HuWay::GangShangHua4);
			} else {
				// 点炮
				pAvatar->addJiePao();
				if (pAvaFangPao == nullptr) {
					pAvaFangPao = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
					if (pAvaFangPao == nullptr)
						continue;
					pAvaFangPao->addFangPao();
				}
				// 放炮的玩家手上仅剩一张牌，为全求炮
				if (pAvaFangPao->getTileNums() == 1)
					pAvatar->addHuWay(MahjongGenre::HuWay::QuanQiuPao);
				if ((pAvatar->getPlayedTileNums() == 0) && noChiPengGang()) {
					if (_actor == _banker)
						pAvatar->addHuWay(MahjongGenre::HuWay::DiHu);	// 庄家打出第一张牌放炮，地胡
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::RenHu);	// 闲家打出第一张牌放炮，人胡
				}
				if (_tilesLeft >= _dealer.getTileLeft())
					pAvatar->addHuWay(MahjongGenre::HuWay::HaiDiPao);	// 牌池已经没有剩余的牌，海底炮
				nTemp = 0;
				if (_waitingQiangGang) {
					// 抢杠胡，胡的牌为加杠的牌
					_huTileId = _actions.back().getTile();
					pAvaFangPao->doQiangGang(_huTileId, static_cast<int>(_actions.size()));
					_gangHu = _actor;
				} else {
					// 胡牌为玩家最后打出的一张牌
					_huTileId = _playedTileId;
				}
				const MahjongActor& ma = _actors.back();
				for (int j = ma.getStart(); j < static_cast<int>(_actions.size()); j++) {
					eType = _actions[j].getType();
					if ((eType == MahjongAction::Type::ZhiGang) ||
						(eType == MahjongAction::Type::JiaGang) ||
						(eType == MahjongAction::Type::AnGang))
						nTemp++;
				}
				if (nTemp == 1) {
					if (_waitingQiangGang)
						pAvatar->addHuWay(MahjongGenre::HuWay::QiangGangHu1);
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::GangShangPao1);
				}
				else if (nTemp == 2) {
					if (_waitingQiangGang)
						pAvatar->addHuWay(MahjongGenre::HuWay::QiangGangHu2);
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::GangShangPao2);
				}
				else if (nTemp == 3) {
					if (_waitingQiangGang)
						pAvatar->addHuWay(MahjongGenre::HuWay::QiangGangHu3);
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::GangShangPao3);
				}
				else if (nTemp == 4) {
					if (_waitingQiangGang)
						pAvatar->addHuWay(MahjongGenre::HuWay::QiangGangHu4);
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::GangShangPao4);
				}
				if (!_waitingQiangGang && (nTemp != 0))
					bGangShangPao = true;
			}
		}
		if (_gangShangPaoVetoed && bGangShangPao && (pAvaFangPao != nullptr))
			pAvaFangPao->vetoLastGangs(1);
		
		// 通知胡牌
		notifyHuTile();
		// 显示所有玩家的手牌
		notifyShowTiles();
		// 算分
		calcHuScore();
		// 结算
		doJieSuan();
		// 切换下一局的庄家
		bankerNextRound(nHuNums, banker);
		// 备份最后的动作者
		int lastActor = _actor;
		// 保存胡动作
		MahjongAction ma;
		ma.setTile(_actions.back().getTile());
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			bTemp = true;
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar->isZiMo())
				ma.setType(MahjongAction::Type::ZiMo);
			else if (pAvatar->isDianPao())
				ma.setType(MahjongAction::Type::DianPao);
			else
				bTemp = false;
			if (!bTemp)
				continue;

			ma.setSlot(static_cast<int>(_actors.size()));
			updateCurrentActor(i);
			_actions.push_back(ma);
		}
		// 还原最后的动作者
		_actor = lastActor;
		// 后续处理
		afterHu();
	}

	void MahjongRoom::noMoreTile() {
		// 进入结束状态
		changeState(StateMachine::End);
		// 算分
		calcHuScore();
		// 通知胡牌
		doJieSuan();
		// 后续处理
		afterHu();
		// 流局最后一个摸牌的玩家为庄家
		MahjongActionList::const_reverse_iterator it = _actions.rbegin();
		while (it != _actions.rend()) {
			if (it->getType() == MahjongAction::Type::Fetch) {
				_banker = _actors[it->getSlot()].getPlayer();
				break;
			}
			++it;
		}
	}

	void MahjongRoom::notifyDealTiles() {
		MsgMahjongTiles msg;
		MahjongAvatar* pAvatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if ((pAvatar == nullptr) || pAvatar->isRobot())
				continue;

			msg.tiles.clear();
			const MahjongTileArray& handTiles = pAvatar->getTiles();
			MahjongTileArray::const_iterator it = handTiles.begin();
			while (it != handTiles.end()) {
				msg.tiles.push_back(*it);
				++it;
			}
			msg.send(pAvatar->getSession());
		}
	}

	void MahjongRoom::notifyActorUpdated(const std::string& playerId) {
		MsgActorUpdated msg;
		msg.actor = _actor;
		if (playerId.empty())
			sendMessageToAll(msg);
		else
			sendMessage(msg, playerId);
	}

	void MahjongRoom::notifyStateChanged(StateMachine oldState) {
		if ((_state == StateMachine::Action) || (_state == StateMachine::Play))
			_waitingTick = BaseUtils::getCurrentMillisecond();

		notifyWaitingAction(BaseUtils::EMPTY_STRING);
	}

	void MahjongRoom::notifyWaitingAction(const std::string& playerId) {
		MahjongAvatar* avatar = nullptr;
		if (!playerId.empty()) {
			avatar = dynamic_cast<MahjongAvatar*>(getAvatar(playerId).get());
			if (avatar == nullptr)
				return;
		}
		MsgWaitAction msg;
		time_t nowTick = BaseUtils::getCurrentMillisecond();
		msg.waitting = ((_state == StateMachine::Action) || (_state == StateMachine::Play));
		msg.second = static_cast<int>(nowTick - _waitingTick) / 1000;
		if (!playerId.empty()) {
			msg.beingHeld = avatar->hasActionOption();
			msg.send(avatar->getSession());
			return;
		}
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			avatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (avatar == nullptr)
				continue;
			msg.beingHeld = avatar->hasActionOption() ? 1 : 0;
			msg.send(avatar->getSession());
		}
	}

	void MahjongRoom::notifyFetchTile(MahjongAvatar* avatar, bool bBack) {
		MsgFetchTile msg;
		msg.player = _actor;
		msg.back = bBack;
		msg.nums = static_cast<int>(_dealer.getTileLeft());
		msg.tile.setId(avatar->getFetchedTileId());
		_dealer.getTile(msg.tile);
		msg.send(avatar->getSession());

		msg.tile = MahjongTile();
		sendMessageToAll(msg, avatar->getPlayerId());
	}

	void MahjongRoom::notifyActionOptions(MahjongAvatar* pAvatar) {
		if (pAvatar == nullptr)
			return;
		const std::vector<int>& actionOptions = pAvatar->getAllActionOptions();
		if (actionOptions.empty())
			return;
		MsgActionOption msg;
		std::vector<int>::const_iterator it = actionOptions.begin();
		while (it != actionOptions.end()) {
			msg.actionOptions.push_back(_acOpPool[*it]);
			++it;
		}
		msg.send(pAvatar->getSession());
	}

	void MahjongRoom::notifyActionOptionsWaiting(MahjongAvatar* pAvatar) {
		// 在玩家选择动作选项之后，通知等待其他玩家选择动作选项
	}

	void MahjongRoom::notifyActionOptionsFinish() {
		MsgActionOptionFinish msg;
		sendMessageToAll(msg);
	}

	void MahjongRoom::notifyPlayTile(const MahjongTile& mt) {
		MsgPlayTile msg;
		msg.actor = _actor;
		msg.tile = mt;

		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		sendMessageToAll(msg, pAvatar->getPlayerId());

		msg.handTiles = pAvatar->getTiles();
		msg.send(pAvatar->getSession());
	}

	void MahjongRoom::notifyGangTile(MahjongAvatar* pAvatar) {
		std::stringstream ss;
		const MahjongChapterArray& lstChaps = pAvatar->getChapters();
		if (lstChaps.empty()) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")没有任何章，此时不应该通知杠发生!";
			LOG_ERROR(ss.str());
			return;
		}
		MsgGangTile msg;
		const MahjongChapter& mc = lstChaps.back();
		msg.chapter = static_cast<int>(mc.getType());
		if (!mc.isGang()) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")最新的章不是杠牌，此时不应该通知杠发生!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongTileArray& lstTiles = mc.getAllTiles();
		if (lstTiles.size() < 4) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")杠章中不够四张牌!";
			LOG_ERROR(ss.str());
			return;
		}
		msg.actor = pAvatar->getSeat();
		msg.player = mc.getTargetPlayer();
		msg.tileNums = static_cast<int>(pAvatar->getTileNums());
		msg.chapters = pAvatar->getChapters();
		msg.handTiles = pAvatar->getTiles();
		msg.send(pAvatar->getSession());

		msg.handTiles.clear();
		if (!_anGangVisible) {
			MahjongChapterArray::iterator it = msg.chapters.begin();
			while (it != msg.chapters.end()) {
				if (it->getType() == MahjongChapter::Type::AnGang)
					it->hideAnGangTiles();
				++it;
			}
		}
		sendMessageToAll(msg, pAvatar->getPlayerId());
	}

	void MahjongRoom::notifyPengChiTile(MahjongAvatar* pAvatar, bool bPeng) {
		std::stringstream ss;
		const MahjongChapterArray& lstChaps = pAvatar->getChapters();
		if (lstChaps.empty()) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")没有任何章，此时不应该通知碰(吃)发生!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongChapter& mc = lstChaps.back();
		if ((bPeng && (mc.getType() != MahjongChapter::Type::Peng)) ||
			(!bPeng && (mc.getType() != MahjongChapter::Type::Chi))) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")最新的章不是碰(吃)牌，此时不应该通知碰(吃)发生!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongTileArray& lstTiles = mc.getAllTiles();
		if (lstTiles.size() < 3) {
			ss << "逻辑错误，牌桌(Id: " << getId() << ")玩家(Id: " << pAvatar->getPlayerId() << ")碰(吃)章中不够三张牌!";
			LOG_ERROR(ss.str());
			return;
		}
		MsgPengChiTile msg;
		msg.actor = pAvatar->getSeat();
		msg.player = mc.getTargetPlayer();
		msg.pengOrChi = (mc.getType() == MahjongChapter::Type::Peng);
		msg.tiles = lstTiles;
		sendMessageToAll(msg, pAvatar->getPlayerId());

		msg.handTiles = pAvatar->getTiles();
		msg.send(pAvatar->getSession());
	}

	void MahjongRoom::notifyTingTile(MahjongAvatar* pAvatar) {
		if (pAvatar == nullptr)
			return;
		MsgTingTile msg;
		const MahjongGenre::TingPaiArray& tingTiles = pAvatar->getTingTiles();
		MahjongGenre::TingPaiArray::const_iterator it = tingTiles.begin();
		while (it != tingTiles.end()) {
			msg.tiles.push_back(it->tile);
			++it;
		}
		msg.send(pAvatar->getSession());
	}

	void MahjongRoom::notifyHuTile() {
		int nums = 0;
		MahjongAvatar* pAvatar = nullptr;
		MsgHuTile msg;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if ((pAvatar != nullptr) && (pAvatar->isHu())) {
				msg.players[nums] = i;
				if (pAvatar->isZiMo())
					msg.ziMo = true;
				nums++;
			}
			if (nums > 2)
				break;
		}
		if (nums > 3) {
			std::stringstream ss;
			ss << "逻辑错误，牌桌(Id: " << getId() << ")超过3位玩家同时胡牌!";
			LOG_ERROR(ss.str());
			return;
		}
		msg.actor = _actor;
		msg.tile.setId(_huTileId);
		_dealer.getTileById(msg.tile);
		sendMessageToAll(msg);
	}

	void MahjongRoom::notifyShowTiles() {
		MsgShowTiles msg;
		MahjongTile mt;
		MahjongAvatar* pAvatar = nullptr;
		unsigned int nums = 0;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;
			nums = pAvatar->getTileNums();
			if ((nums % 3) == 2) {
				pAvatar->getTilesNoFetched(msg.handTiles[i]);
				pAvatar->getFetchedTile(mt);
				msg.handTiles[i].push_back(mt);
			}
			else
				msg.handTiles[i] = pAvatar->getTiles();
		}
		sendMessageToAll(msg);
	}

	void MahjongRoom::notifyPassTip(MahjongAvatar* avatar, int action, const std::string& tile) {
		MsgPassTip msg;
		msg.action = action;
		msg.tile = tile;
		msg.send(avatar->getSession());
	}

	bool MahjongRoom::getFirstWaitingActionOption(MahjongActionOption& ao) const {
		bool bFound = false;
		std::vector<unsigned int>::const_iterator it;
		for (unsigned int i = 0; i < 4; i++){
			if (_acOps1[i].empty())
				continue;

			bFound = true;
			ao = _acOpPool[_acOps1[i].front()];
			break;
		}
		return bFound;
	}

	void MahjongRoom::getSettlementData(MahjongSettlement* dt) const {
		if (dt == nullptr)
			return;

		dt->actor = _actor;
		unsigned int nums = 0;
		MahjongAvatar* pAvatar = nullptr;
		MahjongTileArray::const_iterator it;
		MahjongChapterArray::const_iterator it1;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;

			dt->huWays[i] = pAvatar->getHuWay();
			dt->huStyles[i] = pAvatar->getHuStyle();
			dt->huStyleExs[i] = pAvatar->getHuStyleEx();
			dt->scores[i] = pAvatar->getScore();
			nums = pAvatar->getTileNums();
			if ((nums % 3) == 2)
				pAvatar->getTilesNoFetched(dt->handTiles[i]);
			else
				dt->handTiles[i] = pAvatar->getTiles();
			dt->chapters[i] = pAvatar->getChapters();
		}
		dt->hu = _hu;
		dt->huTile.setId(_huTileId);
		_dealer.getTileById(dt->huTile);
	}

	void MahjongRoom::getPlaybackData(MahjongPlaybackData& dt) const {
		MahjongAvatar* pAvatar = nullptr;
		for (int i = 0; i < getMaxPlayerNums(); i++) {
			pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
			if (pAvatar == nullptr)
				continue;

			dt.dealedTiles[i] = pAvatar->getDealedTiles();
			dt.chapters[i] = pAvatar->getChapters();
		}
		dt.actions = _actions;
		dt.actors = _actors;
	}

	void MahjongRoom::bankerNextRound(int huNums, int huPlayer) {
		// 一炮多响，下局庄家为放炮者
		if (huNums > 1)
			_banker = _actor;
		else if (huNums == 1)
			_banker = huPlayer;
	}
}