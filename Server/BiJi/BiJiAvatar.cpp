// BiJiAvatar.cpp

#include "Base/Log.h"
#include "BiJiRule.h"
#include "BiJiAvatar.h"

#include <map>
#include <algorithm>

namespace NiuMa
{
	BiJiAvatar::BiJiAvatar(const std::string& playerId, int seat, bool robot)
		: GameAvatar(playerId, robot)
		, _joinRound(false)
		, _offlines(0)
	{
		setSeat(seat);
		clear();
	}

	BiJiAvatar::~BiJiAvatar()
	{}

	void BiJiAvatar::clear() {
		_occupiedIds.clear();
		_cardIds.clear();
		for (int i = 0; i < 9; i++) {
			_orderRaw[i] = -1;
			_orderSuit[i] = -1;
		}
		for (int i = 0; i < 3; i++) {
			_scores[i] = 0;
			_dunOKs[i] = false;
		}
		_winGold = 0;
		_fixed = false;
		_qiPaiOrder = -1;
		_rewardType = 0;
		_reward = 0;
		for (int i = 0; i < 5; i++)
			_supportedGenres[i] = false;
	}

	void BiJiAvatar::setJoinRound() {
		_joinRound = true;
	}

	bool BiJiAvatar::isJoinRound() const {
		return _joinRound;
	}

	void BiJiAvatar::dealCards(const CardArray& cards, const std::shared_ptr<BiJiRule>& rule) {
		if (rule == NULL || cards.size() != 9)
			return;
		_cards = cards;
		std::sort(_cards.begin(), _cards.end(), CardComparator(rule));

		int suit = 0;
		int point = 0;
		int ret = 0;
		std::map<int, int> temp;
		for (unsigned int i = 0; i < 9; i++) {
			const PokerCard& c = _cards[8 - i];
			_orderRaw[i] = c.getId();
		}
		for (unsigned int i = 0; i < 9; i++) {
			const PokerCard& c = cards[i];
			point = rule->getPointOrder(c.getPoint());
			suit = cards[i].getSuit();
			ret = suit * 14 + point;
			temp.insert(std::make_pair(ret, c.getId()));
		}
		ret = 0;
		std::map<int, int>::const_reverse_iterator it = temp.rbegin();
		while (it != temp.rend()) {
			_orderSuit[ret] = it->second;
			++ret;
			++it;
		}
		_cardIds.clear();
		CardArray::const_iterator it1 = _cards.begin();
		while (it1 != _cards.end()) {
			ret = it1->toInt32();
			_cardIds.insert(std::make_pair(ret, it1->getId()));
			++it1;
		}
	}

	const CardArray& BiJiAvatar::getCards() const {
		return _cards;
	}

	void BiJiAvatar::getOrderRaw(int arr[]) const {
		for (int i = 0; i < 9; i++)
			arr[i] = _orderRaw[i];
	}

	void BiJiAvatar::getOrderSuit(int arr[]) const {
		for (int i = 0; i < 9; i++)
			arr[i] = _orderSuit[i];
	}

	const PokerGenre& BiJiAvatar::getDun(int dun) const {
		if (dun < 0 || dun > 2)
			return PokerGenre::NullGenre;

		return _duns[dun];
	}

	bool BiJiAvatar::makeDun(int dun, int ids[], const std::shared_ptr<BiJiRule>& rule) {
		if (dun < 0 || dun > 2 || (ids == NULL))
			return false;
		if (_fixed || _dunOKs[dun])
			return false;
		std::unordered_set<int>::const_iterator it1;
		for (int i = 0; i < 3; i++) {
			it1 = _occupiedIds.find(ids[i]);
			if (it1 != _occupiedIds.end()) {
				ErrorS << "玩家(ID: " << getPlayerId() << ")配墩失败，牌(ID: " << ids[i] << ")已经被其他墩使用。";
				return false;
			}
		}
		bool found = false;
		CardArray cards;
		CardArray::const_iterator it;
		for (int i = 0; i < 3; i++) {
			found = false;
			it = _cards.begin();
			while (it != _cards.end()) {
				if (it->getId() == ids[i]) {
					cards.push_back(*it);
					found = true;
					break;
				}
				++it;
			}
			if (!found) {
				ErrorS << "玩家(ID: " << getPlayerId() << ")配墩失败，找不到牌ID为" << ids[i] << "的牌。";
				return false;
			}
		}
		_duns[dun].setCards(cards, rule);
		for (int i = 0; i < 3; i++)
			_occupiedIds.insert(ids[i]);
		_dunOKs[dun] = true;
		finalDun(rule);
		return true;
	}

	void BiJiAvatar::finalDun(const std::shared_ptr<BiJiRule>& rule) {
		if (_occupiedIds.size() != 6)
			return;
		int dun = 0;
		for (int i = 0; i < 3; i++) {
			if (!_dunOKs[i]) {
				dun = i;
				break;
			}
		}
		int nums = 0;
		int ids[3] = { 0, 0, 0 };
		CardArray::const_iterator it = _cards.begin();
		std::unordered_set<int>::const_iterator it1;
		while (it != _cards.end()) {
			it1 = _occupiedIds.find(it->getId());
			if (it1 == _occupiedIds.end()) {
				ids[nums] = it->getId();
				nums++;
				if (nums == 3)
					break;
			}
			++it;
		}
		makeDun(dun, ids, rule);
	}

	bool BiJiAvatar::revocateDun(int dun) {
		if (dun < 0 || dun > 2)
			return false;
		if (_fixed || !_dunOKs[dun])
			return false;
		int ids[3] = { 0, 0, 0 };
		getDunIds(dun, ids);
		std::unordered_set<int>::const_iterator it;
		for (int i = 0; i < 3; i++){
			it = _occupiedIds.find(ids[i]);
			if (it != _occupiedIds.end())
				_occupiedIds.erase(it);
		}
		_dunOKs[dun] = false;

		return true;
	}

	bool BiJiAvatar::isDunOK(int dun) const {
		if (dun < 0 || dun > 2)
			return false;
		return _dunOKs[dun];
	}

	bool BiJiAvatar::getDunIds(int dun, int ids[]) const {
		if (dun < 0 || dun > 2)
			return false;
		if (!_dunOKs[dun])
			return false;
		const CardArray& cards = _duns[dun].getCards();
		for (unsigned int i = 0; i < 3; i++)
			ids[i] = cards[i].getId();
		return true;
	}

	void BiJiAvatar::checkSupportGenre(const std::shared_ptr<BiJiRule>& rule) {
		for (int i = 0; i < 14; i++) {
			_points[i] = 0;
			for (int j = 0; j < 4; j++)
				_suits[i][j] = false;
		}
		for (int i = 0; i < 5; i++)
			_supportedGenres[i] = false;
		int suit = 0;
		int point = 0;
		int ret = 0;
		CardArray::const_iterator it1 = _cards.begin();
		std::unordered_set<int>::const_iterator it2;
		while (it1 != _cards.end()) {
			it2 = _occupiedIds.find(it1->getId());
			if (it2 != _occupiedIds.end()) {
				++it1;
				continue;
			}
			point = it1->getPoint();
			suit = it1->getSuit();
			ret = rule->getPointOrder(point);
			_points[ret] += 1;
			if (suit == static_cast<int>(PokerSuit::Little)) {
				_suits[ret][1] = true;
				_suits[ret][3] = true;
			}
			else if (suit == static_cast<int>(PokerSuit::Big)) {
				_suits[ret][0] = true;
				_suits[ret][2] = true;
			}
			else
				_suits[ret][suit - 1] = true;
			++it1;
		}
		for (int i = 1; i < 14; i++) {
			if (_points[i] > 1)	// 支持对子
				_supportedGenres[4] = true;
			if (_points[i] > 2) // 支持三条
				_supportedGenres[0] = true;
		}
		int idx1 = 0;
		int idx2 = 0;
		int idx3 = 0;
		for (int i = 0; i < 12; i++) {
			// 支持同花顺(必定同时支持顺子和同花，因此循环没必要再执行下去)
			if (_supportedGenres[1])
				break;
			if (i == 0)	// 最小A23顺子
				idx1 = 13;
			else
				idx1 = i;
			idx2 = i + 1;
			idx3 = i + 2;
			if ((_points[idx1] > 0) && (_points[idx2] > 0) && (_points[idx3] > 0)) {
				_supportedGenres[3] = true;	// 支持顺子
				for (int j = 0; j < 4; j++) {
					if (_suits[idx1][j] && _suits[idx2][j] && _suits[idx3][j]) {
						_supportedGenres[1] = true;
						_supportedGenres[2] = true;
						break;
					}
				}
			}
		}
		int nums = 0;
		for (int i = 0; i < 4; i++) {
			if (_supportedGenres[2])
				break;
			nums = 0;
			for (int j = 0; j < 14; j++) {
				if (_suits[j][i])
					nums = nums + 1;
				if (nums >= 3) {
					_supportedGenres[2] = true;
					break;
				}
			}
		}
	}

	bool BiJiAvatar::isSupportGenre(BiJiGenre genre) const {
		if (_occupiedIds.size() == 9)
			return false;
		if (BiJiGenre::Single == genre)
			return true;
		const BiJiGenre GENRES[5] = { BiJiGenre::Triple, BiJiGenre::FlushStraight, BiJiGenre::Flush, BiJiGenre::Straight, BiJiGenre::Pair };
		int idx = -1;
		for (int i = 0; i < 5; i++) {
			if (genre == GENRES[i]) {
				idx = i;
				break;
			}
		}
		if (idx < 0)
			return false;
		return _supportedGenres[idx];
	}

	bool BiJiAvatar::getGenreIds(BiJiGenre genre, int ids[], const std::shared_ptr<BiJiRule>& rule) const {
		if (!isSupportGenre(genre))
			return false;
		if (BiJiGenre::Triple == genre)
			return get3TiaoIds(ids, rule);
		else if (BiJiGenre::FlushStraight == genre)
			return getShunZiIds(true, ids, rule);
		else if (BiJiGenre::Flush == genre)
			return getTongHuaIds(ids, rule);
		else if (BiJiGenre::Straight == genre)
			return getShunZiIds(false, ids, rule);
		else if (BiJiGenre::Pair == genre)
			return getDuiZiIds(ids, rule);
		return getWuLongIds(ids);
	}

	bool BiJiAvatar::get3TiaoIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const {
		int point = 0;
		int suit = 0;
		int ret = 0;
		int nums = 0;
		PokerCard c;
		std::unordered_map<int, int>::const_iterator it;
		for (int i = 13; i > 0; i--) {
			if (_points[i] < 3)
				continue;

			point = rule->getPointByOrder(i);
			for (int j = 0; j < 4; j++) {
				if (!_suits[i][j])
					continue;
				suit = j + 1;
				c.setPoint(point);
				c.setSuit(suit);
				ret = c.toInt32();
				it = _cardIds.find(ret);
				if (it != _cardIds.end()) {
					ids[nums] = it->second;
					nums++;
					if (nums == 3)
						break;
				}
			}
			break;
		}
		return (nums == 3);
	}

	bool BiJiAvatar::getShunZiIds(bool tongHua, int ids[], const std::shared_ptr<BiJiRule>& rule) const {
		int points[3] = { 0, 0, 0 };
		int suits[3] = { 0, 0, 0 };
		int ret = 0;
		int nums = 0;
		int idx1 = 0;
		int idx2 = 0;
		int idx3 = 0;
		bool test = false;
		PokerCard c;
		std::unordered_map<int, int>::const_iterator it;
		for (int i = 11; i > -1; i--) {
			if (i == 0)
				idx1 = 13;	// A23顺子
			else
				idx1 = i;
			idx2 = i + 1;
			idx3 = i + 2;
			if (_points[idx1] < 1 || _points[idx2] < 1 || _points[idx3] < 1)
				continue;
			if (tongHua) {
				for (int j = 3; j > -1; j--) {
					if (_suits[idx1][j] && _suits[idx2][j] && _suits[idx3][j]) {
						points[0] = rule->getPointByOrder(idx1);
						points[1] = rule->getPointByOrder(idx2);
						points[2] = rule->getPointByOrder(idx3);
						for (int k = 0; k < 3; k++)
							suits[k] = j + 1;
						test = true;
						break;
					}
				}
			}
			else {
				test = true;
				points[0] = rule->getPointByOrder(idx1);
				points[1] = rule->getPointByOrder(idx2);
				points[2] = rule->getPointByOrder(idx3);
				for (int j = 3; j > -1; j--) {
					if (_suits[idx1][j]) {
						suits[0] = j + 1;
						break;
					}
				}
				for (int j = 3; j > -1; j--) {
					if (_suits[idx2][j]) {
						suits[1] = j + 1;
						break;
					}
				}
				for (int j = 3; j > -1; j--) {
					if (_suits[idx3][j]) {
						suits[2] = j + 1;
						break;
					}
				}
			}
			if (test) {
				for (int j = 0; j < 3; j++) {
					c.setPoint(points[j]);
					c.setSuit(suits[j]);
					ret = c.toInt32();
					it = _cardIds.find(ret);
					if (it == _cardIds.end())
						continue;
					ids[nums] = it->second;
					nums++;
				}
				break;
			}
		}
		return (nums == 3);
	}

	bool BiJiAvatar::getTongHuaIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const {
		int points[3] = { 0, 0, 0 };
		int ret = 0;
		int nums = 0;
		bool test = false;
		PokerCard c;
		std::unordered_map<int, int>::const_iterator it;
		for (int i = 13; i > -1; i--) {
			if (_points[i] < 1)
				continue;
			for (int j = i - 1; j > 0; j--) {
				if (_points[j] < 1)
					continue;
				for (int k = j - 1; k > -1; k--) {
					if (_points[k] < 1)
						continue;
					for (int p = 3; p > -1; p--) {
						if (!_suits[i][p] || !_suits[j][p] || !_suits[k][p])
							continue;
						points[0] = rule->getPointByOrder(i);
						points[1] = rule->getPointByOrder(j);
						points[2] = rule->getPointByOrder(k);
						for (int q = 0; q < 3; q++) {
							c.setPoint(points[q]);
							if (q == 2 && k == 0) {
								if (p == 3 || p == 1)
									c.setSuit(static_cast<int>(PokerSuit::Little));
								else
									c.setSuit(static_cast<int>(PokerSuit::Big));
							}
							else
								c.setSuit(p + 1);
							ret = c.toInt32();
							it = _cardIds.find(ret);
							if (it == _cardIds.end())
								continue;
							ids[nums] = it->second;
							nums++;
						}
						test = true;
						break;
					}
					if (test)
						break;
				}
				if (test)
					break;
			}
			if (test)
				break;
		}
		return (nums == 3);
	}

	bool BiJiAvatar::getDuiZiIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const {
		int points[3] = { 0, 0, 0 };
		int suits[3] = { 0, 0, 0 };
		int ret = 0;
		int nums = 0;
		int p = -1;
		if (_points[0] > 1) {
			// 大小王是最大的对子
			points[0] = static_cast<int>(PokerPoint::Joker);
			points[1] = static_cast<int>(PokerPoint::Joker);
			suits[0] = static_cast<int>(PokerSuit::Big);
			suits[1] = static_cast<int>(PokerSuit::Little);
			nums = 2;
			p = 0;
		}
		else {
			for (int i = 13; i > 0; i--) {
				if (_points[i] < 2)
					continue;
				p = i;
				points[0] = rule->getPointByOrder(i);
				points[1] = points[0];
				for (int j = 0; j < 4; j++) {
					if (_suits[i][j]) {
						suits[nums] = j + 1;
						nums++;
						if (nums == 2)
							break;
					}
				}
				break;
			}
			if (nums != 2)
				return false;
		}
		for (int i = 0; i < 14; i++) {
			if (i == p)
				continue;
			if (_points[i] < 1)
				continue;
			points[2] = rule->getPointByOrder(i);
			for (int j = 0; j < 4; j++) {
				if (!_suits[i][j])
					continue;
				if (i == 0) {
					if (j == 0)
						suits[2] = static_cast<int>(PokerSuit::Big);
					else
						suits[2] = static_cast<int>(PokerSuit::Little);
				}
				else
					suits[2] = j + 1;
				nums++;
				break;
			}
			break;
		}
		if (nums != 3)
			return false;
		nums = 0;
		PokerCard c;
		std::unordered_map<int, int>::const_iterator it;
		for (int i = 0; i < 3; i++) {
			c.setPoint(points[i]);
			c.setSuit(suits[i]);
			ret = c.toInt32();
			it = _cardIds.find(ret);
			if (it == _cardIds.end())
				continue;
			ids[nums] = it->second;
			nums++;
		}
		return (nums == 3);
	}

	bool BiJiAvatar::getWuLongIds(int ids[]) const {
		int nums = 0;
		std::unordered_set<int>::const_iterator it1;
		CardArray::const_reverse_iterator it = _cards.rbegin();
		while (it != _cards.rend()) {
			it1 = _occupiedIds.find(it->getId());
			if (it1 == _occupiedIds.end()) {
				ids[0] = it->getId();
				nums++;
				break;
			}
			++it;
		}
		if (nums == 0)
			return false;
		CardArray::const_iterator it2 = _cards.begin();
		while (it2 != _cards.end()) {
			it1 = _occupiedIds.find(it2->getId());
			if (it1 != _occupiedIds.end()) {
				++it2;
				continue;
			}
			ids[nums] = it2->getId();
			if (ids[nums] == ids[0])
				break;
			++nums;
			if (nums == 3)
				break;
			++it2;
		}
		return (nums == 3);
	}

	bool BiJiAvatar::sortDuns(bool duns[], const std::shared_ptr<BiJiRule>& rule) {
		for (int i = 0; i < 3; i++) {
			if (!_dunOKs[i])
				return false;
		}
		int orders[3] = { 0, 0, 0 };
		calcGenreOrders(orders, rule);
		bool test = false;
		for (int i = 0; i < 3; i++) {
			duns[i] = false;
			if (orders[i] != i)
				test = true;
		}
		if (!test)
			return false;
		PokerGenre genres[3];
		for (int i = 0; i < 3; i++)
			genres[i] = _duns[i];
		for (int i = 0; i < 3; i++) {
			if (orders[i] != i) {
				_duns[i] = genres[orders[i]];
				duns[i] = true;
			}
			else
				duns[i] = false;
		}
		return true;
	}

	void BiJiAvatar::calcGenreOrders(int orders[], const std::shared_ptr<BiJiRule>& rule) const {
		int ret1 = rule->compareGenre(_duns[0], _duns[1]);
		int ret2 = rule->compareGenre(_duns[0], _duns[2]);
		if (ret1 == 1) {
			if (ret2 == 1) {
				orders[0] = 0;
				int ret3 = rule->compareGenre(_duns[1], _duns[2]);
				if (ret3 == 1) {
					// 0 > 1 > 2
					orders[1] = 1;
					orders[2] = 2;
				}
				else {
					// 0 > 2 > 1
					orders[1] = 2;
					orders[2] = 1;
				}
			}
			else {
				// 2 > 0 > 1
				orders[0] = 2;
				orders[1] = 0;
				orders[2] = 1;
			}
		}
		else if (ret2 == 1) {
			// 1 > 0 > 2
			orders[0] = 1;
			orders[1] = 0;
			orders[2] = 2;
		}
		else {
			int ret3 = rule->compareGenre(_duns[1], _duns[2]);
			if (ret3 == 1) {
				// 1 > 2 > 0
				orders[0] = 1;
				orders[1] = 2;
				orders[2] = 0;
			}
			else {
				// 2 > 1 > 0
				orders[0] = 2;
				orders[1] = 1;
				orders[2] = 0;
			}
		}
		int tmp = orders[0];
		orders[0] = orders[2];
		orders[2] = tmp;
	}

	void BiJiAvatar::setFixed() {
		_fixed = true;
	}

	bool BiJiAvatar::isFixed() const {
		return _fixed;
	}

	void BiJiAvatar::setQiPaiOrder(int order) {
		_qiPaiOrder = order;
	}

	int BiJiAvatar::getQiPaiOrder() const {
		return _qiPaiOrder;
	}

	bool BiJiAvatar::isGiveUp() const {
		return (_qiPaiOrder != -1);
	}

	void BiJiAvatar::setDunScore(int d, int s) {
		if (d < 0 || d > 2)
			return;

		_scores[d] = s;
	}

	int BiJiAvatar::getDunScore(int d) const {
		if (d < 0 || d > 2)
			return 0;

		return _scores[d];
	}

	void BiJiAvatar::setWinGold(int64_t winGold) {
		_winGold = winGold;
	}

	int64_t BiJiAvatar::getWinGold() const {
		return _winGold;
	}

	void BiJiAvatar::detectRewardType(const std::shared_ptr<BiJiRule>& rule) {
		_rewardType = 0;
		if (isGiveUp())
			return;
		if (_scores[0] > 0 && _scores[1] > 0 && _scores[2] > 0)
			_rewardType |= static_cast<int>(BiJiRewardType::TongGuan);		// 通关
		bool bBlack = false;
		bool bRed = false;
		int suit = 0;
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			suit = it->getSuit();
			if ((suit == static_cast<int>(PokerSuit::Spade)) ||
				(suit == static_cast<int>(PokerSuit::Club)) ||
				(suit == static_cast<int>(PokerSuit::Little)))
				bBlack = true;
			else
				bRed = true;
			if (bBlack && bRed)
				break;
			++it;
		}
		if (bBlack) {
			if (!bRed)
				_rewardType |= static_cast<int>(BiJiRewardType::QuanHeiSe);
		}
		else if (bRed)
			_rewardType |= static_cast<int>(BiJiRewardType::QuanHongSe);
		for (int i = 0; i < 14; i++) {
			_points[i] = 0;
			for (int j = 0; j < 4; j++)
				_suits[i][j] = false;
		}
		int order = 0;
		it = _cards.begin();
		while (it != _cards.end()) {
			order = rule->getPointOrder(it->getPoint());
			suit = it->getSuit();
			_points[order]++;
			if (suit == static_cast<int>(PokerSuit::Little)) {
				_suits[order][1] = true;
				_suits[order][3] = true;
			}
			else if (suit == static_cast<int>(PokerSuit::Big)) {
				_suits[order][0] = true;
				_suits[order][2] = true;
			}
			else
				_suits[order][suit - 1] = true;
			++it;
		}
		int nums = 0;
		for (int i = 1; i < 14; i++) {
			if (_points[i] > 2)
				nums++;
			if (_points[i] > 3)
				_rewardType |= static_cast<int>(BiJiRewardType::SiZhang);
		}
		if (nums > 1)
			_rewardType |= static_cast<int>(BiJiRewardType::ShuangBaoZi);
		if (nums > 2)
			_rewardType |= static_cast<int>(BiJiRewardType::QuanSanTiao);

		int start = 0;
		int idx = 0;
		int pos0 = 0;
		int pos1 = 0;
		int pos2 = 0;
		// 判定全顺子
		if (_points[0] == 0) {
			// 有王就不会是全顺子
			nums = 0;
			_points[0] = _points[13];
			for (int i = 0; i < 14; i++) {
				if (_points[i] == 0) {
					if (nums > 0)
						break;
				}
				else if (_points[i] == 1) {
					nums++;
					if (nums == 9)
					{
						// 9连顺才是全顺子
						_rewardType |= static_cast<int>(BiJiRewardType::QuanShuanZi);
						break;
					}
				}
				else if (_points[i] > 1)
					break;
			}
		}
		for (int i = 0; i < 4; i++)
			_suits[0][i] = _suits[13][i];
		start = 0;
		while (start < 2) {
			nums = 0;
			for (int i = 0; i < 4; i++) {
				idx = 0;
				while (idx < 11) {
					pos0 = start + idx + 0;
					pos1 = start + idx + 1;
					pos2 = start + idx + 2;
					if (_suits[pos0][i] && _suits[pos1][i] && _suits[pos2][i]) {
						nums++;
						if (nums > 1) {
							_rewardType |= static_cast<int>(BiJiRewardType::ShuangTongHuaShun);
							break;
						}
						idx += 3;
					}
					else
						idx++;
				}
				if (nums > 1)
					break;
			}
			if (nums > 1)
				break;
			++start;
		}
	}

	int BiJiAvatar::getRewardType() const {
		return _rewardType;
	}

	void BiJiAvatar::setReward(int r) {
		_reward = r;
	}

	int BiJiAvatar::getReward() const {
		return _reward;
	}

	int BiJiAvatar::getTotal() const {
		int total = 0;
		for (int i = 0; i < 3; i++)
			total += _scores[i];
		total += _reward;
		return total;
	}

	void BiJiAvatar::addOfflines() {
		_offlines++;
	}

	void BiJiAvatar::emptyOfflines() {
		_offlines = 0;
	}

	int BiJiAvatar::getOfflines() const {
		return _offlines;
	}
}
