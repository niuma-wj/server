// LackeyAvatar.cpp

#include "Base/Log.h"
#include "PokerUtilities.h"
#include "LackeyRule.h"
#include "LackeyAvatar.h"

namespace NiuMa
{
	LackeyAvatar::LackeyAvatar(const std::shared_ptr<LackeyRule>& rule, const std::string& playerId, int seat, bool robot)
		: DouDiZhuAvatar(rule, playerId, seat, robot)
		, _lackeyCard(-1)
		, _xiQian(0)
	{}

	LackeyAvatar::~LackeyAvatar()
	{}

	void LackeyAvatar::setLackeyCard(int id) {
		_lackeyCard = id;
	}

	int LackeyAvatar::getLackeyCard() const {
		return _lackeyCard;
	}

	void LackeyAvatar::combineAllGenres() {
		for (unsigned int i = 0; i < 49; i++)
			combineGenre(LackeyRule::CANDIDATE_ORDERS[i]);
		completeButterfly();

		// 计算所有组合的破坏系数
		int order = 0;
		int nums = 0;
		int tmp = 0;
		int genre = 0;
		int bomb = 0;
		DouDiZhuCombination::Ptr comb;
		CombinationVec::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		std::unordered_map<int, int>::const_iterator it3;
		while (it2 != _combinations.end()) {
			CombinationVec& vec = (it2->second);
			it1 = vec.begin();
			while (it1 != vec.end()) {
				comb = *it1;
				nums = 0;
				genre = comb->getGenre();
				bomb = LackeyRule::getBombOrder(genre);
				const std::unordered_map<int, int>& orders = comb->getOrders();
				it3 = orders.begin();
				while (it3 != orders.end()) {
					order = it3->first;
					tmp = it3->second;
					++it3;
					if (bomb == -1) {
						// 非炸弹牌型
						if (order == 13) {
							if (comb->containsCard(_jokerCards[0])) {
								if (_jokerCards[0].size() > static_cast<std::size_t>(tmp))
									nums++;
								if (_jokerCards[0].size() > 2)	// 破坏王炸
									nums++;
							}
							if (comb->containsCard(_jokerCards[1])) {
								if (_jokerCards[1].size() > static_cast<std::size_t>(tmp))
									nums++;
								if (_jokerCards[1].size() > 2)	// 破坏王炸
									nums++;
							}
						}
						else {
							if (_pointNums[order] > tmp)
								nums++;
							if (_pointNums[order] > 3)	// 破坏炸弹
								nums++;
						}
					}
					else if (_pointNums[order] > tmp)	// 破坏更大的炸弹
						nums++;
				}
				comb->setDamages(nums);
				++it1;
			}
			++it2;
		}
	}

	void LackeyAvatar::combineGenre(int genre) {
		if (genre < static_cast<int>(LackeyGenre::Bomb4) &&
			genre > static_cast<int>(LackeyGenre::Triple12)) {
			combineButterfly(genre);
			if (genre == static_cast<int>(LackeyGenre::Butterfly1))
				combineButterfly();
		}
		if (genre < static_cast<int>(LackeyGenre::Butterfly1) &&
			genre > static_cast<int>(LackeyGenre::Pair12))
			combineTriple(genre);
		if (genre < static_cast<int>(LackeyGenre::Triple1) &&
			genre > static_cast<int>(LackeyGenre::Single)) {
			combinePair(genre);
			if (genre == static_cast<int>(LackeyGenre::Pair1))
				combinePair();
		}
		if (genre == static_cast<int>(LackeyGenre::Single))
			combineSingle();
		if (genre < static_cast<int>(LackeyGenre::Bomb3L) &&
			genre > static_cast<int>(LackeyGenre::Butterfly7))
			combineBomb(genre);
		if (genre < static_cast<int>(LackeyGenre::BombLackey) &&
			genre > static_cast<int>(LackeyGenre::Bomb12))
			combineBombJoker(genre);
		if ((genre == static_cast<int>(LackeyGenre::BombLackey)) && (_lackeyCard > 0))
			combineBombLackey();
	}

	void LackeyAvatar::combineButterfly(int genre) {
		bool test = true;
		int tmp = 0;
		int end = 12;
		int straights = genre - static_cast<int>(LackeyGenre::Butterfly1) + 1;
		DouDiZhuCombination::Ptr comb;
		std::vector<int> ids;
		std::string str;
		if (genre == static_cast<int>(LackeyGenre::Butterfly1))	// 3带2可以有2，其他蝴蝶牌型不可以
			end = 13;
		int tmpNums[14] = { 0 };
		for (int i = 0; i <= (end - straights); i++) {
			test = true;
			for (int j = 0; j < straights; j++) {
				if (_pointNums[i + j] < 3) {
					test = false;
					break;
				}
			}
			if (!test)
				continue;
#ifdef _MSC_VER
			memcpy_s(tmpNums, 14 * sizeof(int), _pointNums, 14 * sizeof(int));
#else
			memcpy(tmpNums, _pointNums, 14 * sizeof(int));
#endif
			for (int j = 0; j < straights; j++)
				tmpNums[i + j] -= 3;
			test = false;
			if (straights == 2) {
				// 蝴蝶2需要对子为连对
				for (int j = 0; j < 11; j++) {
					if ((tmpNums[j] > 1) && (tmpNums[j + 1] > 1)) {
						test = true;
						break;
					}
				}
			}
			else {
				// 其他蝴蝶不需要连对
				tmp = 0;
				for (int j = 0; j < end; j++) {
					// “3带2”的“3”与“2”不能是同点数牌(否则就是炸弹而不是3带2了)
					if ((genre == static_cast<int>(LackeyGenre::Butterfly1)) && (i == j))
						continue;
					if (tmpNums[j] > 1) {
						tmp++;
						if (tmp == straights) {
							test = true;
							break;
						}
					}
				}
				if (!test && (genre == static_cast<int>(LackeyGenre::Butterfly1))) {
					// 3带2可以带一对大王或者一对小王
					if (_jokerCards[0].size() > 1 || _jokerCards[1].size() > 1)
						test = true;
				}
			}
			if (!test)
				continue;
			comb = allocateCombination();
			comb->setGenre(genre);
			for (int j = 0; j < straights; j++) {
				if (!getCardIds(i + j, 3, ids)) {
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "检索牌型(" << genre << ")错误，牌数组: " << str;
					freeCombination(comb);
					test = false;
					break;
				}
				comb->addCards(ids);
				comb->addOrder(i + j, 3);
				if (j == (straights - 1))
					comb->setOfficer(ids.back());
			}
			if (test)
				addCombination(genre, comb);
		}
	}

	void LackeyAvatar::combineButterfly() {
		// 3-2已经检索了，这里检索3大王或3小王牌带一对
		if (_jokerCards[0].size() < 3 && _jokerCards[1].size() < 3)
			return;
		bool test = false;
		for (int i = 0; i < 13; i++) {
			if (_pointNums[i] > 1) {
				test = true;
				break;
			}
		}
		if (!test)
			return;
		int genre = static_cast<int>(LackeyGenre::Butterfly1);
		DouDiZhuCombination::Ptr comb;
		for (int i = 0; i < 2; i++) {
			if (_jokerCards[i].size() < 3)
				continue;
			comb = allocateCombination();
			comb->setGenre(genre);
			addCombination(genre, comb);
			for (unsigned int j = 0; j < 3; j++)
				comb->addCard(_jokerCards[i][j]);
			comb->addOrder(13, 3);
			comb->setOfficer(_jokerCards[i][0]);
		}
	}

	int findValue(const std::vector<int>& arr, int val) {
		int nums = static_cast<int>(arr.size());
		for (int i = 0; i < nums; i++) {
			if (arr[i] == val)
				return i;
		}
		return -1;
	}

	void LackeyAvatar::completeButterfly() {
		const LackeyGenre GENRES[7] = {
			LackeyGenre::Butterfly1,
			LackeyGenre::Butterfly2,
			LackeyGenre::Butterfly3,
			LackeyGenre::Butterfly4,
			LackeyGenre::Butterfly5,
			LackeyGenre::Butterfly6,
			LackeyGenre::Butterfly7,
		};
		int id = 0;
		int tmp = 0;
		int min = 0;
		int nums = 0;
		int nums1 = 0;
		int nums2 = 0;
		int genre = 0;
		int buf[14][3];
		int straights = 0;
		DouDiZhuCombination::Ptr comb;
		bool test = false;
		std::string str;
		std::vector<int> orders;
		CombinationMap::iterator it;
		CombinationVec::iterator it1;
		std::vector<int>::const_iterator it2;
		for (int i = 0; i < 7; i++) {
			genre = static_cast<int>(GENRES[i]);
			straights = genre - static_cast<int>(LackeyGenre::Butterfly1) + 1;
			it = _combinations.find(genre);
			if (it == _combinations.end())
				continue;
			CombinationVec& vec = it->second;
			if (vec.empty())
				continue;
			it1 = vec.begin();
			while (it1 != vec.end()) {
				comb = *it1;
				++it1;
				for (int j = 0; j < 14; j++)
					buf[j][0] = -1;
				for (int j = 0; j < 14; j++) {
					test = comb->containsOrder(j);
					if (genre != static_cast<int>(LackeyGenre::Butterfly1)) {
						// 只有3带2牌型可以有2及王
						if (j > 11)
							break;
					}
					else if (test) {
						// “3带2”的“3”与“2”不能是同点数牌(否则就是炸弹而不是3带2了)
						continue;
					}
					if (j == 13) {
						if (_jokerCards[0].size() < 2 && _jokerCards[1].size() < 2)
							continue;
					}
					else {
						tmp = _pointNums[j];
						if (test)
							tmp -= 3;
						if (tmp < 2)
							continue;
					}
					const std::vector<int>& ids = _pointCards[j];
					if (test) {
						nums = 0;
						it2 = ids.begin();
						while (it2 != ids.end()) {
							if (!comb->containsCard(*it2)) {
								nums++;
								buf[j][nums] = *it2;
								if (nums == 2)
									break;
							}
							++it2;
						}
						buf[j][0] = _pointNums[j] - 3;
					}
					else if (j == 13) {
						nums1 = static_cast<int>(_jokerCards[0].size());
						nums2 = static_cast<int>(_jokerCards[1].size());
						if (nums1 < 2)
							tmp = 1;
						else if (nums2 < 2)
							tmp = 0;
						else if (nums1 > nums2)
							tmp = 1;
						else
							tmp = 0;
						buf[j][1] = _jokerCards[tmp][0];
						buf[j][2] = _jokerCards[tmp][1];
						buf[j][0] = static_cast<int>(_jokerCards[tmp].size());
					}
					else {
						buf[j][1] = ids[0];
						buf[j][2] = ids[1];
						buf[j][0] = _pointNums[j];
					}
				}
				orders.clear();
				for (int j = 0; j < 14; j++) {
					if (buf[j][0] == -1)
						continue;
					test = false;
					it2 = orders.begin();
					while (it2 != orders.end()) {
						if (buf[*it2][0] > buf[j][0]) {
							orders.insert(it2, j);
							test = true;
							break;
						}
						++it2;
					}
					if (!test)
						orders.push_back(j);
				}
				if (orders.size() < static_cast<std::size_t>(straights)) {
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "补全蝴蝶牌型(" << genre << ")错误，牌数组: " << str;
					return;
				}
				if (genre == static_cast<int>(LackeyGenre::Butterfly2)) {
					// 蝴蝶2需要连对
					test = true;
					for (int j = 0; j < 11; j++) {
						nums1 = findValue(orders, j);
						nums2 = findValue(orders, j + 1);
						if (nums1 == -1 || nums2 == -1)
							continue;
						nums1 = buf[j][0];
						nums2 = buf[j + 1][0];
						if (nums1 > nums2)
							nums = nums1;
						else
							nums = nums2;
						if (test || min > nums) {
							test = false;
							tmp = j;
							min = nums;
						}
					}
					if (test) {
						PokerUtilities::cardArray2String(_cards, str);
						ErrorS << "补全蝴蝶2错误，牌数组: " << str;
						return;
					}
					for (int j = 0; j < 2; j++) {
						for (int k = 1; k < 3; k++) {
							id = buf[tmp + j][k];
							comb->addCard(id);
						}
						comb->addOrder(tmp + j, 2);
					}
				}
				else {
					for (int j = 0; j < straights; j++) {
						tmp = orders.at(j);
						for (int k = 1; k < 3; k++) {
							id = buf[tmp][k];
							comb->addCard(id);
						}
						comb->addOrder(tmp, 2);
					}
				}
			}
		}
	}

	void LackeyAvatar::combineTriple(int genre) {
		bool test = true;
		int end = 12;
		int straights = genre - static_cast<int>(LackeyGenre::Triple1) + 1;
		DouDiZhuCombination::Ptr comb;
		std::vector<int> ids;
		std::vector<int>::const_iterator it;
		std::string str;
		if (genre == static_cast<int>(LackeyGenre::Triple1))
			end = 13;
		for (int i = 0; i <= (end - straights); i++) {
			test = true;
			for (int j = 0; j < straights; j++) {
				if (_pointNums[i + j] < 3) {
					test = false;
					break;
				}
			}
			if (!test)
				continue;
			comb = allocateCombination();
			comb->setGenre(genre);
			for (int j = 0; j < straights; j++) {
				if (!getCardIds(i + j, 3, ids)) {
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "检索牌型(" << genre << ")错误，牌数组: " << str;
					freeCombination(comb);
					test = false;
					break;
				}
				comb->addCards(ids);
				comb->addOrder(i + j, 3);
				if (j == (straights - 1))
					comb->setOfficer(ids[2]);
			}
			if (test)
				addCombination(genre, comb);
		}
	}

	void LackeyAvatar::combinePair(int genre) {
		bool test = true;
		int end = 12;
		int straights = genre - static_cast<int>(LackeyGenre::Pair1) + 2;
		DouDiZhuCombination::Ptr comb;
		std::vector<int> ids;
		std::vector<int>::const_iterator it;
		std::set<int>::const_iterator it1;
		std::string str;
		if (genre == static_cast<int>(LackeyGenre::Pair1)) {
			end = 13;
			straights = 1;
		}
		for (int i = 0; i <= (end - straights); i++) {
			test = true;
			for (int j = 0; j < straights; j++) {
				if (_pointNums[i + j] < 2) {
					test = false;
					break;
				}
			}
			if (!test)
				continue;
			comb = allocateCombination();
			comb->setGenre(genre);
			for (int j = 0; j < straights; j++) {
				if (!getCardIds(i + j, 2, ids)) {
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "检索牌型(" << genre << ")错误，牌数组:" << str;
					freeCombination(comb);
					test = false;
					break;
				}
				comb->addCards(ids);
				comb->addOrder(i + j, 2);
				if (j == (straights - 1))
					comb->setOfficer(ids[1]);
			}
			if (test)
				addCombination(genre, comb);
		}
	}

	void LackeyAvatar::combinePair() {
		// 3-2的对子牌型已经检索过了，这里只需要检索对小王或对大王
		if (_pointNums[13] < 2)
			return;
		int id = 0;
		DouDiZhuCombination::Ptr comb;
		for (int i = 0; i < 2; i++) {
			if (_jokerCards[i].size() < 2)
				continue;
			comb = allocateCombination();
			comb->setGenre(static_cast<int>(LackeyGenre::Pair1));
			addCombination(static_cast<int>(LackeyGenre::Pair1), comb);
			for (int j = 0; j < 2; j++) {
				id = _jokerCards[i][j];
				comb->addCard(id);
			}
			comb->setOfficer(_jokerCards[i][0]);
			comb->addOrder(13, 2);
		}
	}

	void LackeyAvatar::combineSingle()
	{
		int order = 0;
		DouDiZhuCombination::Ptr comb;
		std::vector<int> ids;
		std::vector<int>::const_iterator it;
		std::string str;
		for (int i = 0; i < 15; i++) {
			if (i > 13)
				order = 13;
			else
				order = i;
			if (_pointNums[order] < 1)
				continue;
			comb = allocateCombination();
			comb->setGenre(static_cast<int>(LackeyGenre::Single));
			if (i == 13) {
				if (_jokerCards[0].empty())
					continue;
				ids.clear();
				ids.push_back(_jokerCards[0][0]);
			}
			else if (i == 14) {
				if (_jokerCards[1].empty())
					continue;
				ids.clear();
				ids.push_back(_jokerCards[1][0]);
			}
			else if (!getCardIds(order, 1, ids)) {
				PokerUtilities::cardArray2String(_cards, str);
				ErrorS << "检索牌型单张错误，牌数组: " << str;
				freeCombination(comb);
				continue;
			}
			if (ids[0] == _lackeyCard) {
				ids.clear();
				if (_pointCards[order].size() > 1)
					ids.push_back(_pointCards[order][1]);
				if (ids.empty())
					continue;
			}
			comb->addCards(ids);
			comb->setOfficer(ids[0]);
			comb->addOrder(order, 1);
			addCombination(static_cast<int>(LackeyGenre::Single), comb);
		}
	}

	void LackeyAvatar::combineBomb(int genre) {
		int nums = genre - static_cast<int>(LackeyGenre::Bomb4) + 4;
		DouDiZhuCombination::Ptr comb;
		std::vector<int> ids;
		std::vector<int>::const_iterator it;
		std::set<int>::const_iterator it1;
		std::string str;
		for (int i = 0; i < 13; i++) {
			if (_pointNums[i] < nums)
				continue;
			comb = allocateCombination();
			comb->setGenre(genre);
			if (!getCardIds(i, nums, ids)) {
				PokerUtilities::cardArray2String(_cards, str);
				ErrorS << "检索牌型(" << genre << ")错误，牌数组: " << str;
				freeCombination(comb);
				continue;
			}
			comb->addCards(ids);
			comb->addOrder(i, nums);
			comb->setOfficer(ids.back());
			addCombination(genre, comb);
		}
	}

	void LackeyAvatar::combineBombJoker(int genre) {
		if (_pointNums[13] < 3)
			return;
		int nums[2] = { 0, 0 };
		int needs[2] = { 0, 0 };
		nums[0] = static_cast<int>(_jokerCards[0].size());
		nums[1] = static_cast<int>(_jokerCards[1].size());
		LackeyGenre eGenre = static_cast<LackeyGenre>(genre);
		switch (eGenre) {
		case LackeyGenre::Bomb3L:
			needs[0] = 3;
			break;
		case LackeyGenre::Bomb3B:
			needs[1] = 3;
			break;
		case LackeyGenre::Bomb2B2L:
			needs[0] = 2;
			needs[1] = 2;
			break;
		case LackeyGenre::Bomb1B3L:
			needs[0] = 3;
			needs[1] = 1;
			break;
		case LackeyGenre::Bomb3B1L:
			needs[0] = 1;
			needs[1] = 3;
			break;
		case LackeyGenre::Bomb2B3L:
			needs[0] = 3;
			needs[1] = 2;
			break;
		case LackeyGenre::Bomb3B2L:
			needs[0] = 2;
			needs[1] = 3;
			break;
		case LackeyGenre::Bomb3B3L:
			needs[0] = 3;
			needs[1] = 3;
			break;
		default:
			break;
		}
		if (nums[0] < needs[0] || nums[1] < needs[1])
			return;
		int id = 0;
		DouDiZhuCombination::Ptr comb = allocateCombination();
		comb->setGenre(genre);
		addCombination(genre, comb);
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < needs[i]; j++) {
				id = _jokerCards[i].at(j);
				comb->addCard(id);
			}
		}
		comb->addOrder(13, needs[0] + needs[1]);
		comb->setOfficer(id);
	}

	void LackeyAvatar::combineBombLackey() {
		PokerCard c;
		if (!getCardById(_lackeyCard, c))
			return;
		int genre = static_cast<int>(LackeyGenre::BombLackey);
		DouDiZhuCombination::Ptr comb = allocateCombination();
		comb->setGenre(genre);
		comb->addCard(_lackeyCard);
		addCombination(genre, comb);
	}

	void LackeyAvatar::candidateCombinationsImpl() {
		// 按照组合顺序表的逆顺序来排列候选组合列表
		// 1、非炸弹组合，破坏系数为0
		// 2、炸弹组合，破坏系数为0
		// 3、非炸弹组合，破坏系数由小到大排列
		bool test = false;
		int genre = 0;
		DouDiZhuCombination::Ptr comb;
		CombinationVec combs;
		CombinationMap::const_iterator it1;
		CombinationVec::const_iterator it2;
		CombinationVec::const_iterator it3;
		for (int j = 0; j < 49; j++) {
			genre = LackeyRule::CANDIDATE_ORDERS[j];
			it1 = _combinations.find(genre);
			if (it1 == _combinations.end())
				continue;
			const CombinationVec& vec = it1->second;
			it2 = vec.begin();
			while (it2 != vec.end()) {
				comb = *it2;
				++it2;
				if (comb->isCandidate() || comb->getDamages() > 0)
					continue;
				_candidates.push_back(comb);
				comb->setCandidate();
			}
		}
		for (int j = 0; j < 31; j++) {
			genre = LackeyRule::CANDIDATE_ORDERS[j];
			it1 = _combinations.find(genre);
			if (it1 == _combinations.end())
				continue;
			const CombinationVec& vec = it1->second;
			it2 = vec.begin();
			while (it2 != vec.end()) {
				comb = *it2;
				++it2;
				if (comb->isCandidate())
					continue;
				test = false;
				it3 = combs.begin();
				while (it3 != combs.end()) {
					if (comb->getDamages() < (*it3)->getDamages()) {
						combs.insert(it3, comb);
						test = true;
						break;
					}
					++it3;
				}
				if (!test)
					combs.push_back(comb);
			}
		}
		it3 = combs.begin();
		while (it3 != combs.end()) {
			comb = (*it3);
			_candidates.push_back(comb);
			comb->setCandidate();
			++it3;
		}
	}

	void LackeyAvatar::candidateCombinationsImpl(const PokerGenre& pg) {
		int genre = pg.getGenre();
		if (!(_rule->isValidGenre(genre)))
			return;
		// 1、同牌型组合，破坏系数为0
		addCandidate(pg);
		// 2、更大的炸弹组合，破坏系数为0
		int order = LackeyRule::getBombOrder(genre);
		addCandidateBomb(order);
		// 3、同牌型，破坏系数由小到大
		addCandidateSort(pg);
	}

	void LackeyAvatar::addCandidate(const PokerGenre& pg) {
		int genre = pg.getGenre();
		CombinationMap::const_iterator it = _combinations.find(genre);
		if (it == _combinations.end())
			return;
		PokerCard c;
		int ret = 0;
		DouDiZhuCombination::Ptr comb;
		const CombinationVec& vec = it->second;
		CombinationVec::const_iterator it1 = vec.begin();
		while (it1 != vec.end()) {
			comb = (*it1);
			++it1;
			if (comb->isCandidate() || comb->getDamages() > 0)
				continue;
			if (!getCardById(comb->getOfficer(), c))
				continue;
			// 同牌型，比较主牌
			ret = _rule->compareCard(c, pg.getOfficer());
			if (ret == 1) {
				_candidates.push_back(comb);
				comb->setCandidate();
			}
		}
	}

	void LackeyAvatar::addCandidateSort(const PokerGenre& pg) {
		int genre = pg.getGenre();
		CombinationMap::const_iterator it = _combinations.find(genre);
		if (it == _combinations.end())
			return;
		bool test = false;
		PokerCard c;
		int ret = 0;
		DouDiZhuCombination::Ptr comb;
		CombinationVec combs;
		const CombinationVec& vec = it->second;
		CombinationVec::const_iterator it1 = vec.begin();
		CombinationVec::const_iterator it2;
		while (it1 != vec.end()) {
			comb = (*it1);
			++it1;
			if (comb->isCandidate())
				continue;
			if (!getCardById(comb->getOfficer(), c))
				continue;
			// 同牌型，比较主牌
			ret = _rule->compareCard(c, pg.getOfficer());
			if (ret != 1)
				continue;
			test = false;
			it2 = combs.begin();
			while (it2 != combs.end()) {
				if (comb->getDamages() < (*it2)->getDamages()) {
					combs.insert(it2, comb);
					test = true;
					break;
				}
				++it2;
			}
			if (!test)
				combs.push_back(comb);
		}
		it2 = combs.begin();
		while (it2 != combs.end()) {
			comb = (*it2);
			_candidates.push_back(comb);
			comb->setCandidate();
			++it2;
		}
	}

	void LackeyAvatar::addCandidateBomb(int order) {
		int genre = 0;
		DouDiZhuCombination::Ptr comb;
		CombinationMap::const_iterator it;
		CombinationVec::const_iterator it1;
		for (int i = order + 1; i < 18; i++) {
			genre = LackeyRule::getBombByOrder(i);
			it = _combinations.find(genre);
			if (it == _combinations.end())
				continue;
			const CombinationVec& vec = it->second;
			it1 = vec.begin();
			while (it1 != vec.end()) {
				comb = (*it1);
				++it1;
				if (comb->isCandidate() || comb->getDamages() > 0)
					continue;
				_candidates.push_back(comb);
				comb->setCandidate();
			}
		}
	}

	void LackeyAvatar::clear() {
		DouDiZhuAvatar::clear();

		_lackeyCard = -1;
		_xiQian = 0;
	}

	void LackeyAvatar::addXiQian(int q) {
		_xiQian += q;
	}

	int LackeyAvatar::getXiQian() const {
		return _xiQian;
	}

	void LackeyAvatar::calcLeftXiQian() {
		analyzeCombinations();

		int order = 0;
		int xiQian = 0;
		CardArray cards;
		std::vector<int> ids;
		DouDiZhuCombination::Ptr comb;
		CombinationVec::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		std::map<int, int>::const_iterator it3;
		while (it2 != _combinations.end()) {
			CombinationVec& vec = (it2->second);
			it1 = vec.begin();
			while (it1 != vec.end()) {
				comb = *it1;
				it1++;
				if (comb->getDamages() > 0)
					continue;
				if (comb->getGenre() == static_cast<int>(LackeyGenre::Bomb4))
					continue;
				order = LackeyRule::getBombOrder(comb->getGenre());
				if (order == -1)
					continue;
				cards.clear();
				comb->getCards(ids);
				if (!getCardsByIds(ids, cards))
					continue;
				xiQian += LackeyRule::calcXiQian(comb->getGenre(), cards);
			}
			++it2;
		}
		addXiQian(xiQian);
	}
}