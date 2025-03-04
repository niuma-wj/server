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
		// ��ʼ���̶����ƣ������齫����㷨����ȷ��
		MahjongTile::Tile tile;
		const std::string tiles1[13] = { "һͲ", "һͲ", "һͲ", "����", "����", "����", "����", "����", "����", "����", "����", "��", "��" };
		const std::string tiles2[13] = { "��Ͳ", "��Ͳ", "��Ͳ", "����", "����", "����", "����", "����", "����", "��", "��", "��", "��" };
		const std::string tiles3[13] = { "��Ͳ", "��Ͳ", "��Ͳ", "һ��", "һ��", "һ��", "����", "����", "����", "��", "��", "��", "��" };
		const std::string tiles4[13] = { "��Ͳ", "��Ͳ", "��Ͳ", "��Ͳ", "����", "����", "����", "����", "����", "����", "����", "����", "��"};
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
			ss << "��ʼ�����Ƴ���:" << tileName << "Ϊ�Ƿ��ƻ������������4��";
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
		// ÿ�˷�13���ƣ�����֮��ׯ������һ����
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
		// ����ȡ�ƺ�״̬
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

		// ֪ͨ����
		notifyFetchTile(pAvatar, bBack);
		// ����֮��֪ͨ���ƻ��
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
				ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���ƣ���������ǰ�����ڵȴ�����״̬!";
				LOG_ERROR(ss.str());
				return;
			}
			if (pAvatar->getSeat() != _actor) {
				ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���ƣ�������Ҳ��ǵ�ǰ����!";
				LOG_ERROR(ss.str());
				return;
			}
			executePlay(tileId);
			return;
		}
		if (_state != StateMachine::Action) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")ѡ����һ������ѡ���������ǰ�����ڵȴ�����ѡ��״̬!";
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
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")�Ķ���ѡ���ȷ!";
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
				LOG_ERROR("�߼����󣬶���ѡ���б����Ѿ�û���κζ�������Ϸ�߼����޷��������У��������ش���Ҫ�ϸ���ԣ�");
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
			// �����ȴ������������
			if (!_acOps1[0].empty())
				return;

			// �������ȡ�����ܣ���ǰ���Ҽ�����ɼӸ����ᶯ��
			const MahjongAction& ma = _actions.back();
			if (ma.getType() == MahjongAction::Type::JiaGang) {
#if defined(DEBUG) || defined(_DEBUG)
				LOG_DEBUG("ȡ�����ܲ���");
#endif
				_waitingQiangGang = false;
				// �ܺ���
				fetchTile(true);
			}
			else
				LOG_ERROR("�߼����󣬵�ǰ���ȴ�������ܣ�Ȼ�������б������һ������ȴ���ǼӸܣ�");
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
					// ֪ͨ����
					int id = _acOpIdAlloc.askForId();
					if (id >= ACTION_OPTION_POOL_SIZE) {
						LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
						return;
					}
					_acOpPool[id].setType(MahjongAction::Type::Play);
					_acOpPool[id].setId(id);
					_acOpPool[id].setPlayer(_actor);
					pAvatar->addActionOption(id);
					notifyActionOptions(pAvatar);

					// ����ȴ�����״̬
					changeState(StateMachine::Play);
				} else {
					// û���κζ���ѡ��ж��Ƿ�Ҫ�ٴ����ƣ����еĵط��Ĺ����Ǵ������֮��Ҫ������
					// ����Ҫ���л�����һλ���
					if (!fetchAgainAfterPlay())
						updateCurrentActor();
					fetchTile();
				}
			} else {
				// �ٴν���ȴ�����ѡ��״̬
				changeState(StateMachine::Action);
			}
		}
	}

	void MahjongRoom::autoActionOption(bool bOnlyAuto) {
		// �Զ�������ԭ���ܺ����->�ܸ����->��������->�ܳ����->�����������
		if (_state == StateMachine::Action) {
			// ��ǰ״̬Ϊ�ȴ�����ѡ��״̬
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
							ss << "�߼���������(Id: " << getId() << ")�ȴ��������(λ��: " << ma.getPlayer() << ")Ϊ��!";
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
			// ��ǰΪ�ȴ�����״̬
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
			// ����
			if (_mutiDianPao) {
				// ����һ�ڶ���
				if (_allDianPao) {
					// ֻҪ��һ�˺���ȫ����
					for (int i = 0; i < getMaxPlayerNums(); i++) {
						if (i == _actor)
							continue;
						pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(i).get());
						if ((pAvatar == nullptr) || !canDianPao() || !(pAvatar->canDianPao(mt, passed)) || !(pAvatar->detectHuStyle(false, mt)))
							continue;
						pAvatar->addHuWay(MahjongGenre::HuWay::DianPao);
					}
				} else {
					// ����ѡ�񡰺����Լ���δѡ�񡰺���������ܺ���ѡ���ˡ���������ҽ����ܺ�
					it = _acOps1[0].begin();
					while (it != _acOps1[0].end()) {
						if (_acOpPool[*it].getType() == MahjongAction::Type::DianPao)
							_acOps2[0].push_back(*it);
						else {
							ss.str("");
							ss << "�߼���������(Id: " << getId() << ")������ѡ���ͳһΪ����!";
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
							ss << "�߼���������(Id: " << getId() << ")�ɺ����(λ��: " << player << ")Ϊ��!";
							LOG_ERROR(ss.str());
							continue;
						}
						if (!(pAvatar->detectHuStyle(false, mt))) {
							ss.str("");
							ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")δ�ܺ���!";
							LOG_ERROR(ss.str());
							continue;
						}
						pAvatar->addHuWay(MahjongGenre::HuWay::DianPao);
						++it;
					}
				}
			} else {
				// ������һ�ڶ��죬��������������������Ȩ
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
				// ������Ǽһ�û�����Ƿ�Ҫ��
				if (min1 < min2)
					return false;
				// ֻ��������Ǽ��ܺ��������˶����ܺ�
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
				ss << "�߼���������(Id: " << getId() << ")����������������ܺ��������ȴ����һ��!";
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
			ss << "�߼���������(Id: " << getId() << ")�������Ȳ��ǵ���Ҳ��������!";
			LOG_ERROR(ss.str());
			return false;
		}
		doHu();
		return true;
	}

	bool MahjongRoom::executeGang() {
		// �ȴ��������ѡ�������
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
				ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")ֱ��ʧ��!";
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
				ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")�Ӹ�ʧ��!";
				LOG_ERROR(ss.str());
				return false;
			}
			ma.setType(MahjongAction::Type::JiaGang);
			ma.setSlot(static_cast<int>(_actors.size() - 1));
		}
		else if (acOp.getType() == MahjongAction::Type::AnGang) {
			if (!pAvatar->doAnGang(mt, static_cast<int>(_actions.size()))) {
				ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")����ʧ��!";
				LOG_ERROR(ss.str());
				return false;
			}
			ma.setType(MahjongAction::Type::AnGang);
			ma.setSlot(static_cast<int>(_actors.size() - 1));
		} else {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���Ƴ���!";
			LOG_ERROR(ss.str());
			return false;
		}
		// ����������Ϣ
		_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
		// ������ж���ѡ��
		clearActionOptions();
		// ֪ͨ�����˶���ѡ�����
		notifyActionOptionsFinish();
		// ֪ͨ��������Ҹ���
		notifyGangTile(pAvatar);
		// ֪ͨ�������
		notifyTingTile(pAvatar);

		if (acOp.getType() == MahjongAction::Type::ZhiGang)
			updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		if (bQiangGang) {
			// ֪ͨ�ȴ�������Ҷ���(�ȴ������������)
			notifyActionOptionsWaiting(pAvatar);
			// ֪ͨ�����������
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
					LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
			// ����ȴ�����ѡ��״̬
			changeState(StateMachine::Action);
		} else {
			// �ܺ���������
			fetchTile(true);
		}
		return true;
	}

	bool MahjongRoom::executePeng() {
		// �ȴ��������ѡ�������
		if (!_acOps1[0].empty())
			return false;
		// �ȴ��������ѡ��ܶ���
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
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���Ƴ���!";
			LOG_ERROR(ss.str());
			return false;
		}
		int player = _actor;
		if (!pAvatar->doPeng(mt, static_cast<int>(_actions.size()), player)) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")����ʧ��!";
			LOG_ERROR(ss.str());
			return false;
		}
		MahjongAvatar* pCurAva = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		pCurAva->setPlayedTileAction(mt.getId(), MahjongAction::Type::Peng, acOp.getPlayer());

		updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		// ������ж���ѡ��
		clearActionOptions();
		// ֪ͨ�����˶���ѡ�����
		notifyActionOptionsFinish();
		// ֪ͨ�������������
		notifyPengChiTile(pAvatar, true);
		// ����֮��֪ͨ���ƻ��߸�
		afterFetchChiPeng(pAvatar);

		return true;
	}

	bool MahjongRoom::executeChi() {
		// �ȴ��������ѡ�������
		if (!_acOps1[0].empty())
			return false;
		// �ȴ��������ѡ��ܶ���
		if (!_acOps1[1].empty())
			return false;
		// �ȴ��������ѡ��������
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
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���Ƴ���!";
			LOG_ERROR(ss.str());
			return false;
		}
		int player = _actor;
		if (!pAvatar->doChi(mt, acOp.getTileId1(), acOp.getTileId2(), static_cast<int>(_actions.size()), player)) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")����ʧ��!";
			LOG_ERROR(ss.str());
			return false;
		}
		MahjongAvatar* pCurAva = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		pCurAva->setPlayedTileAction(mt.getId(), MahjongAction::Type::Chi, acOp.getPlayer());

		updateCurrentActor(acOp.getPlayer());
		_actions.push_back(ma);

		// ������ж���ѡ��
		clearActionOptions();
		// ֪ͨ�����˶���ѡ�����
		notifyActionOptionsFinish();
		// ֪ͨ��������ҳ���
		notifyPengChiTile(pAvatar, false);
		// ����֮��֪ͨ���ƻ��߸�
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
						LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
					LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
			// ����ȴ�����ѡ��״̬
			changeState(StateMachine::Action);
		} else {
			// ֪ͨ��ҳ���
			id = _acOpIdAlloc.askForId();
			if (id >= ACTION_OPTION_POOL_SIZE) {
				LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
				return;
			}
			_acOpPool[id].setType(MahjongAction::Type::Play);
			_acOpPool[id].setId(id);
			_acOpPool[id].setPlayer(_actor);
			pAvatar->addActionOption(id);

			// ����ȴ�����״̬
			changeState(StateMachine::Play);
		}
		notifyActionOptions(pAvatar);
	}

	bool MahjongRoom::executePlay(int tileId) {
		// �ȴ��������ѡ�������
		if (!_acOps1[0].empty())
			return false;
		// �ȴ��������ѡ��ܶ���
		if (!_acOps1[1].empty())
			return false;
		// �ȴ��������ѡ��������
		if (!_acOps1[2].empty())
			return false;
		// �ȴ��������ѡ��Զ���
		if (!_acOps1[3].empty())
			return false;

		std::stringstream ss;
		MahjongAvatar* pAvatar = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
		if (!pAvatar->playTile(tileId)) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")����ʧ��!";
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
		
		// ����������Ϣ
		_rule->checkTingPai(pAvatar->getTiles(), pAvatar->getGangTiles(), pAvatar->getTingTiles(), pAvatar);
		// ������ж���ѡ��
		clearActionOptions();
		// ֪ͨ������ҵ�ǰ���ҳ���
		notifyPlayTile(mt);
		// ֪ͨ�������
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
					LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
					LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
					LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
						LOG_ERROR("�߼����󣬶���id���ڶ���ѡ��ش�С");
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
			// û���κζ���ѡ��ж��Ƿ�Ҫ�ٴ����ƣ����еĵط��Ĺ����Ǵ������֮��Ҫ������
			// ����Ҫ���л�����һλ���
			if (!fetchAgainAfterPlay())
				updateCurrentActor();
			fetchTile();
		} else {
			// ����ȴ�����ѡ��״̬
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
		// ������ж���ѡ��
		clearActionOptions();
		// ֪ͨ����ѡ�����
		notifyActionOptionsFinish();
		// �������״̬
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
				// ������������Ϊ�����õ����һ����
				_huTileId = pAvatar->getFetchedTileId();
				// ����������ֻ�������ƣ���Ϊȫ����
				if (pAvatar->getTileNums() == 2)
					pAvatar->addHuWay(MahjongGenre::HuWay::QuanQiuRen);
				if (i == _banker) {
					// ׯ�������һ���ƺ��ƣ����
					if ((pAvatar->getPlayedTileNums() == 0) && noChiPengGang())
						pAvatar->addHuWay(MahjongGenre::HuWay::TianHu);
				}
				else if ((pAvatar->getPlayedTileNums() == 0)) {
					// �м������һ���ƺ��ƣ�����������û���κγ����ܣ��˺�
					if (noChiPengGang())
						pAvatar->addHuWay(MahjongGenre::HuWay::RenHu);
				}
				if (_tilesLeft >= _dealer.getTileLeft())
					pAvatar->addHuWay(MahjongGenre::HuWay::HaiDiLaoYue);	// �Ƴ��Ѿ�û��ʣ����ƣ���������
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
									ss << "�߼���������(Id: " << getId() << ")���ƣ����(λ��: " << k << ")Ϊ��!";
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
				// ����
				pAvatar->addJiePao();
				if (pAvaFangPao == nullptr) {
					pAvaFangPao = dynamic_cast<MahjongAvatar*>(getAvatar(_actor).get());
					if (pAvaFangPao == nullptr)
						continue;
					pAvaFangPao->addFangPao();
				}
				// ���ڵ�������Ͻ�ʣһ���ƣ�Ϊȫ����
				if (pAvaFangPao->getTileNums() == 1)
					pAvatar->addHuWay(MahjongGenre::HuWay::QuanQiuPao);
				if ((pAvatar->getPlayedTileNums() == 0) && noChiPengGang()) {
					if (_actor == _banker)
						pAvatar->addHuWay(MahjongGenre::HuWay::DiHu);	// ׯ�Ҵ����һ���Ʒ��ڣ��غ�
					else
						pAvatar->addHuWay(MahjongGenre::HuWay::RenHu);	// �мҴ����һ���Ʒ��ڣ��˺�
				}
				if (_tilesLeft >= _dealer.getTileLeft())
					pAvatar->addHuWay(MahjongGenre::HuWay::HaiDiPao);	// �Ƴ��Ѿ�û��ʣ����ƣ�������
				nTemp = 0;
				if (_waitingQiangGang) {
					// ���ܺ���������Ϊ�Ӹܵ���
					_huTileId = _actions.back().getTile();
					pAvaFangPao->doQiangGang(_huTileId, static_cast<int>(_actions.size()));
					_gangHu = _actor;
				} else {
					// ����Ϊ����������һ����
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
		
		// ֪ͨ����
		notifyHuTile();
		// ��ʾ������ҵ�����
		notifyShowTiles();
		// ���
		calcHuScore();
		// ����
		doJieSuan();
		// �л���һ�ֵ�ׯ��
		bankerNextRound(nHuNums, banker);
		// �������Ķ�����
		int lastActor = _actor;
		// ���������
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
		// ��ԭ���Ķ�����
		_actor = lastActor;
		// ��������
		afterHu();
	}

	void MahjongRoom::noMoreTile() {
		// �������״̬
		changeState(StateMachine::End);
		// ���
		calcHuScore();
		// ֪ͨ����
		doJieSuan();
		// ��������
		afterHu();
		// �������һ�����Ƶ����Ϊׯ��
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
		// �����ѡ����ѡ��֮��֪ͨ�ȴ��������ѡ����ѡ��
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
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")û���κ��£���ʱ��Ӧ��֪ͨ�ܷ���!";
			LOG_ERROR(ss.str());
			return;
		}
		MsgGangTile msg;
		const MahjongChapter& mc = lstChaps.back();
		msg.chapter = static_cast<int>(mc.getType());
		if (!mc.isGang()) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���µ��²��Ǹ��ƣ���ʱ��Ӧ��֪ͨ�ܷ���!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongTileArray& lstTiles = mc.getAllTiles();
		if (lstTiles.size() < 4) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")�����в���������!";
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
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")û���κ��£���ʱ��Ӧ��֪ͨ��(��)����!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongChapter& mc = lstChaps.back();
		if ((bPeng && (mc.getType() != MahjongChapter::Type::Peng)) ||
			(!bPeng && (mc.getType() != MahjongChapter::Type::Chi))) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")���µ��²�����(��)�ƣ���ʱ��Ӧ��֪ͨ��(��)����!";
			LOG_ERROR(ss.str());
			return;
		}
		const MahjongTileArray& lstTiles = mc.getAllTiles();
		if (lstTiles.size() < 3) {
			ss << "�߼���������(Id: " << getId() << ")���(Id: " << pAvatar->getPlayerId() << ")��(��)���в���������!";
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
			ss << "�߼���������(Id: " << getId() << ")����3λ���ͬʱ����!";
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
		// һ�ڶ��죬�¾�ׯ��Ϊ������
		if (huNums > 1)
			_banker = _actor;
		else if (huNums == 1)
			_banker = huPlayer;
	}
}