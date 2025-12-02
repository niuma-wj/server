// GuanDanAvatar.cpp

#include "Base/Log.h"
#include "GuanDanAvatar.h"
#include "GuanDanRule.h"
#include "GuanDanCandidateOrder.h"
#include "PokerUtilities.h"

#include <boost/locale.hpp>

namespace NiuMa
{
	GuanDanAvatar::GuanDanAvatar(const std::shared_ptr<GuanDanRule>& rule, const std::string& playerId, int seat, bool robot)
		: PokerAvatar(rule, playerId, seat, robot)
		, _waitingPresentTribute(false)
		, _waitingRefundTribute(false)
		, _presentTributeSeat(-1)
		, _refundTributeSeat(-1)
		, _variableCards(0)
		, _occupiedVariableCards(0)
		, _searchId(1)
		, _groupId(1)
		, _straightFlushIndex(0)
	{
		for (int i = 0; i < 13; i++) {
			for (int j = 0; j < 4; j++)
				_pointOrderSuitGraph[i][j] = 0;
		}
	}

	GuanDanAvatar::~GuanDanAvatar()
	{}

	void GuanDanAvatar::clear() {
		PokerAvatar::clear();

		_waitingPresentTribute = false;
		_waitingRefundTribute = false;
		_presentTributeSeat = -1;
		_refundTributeSeat = -1;
		_refundTributeText.clear();
		_cardOut = PokerCard();
		_cardIn = PokerCard();

		_variableCards = 0;
		_occupiedVariableCards = 0;
		for (int i = 0; i < 13; i++) {
			for (int j = 0; j < 4; j++)
				_pointOrderSuitGraph[i][j] = 0;
		}
		_occupiedCardIds.clear();
	}

	void GuanDanAvatar::setWaitingPresentTribute(bool setting) {
		_waitingPresentTribute = setting;
	}

	bool GuanDanAvatar::isWaitingPresentTribute() const {
		return _waitingPresentTribute;
	}

	void GuanDanAvatar::setWaitingRefundTribute(bool setting) {
		_waitingRefundTribute = setting;
	}

	bool GuanDanAvatar::isWaitingRefundTribute() const {
		return _waitingRefundTribute;
	}

	void GuanDanAvatar::setPresentTributeSeat(int seat) {
		_presentTributeSeat = seat;
	}

	int GuanDanAvatar::getPresentTributeSeat() const {
		return _presentTributeSeat;
	}

	void GuanDanAvatar::setRefundTributeSeat(int seat) {
		_refundTributeSeat = seat;
	}

	int GuanDanAvatar::getRefundTributeSeat() const {
		return _refundTributeSeat;
	}

	void GuanDanAvatar::setRefundTributeText(const std::string& text) {
#ifdef _MSC_VER
		// VC环境下gb2312编码转utf8
		_refundTributeText = boost::locale::conv::to_utf<char>(text, std::string("gb2312"));
#else
		_refundTributeText = text;
#endif
	}

	const std::string& GuanDanAvatar::getRefundTributeText() const {
		return _refundTributeText;
	}

	void GuanDanAvatar::setCardOut(const PokerCard& card) {
		_cardOut = card;
	}

	const PokerCard& GuanDanAvatar::getCardOut() const {
		return _cardOut;
	}

	void GuanDanAvatar::setCardIn(const PokerCard& card) {
		_cardIn = card;
	}

	const PokerCard& GuanDanAvatar::getCardIn() const {
		return _cardIn;
	}

	void GuanDanAvatar::clearAnalysis() {
		PokerAvatar::clearAnalysis();

		_variableCards = 0;
		_occupiedVariableCards = 0;
		for (int i = 0; i < 13; i++) {
			for (int j = 0; j < 4; j++)
				_pointOrderSuitGraph[i][j] = 0;
		}
		std::list<GuanDanSearchGroup::Ptr>::iterator it_g;
		for (it_g = _groups.begin(); it_g != _groups.end(); it_g++) {
			GuanDanSearchGroup::Ptr& group = *it_g;
			group->clear();
			freeSearchGroup(group);
		}
		_groups.clear();
		std::list<GuanDanSearch::Ptr>::iterator it_s;
		for (it_s = _searches.begin(); it_s != _searches.end(); it_s++) {
			GuanDanSearch::Ptr& search = *it_s;
			search->clear();
			freeSearch(search);
		}
		_searches.clear();
		_occupiedCardIds.clear();
		_woodOrders.clear();
		_steelOrders.clear();
	}

	bool GuanDanAvatar::analyzeIgnore(const PokerCard& c) {
		int order = _rule->getPointOrder(c.getPoint());
		if (order > 12)
			return false;
		if (c.getSuit() == static_cast<int>(PokerSuit::Heart)) {
			GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
			if (c.getPoint() == rule->getGradePoint()) {
				_variableCards++;
				return true;
			}
		}
		int suit = c.getSuit();
		if (suit < static_cast<int>(PokerSuit::Diamond) || suit > static_cast<int>(PokerSuit::Spade))
			return false;
		suit -= static_cast<int>(PokerSuit::Diamond);
		_pointOrderSuitGraph[order][suit] += 1;
		return false;
	}

	/**
	 * 获取数组的最小值加1后数组的最小值
	 * @param arr 数组
	 * @param size 数组大小
	 * @param idx 返回数组最小值索引(即加1的位置)
	 * @return 返回数组最小值加1后的数组的最小值
	 */
	int minAddOne(int* arr, int size, int& idx) {
		bool test = false;
		int min = 0;
		idx = 0;
		for (int i = 0; i < size; i++) {
			if (arr[i] == 0) {
				arr[i]++;
				idx = i;
				test = true;
				break;
			}
			if (i == 0 || min > arr[i]) {
				min = arr[i];
				idx = i;
			}
		}
		if (!test)
			arr[idx]++;
		for (int i = 0; i < size; i++) {
			if (arr[i] == 0) {
				min = 0;
				break;
			}
			if (i == 0 || min > arr[i])
				min = arr[i];
		}
		return min;
	}

	/**
	 * 计算x倍顺子同花层数
	 * @param arr 5个连续牌值的各种花色数量数组，arr[20]
	 * @param idx1 第一张逢人配匹配的牌值位置索引，-1表示无逢人配
	 * @param idx2 第二张逢人配匹配的牌值位置索引，-1表示无逢人配
	 * 返回同花顺花色
	 */
	int takeSameSuit(int* arr, int& idx1, int& idx2) {
		int idx = 0;
		int num = 0;
		int minNum = 0;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 5; j++) {
				idx = j * 4 + i;
				num = arr[idx];
				if (j == idx1) {
					// 逢人配可以匹配任意花色
					num += 1;
				}
				else if (j == idx2) {
					// 逢人配可以匹配任意花色
					num += 1;
				}
				if (j == 0 || minNum > num)
					minNum = num;
			}
			if (minNum > 0) {
				for (int j = 0; j < 5; j++) {
					idx = j * 4 + i;
					// 优先使用逢人配进行花色匹配，匹配之后将逢人配的牌值位置索引置为-1，表示该逢人配已被消耗，不能再进行下一次的花色匹配
					if (j == idx1)
						idx1 = -1;
					else if (j == idx2)
						idx2 = -1;
					else
						arr[idx] -= 1;
				}
				return i;
			}
		}
		return -1;
	}

	void GuanDanAvatar::combineAllGenres() {
		int order = 12;
		int pos = 0;
		int nums[5] = { 0 };
		int suits[20] = { 0 };
		int tmpSuits[20] = { 0 };
		int min0 = 0;
		int min1 = 0;
		int min2 = 0;
		int idx1 = -1;
		int idx2 = -1;
		int sameSuit = -1;
		int tmpIdx1 = -1;
		int tmpIdx2 = -1;
		int layers = 0;
		GuanDanSearch::Ptr search;
		while (order > 2) {
			min0 = 0;
			min1 = 0;
			min2 = 0;
			idx1 = -1;
			idx2 = -1;
			for (int i = 4; i > -1; i--) {
				if (order == 3 && i == 4)
					pos = 12;
				else
					pos = order - i;
				nums[4 - i] = _pointOrderNums[pos];
				for (int j = 0; j < 4; j++)
					suits[(4 - i) * 4 + j] = _pointOrderSuitGraph[pos][j];
				if ((i == 4) || (min0 > nums[4 - i]))
					min0 = nums[4 - i];
			}
			if (min0 > 3) {
				order--;
				continue;
			}
			if ((_variableCards > 0) && (min0 < 3))
				min1 = minAddOne(nums, 5, idx1);
			if ((_variableCards > 1) && (min1 < 3))
				min2 = minAddOne(nums, 5, idx2);
			if (min0 > 0) {
				// 不带逢人配的顺子
				search = allocateSearch();
				for (int i = 4; i > -1; i--) {
					if (order == 3 && i == 4)
						pos = 12;
					else
						pos = order - i;
					search->addPointOrderNum(pos, min0);
				}
				search->setGenre(static_cast<int>(GuanDanGenre::Straight));
				search->setOfficerPointOrder(pos);
				search->setStraightMultiple(min0);

				tmpIdx1 = -1;
				tmpIdx2 = -1;
#if defined(WIN32)
				memcpy_s(tmpSuits, sizeof(int) * 20, suits, sizeof(int) * 20);
#else
				memcpy(tmpSuits, suits, sizeof(int) * 20);
#endif
				for (int i = 0; i < min0; i++) {
					sameSuit = takeSameSuit(tmpSuits, tmpIdx1, tmpIdx2);
					if (sameSuit == -1)
						break;
					else {
						sameSuit += static_cast<int>(PokerSuit::Diamond);
						search->addSameSuit(sameSuit);
					}
				}
				calcDamages(search);
				insertSearch(search);
			}
			if (min1 > min0) {
				// 带一张逢人配的顺子
				search = allocateSearch();
				for (int i = 4; i > -1; i--) {
					if (order == 3 && i == 4)
						pos = 12;
					else
						pos = order - i;
					if ((4 - i) == idx1)
						search->addPointOrderNum(pos, min1 - 1);
					else
						search->addPointOrderNum(pos, min1);
				}
				search->setGenre(static_cast<int>(GuanDanGenre::Straight));
				search->setOfficerPointOrder(pos);
				search->setStraightMultiple(min1);
				tmpIdx1 = idx1;
				tmpIdx2 = -1;
#if defined(WIN32)
				memcpy_s(tmpSuits, sizeof(int) * 20, suits, sizeof(int) * 20);
#else
				memcpy(tmpSuits, suits, sizeof(int) * 20);
#endif
				for (int i = 0; i < min1; i++) {
					sameSuit = takeSameSuit(tmpSuits, tmpIdx1, tmpIdx2);
					if (sameSuit == -1)
						break;
					else {
						sameSuit += static_cast<int>(PokerSuit::Diamond);
						search->addSameSuit(sameSuit);
					}
				}
				search->setVariableCards(1);
				calcDamages(search);
				insertSearch(search);
			}
			if (min2 > min1) {
				// 带两张逢人配的顺子
				search = allocateSearch();
				for (int i = 4; i > -1; i--) {
					if (order == 3 && i == 4)
						pos = 12;
					else
						pos = order - i;
					layers = min2;
					if ((4 - i) == idx1)
						layers--;
					if ((4 - i) == idx2)
						layers--;
					search->addPointOrderNum(pos, layers);
				}
				search->setGenre(static_cast<int>(GuanDanGenre::Straight));
				search->setOfficerPointOrder(pos);
				search->setStraightMultiple(min2);
				tmpIdx1 = idx1;
				tmpIdx2 = idx2;
#if defined(WIN32)
				memcpy_s(tmpSuits, sizeof(int) * 20, suits, sizeof(int) * 20);
#else
				memcpy(tmpSuits, suits, sizeof(int) * 20);
#endif
				for (int i = 0; i < min2; i++) {
					sameSuit = takeSameSuit(tmpSuits, tmpIdx1, tmpIdx2);
					if (sameSuit == -1)
						break;
					else {
						sameSuit += static_cast<int>(PokerSuit::Diamond);
						search->addSameSuit(sameSuit);
					}
				}
				search->setVariableCards(2);
				calcDamages(search);
				insertSearch(search);
			}
			order--;
		}
		collectStraightFlush();
		bool test = false;
		int variableCards = 0;
		order = 12;
		while (order > -1) {
			min0 = 0;
			min1 = 0;
			min2 = 0;
			idx1 = -1;
			idx2 = -1;
			test = false;
			for (int i = 1; i > -1; i--) {
				if (order == 0 && i == 1)
					pos = 12;
				else
					pos = order - i;
				nums[1 - i] = _pointOrderNums[pos];
				if ((i == 1) || (min0 > nums[1 - i]))
					min0 = nums[1 - i];
			}
			if (min0 > 3) {
				order--;
				continue;
			}
			if (min0 > 2) {
				// 不需要逢人配就能组成钢板
				test = true;
				variableCards = 0;
			}
			else if (_variableCards > 0) {
				min1 = minAddOne(nums, 2, idx1);
				if (min1 > 2) {
					// 需要一张逢人配才能组成钢板
					test = true;
					variableCards = 1;
				}
				else if (_variableCards > 1) {
					min2 = minAddOne(nums, 2, idx2);
					if (min2 > 2) {
						// 需要两张逢人配才能组成钢板
						test = true;
						variableCards = 2;
					}
				}
			}
			if (test) {
				search = allocateSearch();
				for (int i = 1; i > -1; i--) {
					if (order == 0 && i == 1)
						pos = 12;
					else
						pos = order - i;
					layers = 3;
					if ((1 - i) == idx1)
						layers--;
					if ((1 - i) == idx2)
						layers--;
					search->addPointOrderNum(pos, layers);
				}
				search->setGenre(static_cast<int>(GuanDanGenre::Triple2));
				search->setOfficerPointOrder(pos);
				search->setVariableCards(variableCards);
				if (variableCards > 0)
					search->setBad();
				calcDamages(search);
				insertSearch(search);
			}
			order--;
		}
		order = 12;
		while (order > 0) {
			min0 = 0;
			min1 = 0;
			min2 = 0;
			idx1 = -1;
			idx2 = -1;
			test = false;
			for (int i = 2; i > -1; i--) {
				if (order == 1 && i == 2)
					pos = 12;
				else
					pos = order - i;
				nums[2 - i] = _pointOrderNums[pos];
				if ((i == 2) || (min0 > nums[2 - i]))
					min0 = nums[2 - i];
			}
			if (min0 > 3) {
				order--;
				continue;
			}
			if (min0 > 1) {
				// 不需要逢人配就能组成木板
				test = true;
				variableCards = 0;
			}
			else if (_variableCards > 0) {
				min1 = minAddOne(nums, 3, idx1);
				if (min1 > 1) {
					// 需要一张逢人配才能组成木板
					test = true;
					variableCards = 1;
				}
				else if (_variableCards > 1) {
					min2 = minAddOne(nums, 3, idx2);
					if (min2 > 1) {
						// 需要两张逢人配才能组成木板
						test = true;
						variableCards = 2;
					}
				}
			}
			if (test) {
				search = allocateSearch();
				for (int i = 2; i > -1; i--) {
					if (order == 1 && i == 2)
						pos = 12;
					else
						pos = order - i;
					layers = 2;
					if ((2 - i) == idx1)
						layers--;
					if ((2 - i) == idx2)
						layers--;
					search->addPointOrderNum(pos, layers);
				}
				search->setGenre(static_cast<int>(GuanDanGenre::Pair3));
				search->setOfficerPointOrder(pos);
				search->setVariableCards(variableCards);
				calcDamages(search);
				insertSearch(search);
			}
			order--;
		}
		seachThreeWith2();
		mergeSearchGroups();
		makeCombinations();
	}

	void GuanDanAvatar::seachThreeWith2() {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return;
		bool test1 = false;
		bool test2 = false;
		int gradeOrder = rule->getPointOrder(rule->getGradePoint());
		std::vector<int> orders3;
		std::vector<int> orders2;
		std::unordered_set<int>::const_iterator it;
		for (int i = 0; i < 13; i++) {
			if (_pointOrderNums[i] == 3) {
				// 首先把钢板尚未占用的三张加入对了
				it = _steelOrders.find(i);
				if (it != _steelOrders.end())
					continue;
				if (gradeOrder == i)
					test1 = true;
				else
					orders3.push_back(i);
			}
			else if (_pointOrderNums[i] == 2) {
				// 首先将木板牌型尚未占用的对子加入队列
				it = _woodOrders.find(i);
				if (it != _woodOrders.end())
					continue;
				if (gradeOrder == i)
					test2 = true;
				else
					orders2.push_back(i);
			}
			else if (_pointOrderNums[i] == 4) {
				// 其次将木板牌型占用后剩余两张的对子加入队列
				it = _woodOrders.find(i);
				if (it == _woodOrders.end())
					continue;
				if (gradeOrder == i)
					test2 = true;
				else
					orders2.push_back(i);
			}
		}
		if (test1)
			orders3.push_back(gradeOrder);
		if (test2)
			orders2.push_back(gradeOrder);
		for (int i = 0; i < 13; i++) {
			if (_pointOrderNums[i] == 3) {
				// 最后把钢板占用的三张也加入对了
				it = _steelOrders.find(i);
				if (it == _steelOrders.end())
					continue;
				if (gradeOrder == i) {
					// 此时test1必定为false，因此可以直接将test1赋值为true而不用担心重复添加
					test1 = true;
				}
				else
					orders3.push_back(i);
			}
			else if (_pointOrderNums[i] == 2) {
				// 最后把木板占用的对子也加入对子队列
				it = _woodOrders.find(i);
				if (it == _woodOrders.end())
					continue;
				if (gradeOrder == i) {
					// 此时test2必定为false，因此可以直接将test2赋值为true而不用担心重复添加
					test2 = true;
				}
				else
					orders2.push_back(i);
			}
		}
		if (test1)
			orders3.push_back(gradeOrder);
		if (test2)
			orders2.push_back(gradeOrder);
		int order3 = 0;
		int order2 = 0;
		GuanDanSearch::Ptr search;
		std::unordered_set<std::string> tmpSet;
		std::unordered_set<std::string>::const_iterator it_s;
		std::string text;
		// 将相邻牌值的三带二(如33344、66655)加入
		int num = static_cast<int>(orders3.size());
		for (int i = 0; i < num; i++) {
			order3 = orders3.at(i);
			for (int j = 0; j < 2; j++) {
				if (j == 0) {
					if (order3 == 12)
						order2 = 0;
					else
						order2 = order3 + 1;
				}
				else {
					if (order3 == 0)
						order2 = 12;
					else
						order2 = order3 - 1;
				}
				if (_pointOrderNums[order2] != 2)
					continue;
				text = std::to_string(order3) + std::string("_") + std::to_string(order2);
				tmpSet.insert(text);
				search = allocateSearch();
				search->addPointOrderNum(order3, 3);
				search->addPointOrderNum(order2, 2);
				search->setGenre(static_cast<int>(GuanDanGenre::ThreeWith2));
				search->setOfficerPointOrder(order3);
				calcDamages(search);
				insertSearch(search);
			}
		}
		if (num > static_cast<int>(orders2.size()))
			num = static_cast<int>(orders2.size());
		for (int i = 0; i < num; i++) {
			order3 = orders3.at(i);
			order2 = orders2.at(i);
			text = std::to_string(order3) + std::string("_") + std::to_string(order2);
			it_s = tmpSet.find(text);
			if (it_s != tmpSet.end())
				continue;
			search = allocateSearch();
			search->addPointOrderNum(order3, 3);
			search->addPointOrderNum(order2, 2);
			search->setGenre(static_cast<int>(GuanDanGenre::ThreeWith2));
			search->setOfficerPointOrder(order3);
			calcDamages(search);
			insertSearch(search);
		}
	}

	GuanDanSearch::Ptr GuanDanAvatar::allocateSearch() {
		GuanDanSearch::Ptr search;
		if (_freeSearches.empty())
			search = std::make_shared<GuanDanSearch>(_searchId++);
		else {
			search = _freeSearches.front();
			search->clear();
			_freeSearches.pop_front();
		}
		return search;
	}

	void GuanDanAvatar::freeSearch(const GuanDanSearch::Ptr& search) {
		_freeSearches.emplace_back(search);
	}

	GuanDanSearchGroup::Ptr GuanDanAvatar::allocateSearchGroup() {
		GuanDanSearchGroup::Ptr group;
		if (_freeGroups.empty())
			group = std::make_shared<GuanDanSearchGroup>(_groupId++);
		else {
			group = _freeGroups.front();
			group->clear();
			_freeGroups.pop_front();
		}
		return group;
	}

	void GuanDanAvatar::freeSearchGroup(const GuanDanSearchGroup::Ptr& group) {
		_freeGroups.emplace_back(group);
	}

	void GuanDanAvatar::calcDamages(const std::unordered_map<int, int>& graph, int& damages, int& undamages) const {
		int total = 0;
		int left = 0;
		int size = static_cast<int>(graph.size());
		int cnt = 0;
		damages = 0;
		undamages = 0;
		std::unordered_map<int, int>::const_iterator it;
		for (it = graph.begin(); it != graph.end(); it++) {
			total = _pointOrderNums[it->first];
			if (total == 0)
				continue;
			if (it->second >= total) {
				undamages += total;
				cnt++;
			}
			else {
				left = total - it->second;
				if (left > 3) {
					if (size == 2)
						damages += 4;	// 钢板
					else if (size == 3)
						damages += 3;	// 木板
					else
						damages += 2;	// 顺子
				}
				else if (total > 3) {
					if (size == 2)
						damages += 5;	// 钢板
					else if (size == 3)
						damages += 4;	// 木板
					else
						damages += 3;	// 顺子
				}
				else {
					if (size == 3)
						damages += 2;	// 木板，如：2233444
					else
						damages += 1;	// 顺子
				}
			}
		}
		if ((size == 5) && (cnt > 3) && (undamages < 6))
			undamages = static_cast<int>(1.5f * undamages);	// 单倍顺子放大无损系数
	}

	void GuanDanAvatar::calcDamages(const std::unordered_map<int, int>& graph1, const std::unordered_map<int, int>& graph2, int& damages, int& undamages) const {
		int total = 0;
		int left = 0;
		int size = static_cast<int>(graph1.size());
		int num = 0;
		int valU = 0;
		int cnt = 0;
		std::unordered_map<int, int>::const_iterator it1;
		std::unordered_map<int, int>::const_iterator it2;
		for (it1 = graph1.begin(); it1 != graph1.end(); it1++) {
			total = _pointOrderNums[it1->first];
			if (total == 0)
				continue;
			it2 = graph2.find(it1->first);
			if (it2 != graph2.end())
				num = it2->second;
			else {
				// 正常不会执行到这里，注意调试
				num = 0;
				LOG_ERROR("error");
			}
			if (num >= total) {
				if (total > 3) {
					// 破坏炸弹
					if (size == 2)
						damages += 3;	// 钢板
					else if (size == 2)
						damages += 2;	// 木板
					else
						damages += 1;	// 顺子
				}
				else {
					valU += (it1->second);
					if (it1->second >= total)
						cnt++;
				}
			}
			else {
				left = total - num;
				if (left > 3) {
					if (size == 2)
						damages += 4;	// 钢板
					else if (size == 3)
						damages += 3;	// 木板
					else
						damages += 2;	// 顺子
				}
				else if (total > 3) {
					if (size == 2)
						damages += 5;	// 钢板
					else if (size == 3)
						damages += 4;	// 木板
					else
						damages += 3;	// 顺子
				}
				else {
					if (size == 3)
						damages += 2;	// 木板，如：2233444
					else
						damages += 1;	// 顺子
				}
			}
		}
		if ((size == 5) && (cnt > 3) && (valU < 6))
			valU = static_cast<int>(1.5f * valU);	// 单倍顺子放大无损系数
		undamages += valU;
	}

	void GuanDanAvatar::calcDamages(const GuanDanSearch::Ptr& search) const {
		int damages = 0;
		int undamages = 0;
		calcDamages(search->getPointOrderNums(), damages, undamages);
		int genre = search->getGenre();
		if (genre == static_cast<int>(GuanDanGenre::Straight))
			undamages += search->getSameSuitNum() * 2;
		search->setDamages(damages);
		search->setUndamages(undamages);
	}

	void GuanDanAvatar::calcBombDamages(const std::unordered_map<int, int>& graph, int& damages) const {
		int total = 0;
		std::unordered_map<int, int>::const_iterator it;
		for (it = graph.begin(); it != graph.end(); it++) {
			total = _pointOrderNums[it->first];
			if ((total < 4) || (it->second < total))
				continue;
			damages += 3;
		}
	}

	void GuanDanAvatar::insertSearch(const GuanDanSearch::Ptr& search) {
		if ((search->getNet() >= 0) && !search->isBad()) {
			if (search->getGenre() == static_cast<int>(GuanDanGenre::Pair3)) {
				bool test = true;
				std::unordered_set<int>::const_iterator it_t;
				const std::unordered_map<int, int>& graph = search->getPointOrderNums();
				std::unordered_map<int, int>::const_iterator it_g;
				for (it_g = graph.begin(); it_g != graph.end(); it_g++) {
					it_t = _steelOrders.find(it_g->first);
					if (it_t != _steelOrders.end()) {
						// 该牌值已被钢板占用，最终传入的木板检索牌型必定会被淘汰
						test = false;
						break;
					}
				}
				if (test) {
					for (it_g = graph.begin(); it_g != graph.end(); it_g++)
						_woodOrders.insert(it_g->first);
				}
			}
			else if (search->getGenre() == static_cast<int>(GuanDanGenre::Triple2)) {
				const std::unordered_map<int, int>& graph = search->getPointOrderNums();
				std::unordered_map<int, int>::const_iterator it_g = graph.begin();
				for (; it_g != graph.end(); it_g++)
					_steelOrders.insert(it_g->first);
			}
		}
		mergeSearch(search);

		bool test = false;
		std::list<GuanDanSearch::Ptr>::const_iterator it = _searches.begin();
		while (it != _searches.end()) {
			const GuanDanSearch::Ptr& tmp = *it;
			if (tmp->getNet() < search->getNet()) {
				_searches.insert(it, search);
				test = true;
				break;
			}
			++it;
		}
		if (!test)
			_searches.push_back(search);
	}

	void GuanDanAvatar::insertGroup(const GuanDanSearchGroup::Ptr& group) {
		bool test = false;
		std::list<GuanDanSearchGroup::Ptr>::const_iterator it = _groups.begin();
		while (it != _groups.end()) {
			const GuanDanSearchGroup::Ptr& tmp = *it;
			if (tmp->getNet() < group->getNet()) {
				_groups.insert(it, group);
				test = true;
				break;
			}
			++it;
		}
		if (!test)
			_groups.push_back(group);
	}

	void GuanDanAvatar::mergeSearch(const GuanDanSearch::Ptr& search) {
		if (search->isBad())
			return;
		std::vector<GuanDanSearchGroup::Ptr> newGroups;
		std::list<GuanDanSearchGroup::Ptr>::const_iterator it_g = _groups.begin();
		while (it_g != _groups.end()) {
			if (mergeSearch(*it_g, search)) {
				newGroups.emplace_back(*it_g);
				it_g = _groups.erase(it_g);
			}
			else
				++it_g;
		}
		GuanDanSearchGroup::Ptr group;
		std::list<GuanDanSearch::Ptr>::const_iterator it_s;
		for (it_s = _searches.begin(); it_s != _searches.end(); it_s++) {
			if ((*it_s)->isBad())
				continue;
			group = mergeSearch(*it_s, search);
			if (group)
				newGroups.emplace_back(group);
		}
		for (const GuanDanSearchGroup::Ptr& group : newGroups)
			insertGroup(group);
	}

	/**
	 * 合并牌值张数曲线图
	 * @param graph1 曲线图1
	 * @param graph2 曲线图2
	 * @param graph3 合并后的曲线图
	 */
	void mergeGraph(const std::unordered_map<int, int>& graph1, const std::unordered_map<int, int>& graph2, std::unordered_map<int, int>& graph3) {
		graph3 = graph1;
		std::unordered_map<int, int>::iterator it;
		std::unordered_map<int, int>::const_iterator it_c = graph2.begin();
		while (it_c != graph2.end()) {
			it = graph3.find(it_c->first);
			if (it == graph3.end())
				graph3.insert(std::pair<int, int>(it_c->first, it_c->second));
			else
				it->second += it_c->second;
			++it_c;
		}
	}

	bool GuanDanAvatar::mergeSearch(const GuanDanSearchGroup::Ptr& group, const GuanDanSearch::Ptr& search) {
		if (group->containsSearch(search->getId()))
			return false;	// 已并入不能再次并入
		if (conflictTest(search.get(), group.get()))
			return false;	// 存在冲突不能并入
		// 计算并入后的组合净值
		std::unordered_map<int, int> graph;
		mergeGraph(group->getPointOrderNums(), search->getPointOrderNums(), graph);
		int damages = 0;
		int undamages = 0;
		calcDamages(search->getPointOrderNums(), graph, damages, undamages);
		const std::unordered_map<int, GuanDanSearch::Ptr>& searchs = group->getSearches();
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it = searchs.begin();
		while (it != searchs.end()) {
			calcDamages((it->second)->getPointOrderNums(), graph, damages, undamages);
			straightUndamages(it->second, graph, undamages);
			++it;
		}
		straightUndamages(search, graph, undamages);
		calcBombDamages(graph, damages);
		int net = undamages - damages;
		// 若并入后净值减小，则不能并入
		if (net < group->getNet())
			return false;
		int variableCards = group->getVariableCards() + search->getVariableCards();
		group->setDamages(damages);
		group->setUndamages(undamages);
		group->setVariableCards(variableCards);
		group->setPointOrderNums(graph);
		group->addSearch(search);
		return true;
	}

	GuanDanSearchGroup::Ptr GuanDanAvatar::mergeSearch(const GuanDanSearch::Ptr& search1, const GuanDanSearch::Ptr& search2) {
		if (search1->getId() == search2->getId())
			return GuanDanSearchGroup::Ptr();
		if (conflictTest(search1.get(), search2.get()))
			return GuanDanSearchGroup::Ptr();	// 存在冲突不能并入
		std::unordered_map<int, int> graph;
		mergeGraph(search1->getPointOrderNums(), search2->getPointOrderNums(), graph);
		int damages = 0;
		int undamages = 0;
		calcDamages(search1->getPointOrderNums(), graph, damages, undamages);
		calcDamages(search2->getPointOrderNums(), graph, damages, undamages);
		straightUndamages(search1, graph, undamages);
		straightUndamages(search2, graph, undamages);
		calcBombDamages(graph, damages);
		GuanDanSearchGroup::Ptr group = allocateSearchGroup();
		int variableCards = search1->getVariableCards() + search2->getVariableCards();
		group->setDamages(damages);
		group->setUndamages(undamages);
		group->setVariableCards(variableCards);
		group->setPointOrderNums(graph);
		group->addSearch(search1);
		group->addSearch(search2);
		return group;
	}

	void GuanDanAvatar::straightUndamages(const GuanDanSearch::Ptr& search, const std::unordered_map<int, int>& graph, int& undamages) const {
		if (search->getGenre() != static_cast<int>(GuanDanGenre::Straight))
			return;
		/*int multiple = search->getStraightMultiple();
		if (multiple > 1) {
			int temp = 0;
			int num = 0;
			int total = 0;
			const std::unordered_map<int, int>& graph1 = search->getPointOrderNums();
			std::unordered_map<int, int>::const_iterator it1;
			std::unordered_map<int, int>::const_iterator it;
			for (it1 = graph1.begin(); it1 != graph1.end(); ++it1) {
				total = _pointOrderNums[it1->first];
				if (total == 0)
					continue;
				it = graph.find(it1->first);
				if (it != graph.end())
					num = it->second;
				else
					num = 0;	// 正常情况下不会执行到这里，注意调试
				if (num >= total)
					temp++;
			}
			temp *= (multiple - 1);
			undamages += temp;
		}*/
		undamages += search->getSameSuitNum() * 2;
	}

	void makePairId(int id1, int id2, std::string& id) {
		std::stringstream ss;
		ss << id1 << '_' << id2;
		id = ss.str();
	}

	void GuanDanAvatar::mergeSearchGroups() {
		GuanDanSearchGroup::Ptr group1;
		GuanDanSearchGroup::Ptr group2;
		GuanDanSearchGroup::Ptr group3;
		std::string id;
		std::vector<GuanDanSearchGroup::Ptr> newGroups;
		std::vector<GuanDanSearchGroup::Ptr> removeGroups;
		std::unordered_set<std::string> pairIds;
		std::unordered_set<int> removeIds;
		std::list<GuanDanSearchGroup::Ptr>::const_iterator it1;
		std::list<GuanDanSearchGroup::Ptr>::const_iterator it2;
		std::unordered_set<int>::const_iterator it_s;
		std::unordered_set<std::string>::const_iterator it_p;
		while (true) {
			it1 = _groups.begin();
			while (it1 != _groups.end()) {
				group1 = *it1;
				it2 = it1;
				it2++;
				while (it2 != _groups.end()) {
					group2 = *it2;
					makePairId(group1->getId(), group2->getId(), id);
					it_p = pairIds.find(id);
					if (it_p == pairIds.end()) {
						group3 = mergeSearchGroup(group1, group2);
						if (group3) {
							removeIds.insert(group1->getId());
							removeIds.insert(group2->getId());
							newGroups.emplace_back(group3);
						}
						else
							pairIds.insert(id);
					}
					it2++;
				}
				++it1;
			}
			if (newGroups.empty())
				break;	// 不再产生新的检索牌型组合，退出循环
			it1 = _groups.begin();
			while (it1 != _groups.end()) {
				const GuanDanSearchGroup::Ptr& group = *it1;
				it_s = removeIds.find(group->getId());
				if (it_s != removeIds.end()) {
					// 注意这里不能释放检索组合，因为如果这里释放了组合，在合并新组合时之前循环删除过的组合id可能会被重复使用
					removeGroups.push_back(group);
					it1 = _groups.erase(it1);
				}
				else
					++it1;
			}
			removeIds.clear();
			for (const GuanDanSearchGroup::Ptr& group : newGroups)
				insertGroup(group);
			newGroups.clear();
		}
		for (const GuanDanSearchGroup::Ptr& group : removeGroups)
			freeSearchGroup(group);
	}

	GuanDanSearchGroup::Ptr GuanDanAvatar::mergeSearchGroup(const GuanDanSearchGroup::Ptr& group1, const GuanDanSearchGroup::Ptr& group2) {
		if (group1->getId() == group2->getId())
			return GuanDanSearchGroup::Ptr();
		if (conflictTest(group1, group2))
			return GuanDanSearchGroup::Ptr();	// 存在冲突不能并入
		std::unordered_map<int, int> graph;
		mergeGraph(group1->getPointOrderNums(), group2->getPointOrderNums(), graph);
		int damages = 0;
		int undamages = 0;
		const std::unordered_map<int, GuanDanSearch::Ptr>& searchs1 = group1->getSearches();
		const std::unordered_map<int, GuanDanSearch::Ptr>& searchs2 = group2->getSearches();
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it = searchs1.begin();
		while (it != searchs1.end()) {
			calcDamages((it->second)->getPointOrderNums(), graph, damages, undamages);
			straightUndamages(it->second, graph, undamages);
			++it;
		}
		it = searchs2.begin();
		while (it != searchs2.end()) {
			calcDamages((it->second)->getPointOrderNums(), graph, damages, undamages);
			straightUndamages(it->second, graph, undamages);
			++it;
		}
		calcBombDamages(graph, damages);
		int net = undamages - damages;
		// 若合并后净值减小，则不能合并
		if ((net < group1->getNet()) || (net < group2->getNet()))
			return GuanDanSearchGroup::Ptr();
		GuanDanSearchGroup::Ptr group = allocateSearchGroup();
		int variableCards = group1->getVariableCards() + group2->getVariableCards();
		group->setDamages(damages);
		group->setUndamages(undamages);
		group->setVariableCards(variableCards);
		group->setPointOrderNums(graph);
		it = searchs1.begin();
		while (it != searchs1.end()) {
			group->addSearch(it->second);
			++it;
		}
		it = searchs2.begin();
		while (it != searchs2.end()) {
			group->addSearch(it->second);
			++it;
		}
		return group;
	}

	bool GuanDanAvatar::conflictTest(const GuanDanSearchData* data1, const GuanDanSearchData* data2) const {
		int variableCards = data1->getVariableCards();
		variableCards += data2->getVariableCards();
		// 逢人配张数冲突
		if (variableCards > _variableCards)
			return true;
		int num = 0;
		const std::unordered_map<int, int>& graph = data1->getPointOrderNums();
		std::unordered_map<int, int>::const_iterator it = graph.begin();
		while (it != graph.end()) {
			num = it->second;
			num += data2->getPointOrderNum(it->first);
			if (num > _pointOrderNums[it->first])
				return true;
			++it;
		}
		return false;
	}

	bool GuanDanAvatar::conflictTest(const GuanDanSearchGroup::Ptr& group1, const GuanDanSearchGroup::Ptr& group2) const {
		const std::unordered_map<int, GuanDanSearch::Ptr>& searchs = group1->getSearches();
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it = searchs.begin();
		while (it != searchs.end()) {
			if (group2->containsSearch(it->first))
				return true;	// 两个组合包含同一个检索牌型，返回冲突
			++it;
		}
		return conflictTest(group1.get(), group2.get());
	}

	// 插入牌值顺序到列表中，按牌值(考虑级牌大小)大小顺序从大到小排序
	void insertPointOrder(const PokerRule::Ptr& rule, std::list<int>& tmpList, int order) {
		int ret = 0;
		int point1 = rule->getPointByOrder(order);
		int point2 = 0;
		bool test = false;
		std::list<int>::const_iterator it_l = tmpList.begin();
		while (it_l != tmpList.end()) {
			point2 = rule->getPointByOrder(*it_l);
			ret = rule->comparePoint(point1, point2);
			if (ret == 1) {
				tmpList.insert(it_l, order);
				test = true;
				break;
			}
			++it_l;
		}
		if (!test)
			tmpList.push_back(order);
	}

	bool occupyVariableCard(const PokerRule::Ptr& rule, std::unordered_map<int, std::list<int> >& table, int num, int vc, int& pointOrder1, int& pointOrder2) {
		std::unordered_map<int, std::list<int> >::iterator it_t = table.find(num);
		if (it_t == table.end())
			return false;
		std::list<int>& tmpList = it_t->second;
		std::list<int>::const_iterator it_l = tmpList.begin();
		int order = *it_l;
		tmpList.erase(it_l);
		if (tmpList.empty())
			table.erase(it_t);
		it_t = table.find(num + 1);
		if (it_t == table.end()) {
			table.insert(std::make_pair(num + 1, std::list<int>()));
			it_t = table.find(num + 1);
		}
		insertPointOrder(rule, it_t->second, order);
		if (vc == 0)
			pointOrder1 = order;
		else
			pointOrder2 = order;
		return true;
	}

	bool GuanDanAvatar::makeCombinations() {
		std::list<GuanDanSearch::Ptr> searches;
		GuanDanSearchData* data = nullptr;
		bool test = false;
		if (!_searches.empty()) {
			const GuanDanSearch::Ptr& search = _searches.front();
			int netS = search->getNet();
			if (_groups.empty()) {
				if (netS > 0)
					test = true;
			}
			else {
				const GuanDanSearchGroup::Ptr& group = _groups.front();
				int netG = group->getNet();
				if ((netG > netS) && (netG > 0)) {
					group->removeThreeWith2();
					group->setAdopt();
					data = group.get();
					bool flag = false;
					std::list<GuanDanSearch::Ptr>::const_iterator it_l;
					const std::unordered_map<int, GuanDanSearch::Ptr>& tmpMap = group->getSearches();
					std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it;
					for (it = tmpMap.begin(); it != tmpMap.end(); it++) {
						flag = false;
						for (it_l = searches.begin(); it_l != searches.end(); it_l++) {
							if ((it->second)->getOfficerPointOrder() > (*it_l)->getOfficerPointOrder()) {
								searches.insert(it_l, it->second);
								flag = true;
								break;
							}
						}
						if (!flag)
							searches.push_back(it->second);
					}
				}
				else if (netS > 0)
					test = true;
			}
			if (test && (search->getGenre() != static_cast<int>(GuanDanGenre::ThreeWith2))) {
				search->setAdopt();
				data = search.get();
				searches.push_back(search);
			}
		}
		if (data != nullptr) {
			if (!makeCombinations(data->getPointOrderNums(), searches))
				return false;
		}
		// 第一张逢人配匹配的牌值顺序
		int pointOrder1 = -1;
		// 第二张逢人配匹配的牌值顺序
		int pointOrder2 = -1;
		std::unordered_map<int, std::list<int> > table;
		makeOtherTable(data, table, pointOrder1, pointOrder2);
		makeCombinations(table, pointOrder1, pointOrder2);
		return true;
	}

	bool GuanDanAvatar::makeCombinations(const std::unordered_map<int, int>& graph, const std::list<GuanDanSearch::Ptr>& searches) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return false;
		int suit = -1;
		int nums = 0;
		int damages = 0;
		int undamages = 0;
		int genre = 0;
		PokerCombination::Ptr comb;
		// 先构建同花顺的出牌组合(因为同花顺指定花色，而普通顺子则无需考虑花色，这样做可以避免普通顺子抽掉了同花顺需要的牌)
		std::list<GuanDanSearch::Ptr>::const_iterator it;
		for (it = searches.begin(); it != searches.end(); ++it) {
			const GuanDanSearch::Ptr& search = *it;
			if (search->getGenre() != static_cast<int>(GuanDanGenre::Straight))
				continue;
			nums = search->getSameSuitNum();
			if (nums == 0)
				continue;
			const std::unordered_map<int, int>& graph_s = search->getPointOrderNums();
			calcDamages(graph_s, graph, damages, undamages);
			for (int i = (nums - 1); i > -1; i--) {
				comb = allocateCombination();
				comb->setGenre(static_cast<int>(GuanDanGenre::StraightFlush));
				comb->setDamages(damages);
				comb->setUndamages(undamages + 2);
				suit = search->getSameSuit(i);
				if (!getCombinationCards(rule, comb, graph_s, 1, search->getStraightMultiple(), search->getVariableCards(), search->getOfficerPointOrder(), suit)) {
					// 同花顺获取牌失败，说明这层同花顺的牌被更大的相邻同花顺抢占，将这层同花顺回退为普通顺子
					freeCombination(comb);
					search->rollbackSameSuit(i);
					DebugS << "Rollback same suit, search id: " << search->getId() << ", layer: " << i << ", suit: " << suit;
				}
				else
					insertCombination(comb, true);
			}
		}
		// 再构建顺子出牌组合
		for (it = searches.begin(); it != searches.end(); it++) {
			const GuanDanSearch::Ptr& search = *it;
			if (search->getGenre() != static_cast<int>(GuanDanGenre::Straight))
				continue;
			nums = search->getStraightMultiple() - search->getSameSuitNum();
			if (nums < 1)
				continue;
			nums = search->getSameSuitNum();
			const std::unordered_map<int, int>& graph_s = search->getPointOrderNums();
			calcDamages(graph_s, graph, damages, undamages);
			for (int i = nums; i < search->getStraightMultiple(); i++) {
				comb = allocateCombination();
				comb->setGenre(static_cast<int>(GuanDanGenre::Straight));
				comb->setDamages(damages);
				comb->setUndamages(undamages);
				if (!getCombinationCards(rule, comb, graph_s, 1, search->getStraightMultiple(), search->getVariableCards(), search->getOfficerPointOrder(), -1)) {
					freeCombination(comb);
					return false;
				}
				insertCombination(comb, true);
			}
		}
		// 构建木板和钢板牌型的出牌组合
		for (it = searches.begin(); it != searches.end(); it++) {
			const GuanDanSearch::Ptr& search = *it;
			genre = search->getGenre();
			if (genre == static_cast<int>(GuanDanGenre::Straight))
				continue;
			else if (genre == static_cast<int>(GuanDanGenre::Pair3))
				nums = 2;
			else if (genre == static_cast<int>(GuanDanGenre::Triple2))
				nums = 3;
			else
				continue;
			comb = allocateCombination();
			const std::unordered_map<int, int>& graph_s = search->getPointOrderNums();
			calcDamages(graph_s, graph, damages, undamages);
			comb->setGenre(search->getGenre());
			comb->setDamages(damages);
			comb->setUndamages(undamages);
			if (!getCombinationCards(rule, comb, graph_s, nums, nums, search->getVariableCards(), search->getOfficerPointOrder(), -1)) {
				freeCombination(comb);
				return false;
			}
			insertCombination(comb, true);
		}
		// 构建三带二牌型的出牌组合
		// 后记：这里构建三带二的代码其实并不会被执行，因为前面已经把三带二牌型从牌型组合中删除掉了，
		// 三带二牌型的构建留在构建其他牌型的代码中构建，所以这里构建三带二的代码实际上可以删掉，只
		// 是先暂时保留
		/*int point = 0;
		int cardId = 0;
		std::unordered_map<int, int>::const_iterator it_g;
		for (it = searches.begin(); it != searches.end(); it++) {
			const GuanDanSearch::Ptr& search = *it;
			genre = search->getGenre();
			if (genre != static_cast<int>(GuanDanGenre::ThreeWith2))
				continue;
			comb = allocateCombination();
			const std::unordered_map<int, int>& graph_s = search->getPointOrderNums();
			calcDamages(graph_s, graph, damages, undamages);
			comb->setGenre(search->getGenre());
			comb->setDamages(damages);
			comb->setUndamages(undamages);
			for (it_g = graph_s.begin(); it_g != graph_s.end(); it_g++) {
				if ((it_g->second) == 3) {
					// 设置主牌牌值
					point = rule->getPointByOrder(it_g->first);
					comb->setOfficerPoint(point);
				}
				for (int i = 0; i < (it_g->second); i++) {
					cardId = getCardId(_occupiedCardIds, it_g->first);
					if (cardId == -1) {
						std::string str;
						PokerUtilities::cardArray2String(_cards, str);
						ErrorS << "为出牌组合(牌型: " << genre << ")分配牌失败，找不到(" << point << ", -1)的牌，当前手牌：" << str;
						freeCombination(comb);
						return false;
					}
					else {
						comb->addCard(cardId);
						_occupiedCardIds.insert(cardId);
					}
				}
			}
			insertCombination(comb, false);
		}*/
		return true;
	}

	bool GuanDanAvatar::getCombinationCards(GuanDanRule* rule, const PokerCombination::Ptr& comb, const std::unordered_map<int, int>& graph, int nums, int layers, int variableCards, int officerPointOrder, int suit) {
		int point = -1;
		int cardId = -1;
		int cnt = 0;
		bool test = false;
		PokerCard c;
		std::vector<int> tmpVec;
		std::unordered_map<int, int>::const_iterator it_g = graph.begin();
		while (it_g != graph.end()) {
			for (int i = 0; i < nums; i++) {
				test = false;
				point = rule->getPointByOrder(it_g->first);
				if (suit == -1)
					cardId = getCardId(_occupiedCardIds, it_g->first);
				else
					cardId = PokerUtilities::getCardId(_cards, _occupiedCardIds, point, suit);
				if (cardId != -1) {
					if ((point == rule->getGradePoint()) && getCardById(cardId, c)) {
						if (c.getSuit() == static_cast<int>(PokerSuit::Heart)) {
							test = true;
							_occupiedVariableCards++;
							cnt++;
						}
					}
				}
				else if ((_occupiedVariableCards < _variableCards) && (cnt < variableCards) && ((it_g->second) < layers)) {
					cardId = PokerUtilities::getCardId(_cards, _occupiedCardIds, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
					if (cardId != -1) {
						test = true;
						_occupiedVariableCards++;
						cnt++;
					}
				}
				if (cardId != -1) {
					_occupiedCardIds.insert(cardId);
					if (suit != -1)
						tmpVec.push_back(cardId);
					comb->addCard(cardId);
					if (it_g->first == officerPointOrder) {
						comb->setOfficerPoint(point);
						if (suit == -1) {
							// 对于非同花顺，使用逢人配匹配的牌，花色固定为方块
							if (test)
								comb->setOfficerSuit(static_cast<int>(PokerSuit::Diamond));
							else if (getCardById(cardId, c))
								comb->setOfficerSuit(c.getSuit());
							else {
								std::string str;
								PokerUtilities::cardArray2String(_cards, str);
								ErrorS << "找不到ID为" << cardId << "的牌，当前手牌：" << str;
								return false;
							}
						}
						else
							comb->setOfficerSuit(suit);
					}
				}
				else {
					// 同花顺不需要打印错误日志，因为同花顺的牌有可能被相邻同花顺抢占了，这种情况下同花顺将回退为普通顺子
					if (suit == -1) {
						std::string str;
						PokerUtilities::cardArray2String(_cards, str);
						ErrorS << "为出牌组合(牌型: " << comb->getGenre() << ")分配牌失败，找不到(" << point << ", " << suit << ")的牌，当前手牌：" << str;
					}
					else {
						// 归还占用的牌
						for (const int& tid : tmpVec)
							_occupiedCardIds.erase(tid);
						// 归还占用的逢人配数量
						_occupiedVariableCards -= cnt;
					}
					return false;
				}
			}
			it_g++;
		}
		return true;
	}

	bool GuanDanAvatar::getCombinationCards1(GuanDanRule* rule, const PokerCombination::Ptr& comb, const std::unordered_map<int, int>& graph, int nums, int officerPointOrder, int suit) {
		int point = -1;
		int cardId = -1;
		int variableCards = 0;
		bool test = false;
		PokerCard c;
		std::unordered_map<int, int>::const_iterator it_g = graph.begin();
		while (it_g != graph.end()) {
			for (int i = 0; i < nums; i++) {
				test = false;
				point = rule->getPointByOrder(it_g->first);
				if (suit == -1)
					cardId = getCardId(comb->getCards(), it_g->first);
				else
					cardId = PokerUtilities::getCardId(_cards, comb->getCards(), point, suit);
				if ((cardId == -1) && (variableCards < _variableCards)) {
					cardId = PokerUtilities::getCardId(_cards, comb->getCards(), rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
					if (cardId != -1) {
						test = true;
						variableCards++;
					}
				}
				if (cardId != -1) {
					comb->addCard(cardId);
					if (it_g->first == officerPointOrder) {
						comb->setOfficerPoint(point);
						if (suit == -1) {
							// 对于非同花顺，使用逢人配匹配的牌，花色固定为方块
							if (test)
								comb->setOfficerSuit(static_cast<int>(PokerSuit::Diamond));
							else if (getCardById(cardId, c))
								comb->setOfficerSuit(c.getSuit());
							else {
								std::string str;
								PokerUtilities::cardArray2String(_cards, str);
								ErrorS << "找不到ID为" << cardId << "的牌，当前手牌：" << str;
								return false;
							}
						}
						else
							comb->setOfficerSuit(suit);
					}
				}
				else {
					std::string str;
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "为出牌组合(牌型: " << comb->getGenre() << ")分配牌失败，找不到(" << point << ", " << suit << ")的牌，当前手牌：" << str;
					return false;
				}
			}
			it_g++;
		}
		return true;
	}

	bool GuanDanAvatar::makeOtherTable(GuanDanSearchData* data, std::unordered_map<int, std::list<int> >& table, int& pointOrder1, int& pointOrder2) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		int num = 0;
		int types = 0;
		std::unordered_map<int, std::list<int> >::iterator it_t;
		std::list<int>::const_iterator it_l;
		// 注意，这里先暂时忽略大小王，因为逢人配不能与大小王作配
		for (int i = 0; i < 13; i++) {
			num = _pointOrderNums[i];
			if (data != nullptr)
				num -= data->getPointOrderNum(i);
			if (num == 0)
				continue;
			types++;
			it_t = table.find(num);
			if (it_t == table.end()) {
				table.insert(std::make_pair(num, std::list<int>()));
				it_t = table.find(num);
			}
			insertPointOrder(_rule, it_t->second, i);
		}
		int tmpPointOrder = 0;
		int variableCards = _variableCards - _occupiedVariableCards;
		if (variableCards > 0) {
			// 当前还有逢人配未被占用
			if (types == 0) {
				// 手中仅剩下逢人配，组成级牌单张或者对子
				num = variableCards;
				pointOrder1 = rule->getPointOrder(rule->getGradePoint());
				if (variableCards > 1)
					pointOrder2 = pointOrder1;
				std::list<int> tmpList;
				tmpList.push_back(pointOrder1);
				table.insert(std::make_pair(num, tmpList));
			}
			else if (types == 1) {
				// 除逢人配外仅剩一种牌值，逢人配全部匹配该牌值
				it_t = table.begin();
				num = it_t->first;
				std::list<int> tmpList = it_t->second;
				pointOrder1 = tmpList.front();
				if (variableCards > 1)
					pointOrder2 = tmpList.front();
				num += variableCards;
				table.erase(it_t);
				table.insert(std::make_pair(num, tmpList));
			}
			else {
				bool test = false;
				for (int i = 0; i < variableCards; i++) {
					// 匹配三张并组成炸弹
					if (occupyVariableCard(_rule, table, 3, i, pointOrder1, pointOrder2))
						continue;
					// 若对子的数量大于2，则匹配最大的对子并组成三张
					it_t = table.find(2);
					if (it_t != table.end()) {
						std::list<int>& tmpList = it_t->second;
						if (tmpList.size() > 1) {
							occupyVariableCard(_rule, table, 2, i, pointOrder1, pointOrder2);
							continue;
						}
					}
					// 与炸弹匹配组成张数加一的炸弹
					test = false;
					for (int j = 4; j < 9; j++) {
						if (occupyVariableCard(_rule, table, j, i, pointOrder1, pointOrder2)) {
							test = true;
							break;
						}
					}
					if (test)
						continue;
					// 逢人配未能匹配任何牌值，执行到这里，说明既没有三张也没有炸弹，只有最多一个对子和单张
					// 匹配对子并组成三张
					if (occupyVariableCard(_rule, table, 2, i, pointOrder1, pointOrder2))
						continue;
					// 逢人配未能匹配任何牌值，执行到这里，说明既没有三张也没有炸弹和对子，只有单张
					it_t = table.find(1);
					if (it_t == table.end()) {
						// 正常不会执行到这里，注意调试
						std::string str;
						PokerUtilities::cardArray2String(_cards, str);
						ErrorS << "组牌出错，当前手牌：" << str;
						return false;
					}
					tmpPointOrder = rule->getPointOrder(rule->getGradePoint());
					std::list<int>& tmpList = it_t->second;
					if (tmpPointOrder == tmpList.front()) {
						// 最大的单张为级牌，与级牌匹配组成对子
						occupyVariableCard(_rule, table, 1, i, pointOrder1, pointOrder2);
						continue;
					}
					// 最大的单张不为级牌，插入单张逢人配组成单张级牌
					tmpList.push_front(tmpPointOrder);
					if (i == 0)
						pointOrder1 = tmpPointOrder;
					else
						pointOrder2 = tmpPointOrder;
				}
			}
		}
		// 添加大小王
		num = _pointOrderNums[13];
		if (num > 0) {
			it_t = table.find(num);
			if (it_t == table.end()) {
				table.insert(std::make_pair(num, std::list<int>()));
				it_t = table.find(num);
			}
			(it_t->second).push_front(13);
		}
		return true;
	}

	bool GuanDanAvatar::makeCombinations(std::unordered_map<int, std::list<int> >& table, int pointOrder1, int pointOrder2) {
		// 最小对子牌值顺序
		int order2 = -1;
		int point = 0;
		int undamages = 0;
		PokerCard c;
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		PokerCombination::Ptr comb;
		std::unordered_map<int, std::list<int> >::iterator it2;
		std::unordered_map<int, std::list<int> >::iterator it_t;
		std::list<int>::const_iterator it_l;
		std::vector<int> cardIds;
		for (int num = 10; num > 0; num--) {
			it_t = table.find(num);
			if (it_t == table.end())
				continue;
			std::list<int>& tmpList = it_t->second;
			if (num == 3)
				tmpList.reverse();
			for (it_l = tmpList.begin(); it_l != tmpList.end(); ++it_l) {
				cardIds.clear();
				undamages = num;
				comb = allocateCombination();
				switch (num) {
				case 1:
					// 构建单张
					comb->setGenre(static_cast<int>(GuanDanGenre::Single));
					break;
				case 2:
					// 构建对子
					comb->setGenre(static_cast<int>(GuanDanGenre::Pair1));
					break;
				case 3:
					it2 = table.find(2);
					if (it2 != table.end()) {
						// 可以组成三带二
						order2 = (it2->second).back();
						(it2->second).pop_back();
						if ((it2->second).empty())
							table.erase(it2);
						// 构建三带二
						undamages = 5;
						comb->setGenre(static_cast<int>(GuanDanGenre::ThreeWith2));
						if (!getCombinationCards(rule, cardIds, order2, 2, pointOrder1, pointOrder2, true)) {
							std::string str;
							PokerUtilities::cardArray2String(_cards, str);
							ErrorS << "三带二获取对子失败，当前手牌：" << str;
							return false;
						}
					}
					else {
						// 构建三张
						comb->setGenre(static_cast<int>(GuanDanGenre::Triple1));
					}
					break;
				case 4:
					if (*it_l == 13) {
						// 构建王炸
						comb->setGenre(static_cast<int>(GuanDanGenre::BombJoker));
					}
					else {
						// 构建4炸
						comb->setGenre(static_cast<int>(GuanDanGenre::Bomb4));
					}
					break;
				case 5:
					// 构建5炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb5));
					break;
				case 6:
					// 构建6炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb6));
					break;
				case 7:
					// 构建7炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb7));
					break;
				case 8:
					// 构建8炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb8));
					break;
				case 9:
					// 构建9炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb9));
					break;
				case 10:
					// 构建10炸
					comb->setGenre(static_cast<int>(GuanDanGenre::Bomb10));
					break;
				default:
					break;
				}
				if (!getCombinationCards(rule, cardIds, *it_l, num, pointOrder1, pointOrder2)) {
					freeCombination(comb);
					std::string str;
					PokerUtilities::cardArray2String(_cards, str);
					ErrorS << "为出牌组合(牌型: " << comb->getGenre() << ")分配牌失败，找不到牌值顺序为" << *it_l << "的牌，当前手牌：" << str;
					return false;
				}
				point = _rule->getPointByOrder(*it_l);
				comb->addCards(cardIds);
				comb->setOfficerPoint(point);
				if (getCardById(cardIds.back(), c))
					comb->setOfficerSuit(c.getSuit());
				comb->setUndamages(undamages);
				insertCombination(comb, false);
			}
			++it_t;
		}
		return true;
	}

	bool GuanDanAvatar::getCombinationCards(GuanDanRule* rule, std::vector<int>& cardIds, int order, int num, int& pointOrder1, int& pointOrder2, bool occupy) {
		bool ret = true;
		int cardId = -1;
		std::vector<int> tmpIds;
		for (int i = 0; i < num; i++) {
			if ((order == pointOrder1) || (order == pointOrder2)) {
				// 抽取逢人配
				cardId = PokerUtilities::getCardId(_cards, _occupiedCardIds, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
				if (cardId != -1) {
					if (order == pointOrder1)
						pointOrder1 = -1;
					else
						pointOrder2 = -1;
					_occupiedVariableCards++;
				}
			}
			else
				cardId = getCardId(_occupiedCardIds, order);
			if (cardId == -1) {
				ret = false;
				break;
			}
			_occupiedCardIds.insert(cardId);
			cardIds.push_back(cardId);
			if (!occupy)
				tmpIds.push_back(cardId);
		}
		if (!occupy) {
			std::unordered_set<int>::const_iterator it_s;
			for (int id : tmpIds) {
				it_s = _occupiedCardIds.find(id);
				if (it_s != _occupiedCardIds.end())
					_occupiedCardIds.erase(it_s);
			}
		}
		return ret;
	}

	void GuanDanAvatar::candidateCombinationsImpl(int situation) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return;
		int genre = 0;
		int order = 0;
		bool test = false;
		CombinationMap::const_iterator it_m;
		for (int i = 0; i < 16; i++) {
			genre = GuanDanRule::GENRE_TABLE[i];
			it_m = _combinations.find(genre);
			if (it_m == _combinations.end())
				continue;
			const CombinationList& combList = it_m->second;
			if (combList.empty())
				continue;
			if (genre == static_cast<int>(GuanDanGenre::Straight) ||
				genre == static_cast<int>(GuanDanGenre::Pair3) ||
				genre == static_cast<int>(GuanDanGenre::Triple2) ||
				genre == static_cast<int>(GuanDanGenre::StraightFlush))
				test = true;
			else
				test = false;
			for (const PokerCombination::Ptr& comb : combList) {
				comb->setCandidate(true);
				if (i < 7) {
					if (situation == 2)
						order = GuanDanCandidateOrder::getSingleton().getOrder2(genre, comb->getOfficerPoint(), test ? -1 : rule->getGradePoint());
					else
						order = GuanDanCandidateOrder::getSingleton().getOrder1(genre, comb->getOfficerPoint(), test ? -1 : rule->getGradePoint());
					if (order == -1) {
						ErrorS << "未找到出牌组合的候选顺序(" << genre << ", " << comb->getOfficerPoint() << ")";
					}
					else {
						comb->setCandidateOrder(order);
						insertCandidate(comb);
					}
				}
				else
					_candidates.push_back(comb);
			}
		}
	}

	void GuanDanAvatar::candidateCombinationsImpl(const PokerGenre& pg, int situation) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		int genre = 0;
		int order = rule->getBombOrder(pg.getGenre());
		bool test = false;
		if (order == -1) {
			// 非炸弹牌型
			getCombination(pg.getGenre(), pg.getOfficer(), _candidates, true);
			order = 0;
			test = true;
		}
		for (int i = order; i < 9; i++) {
			genre = rule->getBombByOrder(i);
			if (test || (i > order))
				getCombination(genre, pg.getOfficer(), _candidates, false);
			else
				getCombination(genre, pg.getOfficer(), _candidates, true);
		}
		const PokerCard& officer = pg.getOfficer();
		genre = pg.getGenre();
		if (test) {
			if (genre == static_cast<int>(GuanDanGenre::Straight) ||
				genre == static_cast<int>(GuanDanGenre::Pair3) ||
				genre == static_cast<int>(GuanDanGenre::Triple2)) {
				gatherStraight(rule, genre, officer.getPoint());
			}
			else {
				// 单张、对子、三张、三带二
				gatherOther(rule, genre, officer);
			}
		}
		gatherBomb(rule, genre, officer.getPoint());
	}

	bool hasSameCombination(const PokerAvatar::CombinationVec& vec, int genre, int officerPoint) {
		for (const PokerCombination::Ptr& comb : vec) {
			if (comb->getGenre() != genre)
				continue;
			if (comb->getOfficerPoint() == officerPoint)
				return true;
		}
		return false;
	}

	void GuanDanAvatar::getCombination(int genre, const PokerCard& officer, PokerAvatar::CombinationVec& vec, bool compareOfficer) const {
		CombinationMap::const_iterator it = _combinations.find(genre);
		if (it == _combinations.end())
			return;
		const CombinationList& combList = it->second;
		if (combList.empty())
			return;
		int ret = 0;
		int order1 = 0;
		int order2 = _rule->getPointOrder(officer.getPoint());
		bool test = false;
		PokerCard c;
		if (genre == static_cast<int>(GuanDanGenre::Straight) ||
			genre == static_cast<int>(GuanDanGenre::Pair3) ||
			genre == static_cast<int>(GuanDanGenre::Triple2) ||
			genre == static_cast<int>(GuanDanGenre::StraightFlush))
			test = true;
		std::string str;
		for (const PokerCombination::Ptr& comb : combList) {
			if (compareOfficer) {
				if (test) {
					// 顺子、同化顺、木板、钢板不考虑级牌的大小，也不考虑花色
					order1 = _rule->getPointOrder(comb->getOfficerPoint());
					if (order1 > order2)
						vec.push_back(comb);
				}
				else {
					c.setPoint(comb->getOfficerPoint());
					c.setSuit(comb->getOfficerSuit());
					ret = _rule->compareCard(c, officer);
					if (ret == 1)
						vec.push_back(comb);
				}
			}
			else
				vec.push_back(comb);
		}
	}

	void GuanDanAvatar::gatherStraight(GuanDanRule* rule, int genre, int officerPoint) {
		int order1 = rule->getPointOrder(officerPoint);
		int order2 = 0;
		int damages = 0;
		int undamages = 0;
		int nums = 1;
		if (genre == static_cast<int>(GuanDanGenre::Pair3))
			nums = 2;
		else if (genre == static_cast<int>(GuanDanGenre::Triple2))
			nums = 3;
		PokerCombination::Ptr comb;
		for (const GuanDanSearch::Ptr& search : _searches) {
			if (search->getGenre() != genre)
				continue;	// 牌型不同，跳过
			if (search->getAdopt())
				continue;	// 已被采用，跳过
			if ((genre == static_cast<int>(GuanDanGenre::Straight)) &&
				(search->getStraightMultiple() == search->getSameSuitNum()))
				continue;	// 跳过同花顺
			order2 = search->getOfficerPointOrder();
			if (order2 <= order1)
				continue;	// 跳过小于或等于已出牌型的检索牌型
			comb = allocateCombination();
			const std::unordered_map<int, int>& graph = search->getPointOrderNums();
			calcDamages(graph, damages, undamages);
			comb->setGenre(genre);
			comb->setDamages(damages);
			comb->setUndamages(undamages);
			comb->setBad(true);
			if (!getCombinationCards1(rule, comb, graph, nums, order2, -1)) {
				freeCombination(comb);
				continue;
			}
			_candidates.push_back(comb);
		}
	}

	void GuanDanAvatar::gatherOther(GuanDanRule* rule, int genre, const PokerCard& officer) {
		int num = 0;
		if (genre == static_cast<int>(GuanDanGenre::Single))
			num = 1;
		else if (genre == static_cast<int>(GuanDanGenre::Pair1))
			num = 2;
		else if (genre == static_cast<int>(GuanDanGenre::Triple1) ||
			genre == static_cast<int>(GuanDanGenre::ThreeWith2))
			num = 3;
		else {
			ErrorS << "牌型错误: " << genre;
			return;
		}
		PokerCombination::Ptr comb;
		std::vector<int> cardIds;
		int damages = 0;
		int undamages = 0;
		int cnt = 0;
		if (officer.getPoint() == static_cast<int>(PokerPoint::Joker)) {
			if (num > 2)
				return;
			if (officer.getSuit() == static_cast<int>(PokerSuit::Big))
				return;
			PokerUtilities::getCardIds(_cards, cardIds, static_cast<int>(PokerPoint::Joker), static_cast<int>(PokerSuit::Big));
			cnt = static_cast<int>(cardIds.size());
			if (num > cnt)
				return;
			comb = allocateCombination();
			comb->setGenre(genre);
			if (num < cnt) {
				damages = 1;
				for (int i = 0; i < num; i++)
					comb->addCard(cardIds.at(i));
			}
			else {
				undamages = 1;
				comb->addCards(cardIds);
			}
			comb->setOfficerPoint(static_cast<int>(PokerPoint::Joker));
			comb->setOfficerSuit(static_cast<int>(PokerSuit::Big));
			comb->setDamages(damages);
			comb->setUndamages(undamages);
			insertCandidateJoker(comb);
			return;
		}
		int order1 = rule->getPointOrder(officer.getPoint());
		int order2 = rule->getPointOrder(rule->getGradePoint());
		if (order1 == order2) {
			order1 = 13;
			order2 = -1;
		}
		else
			order1 += 1;
		// key-牌值顺序，value-逢人配张数
		std::unordered_map<int, int> tmpMap;
		for (int i = 0; i < 14; i++) {
			if ((i < order1) && (i != order2))
				continue;
			if (_pointOrderNums[i] > 3)
				continue;
			if ((_pointOrderNums[i] == 0) && (i != order2))
				continue;
			if (i == 13) {
				// 大小王不能与逢人配匹配
				if (_pointOrderNums[i] < num)
					continue;
			}
			else if ((_pointOrderNums[i] + _variableCards) < num)
				continue;
			if (hasSameCombination(_candidates, genre, rule->getPointByOrder(i)))
				continue;
			if (num > _pointOrderNums[i])
				cnt = num - _pointOrderNums[i];
			else
				cnt = 0;
			tmpMap.insert(std::make_pair(i, cnt));
		}
		if (tmpMap.empty())
			return;
		bool flag = false;
		int cardId = 0;
		PokerCard c;
		CombinationList tmpList1;
		CombinationList tmpList2;
		std::unordered_set<int> tmpSet;
		std::unordered_map<int, int>::const_iterator it_m;
		if (genre == static_cast<int>(GuanDanGenre::ThreeWith2)) {
			undamages = 5;
			std::pair<int, int> pr;
			// 查找最小对子
			int idx = 0;
			int orders[14] = { 0 };
			if (order2 == -1)
				order2 = rule->getPointOrder(rule->getGradePoint());
			for (int i = 0; i < 13; i++) {
				if (i == order2)
					continue;
				orders[idx] = i;
				idx++;
			}
			orders[idx] = order2;
			idx++;
			orders[idx] = 13;
			for (int i = 0; i < 14; i++) {
				order1 = orders[i];
				if (_pointOrderNums[order1] == 2) {
					pr.first = order1;
					pr.second = 0;
					flag = true;
					break;
				}
			}
			if (!flag && (_variableCards > 0)) {
				for (int i = 0; i < 14; i++) {
					order1 = orders[i];
					if (_pointOrderNums[order1] == 1) {
						pr.first = order1;
						pr.second = 1;
						undamages = 4;
						flag = true;
						break;
					}
				}
			}
			if (!flag) {
				for (int i = 0; i < 14; i++) {
					order1 = orders[i];
					if (_pointOrderNums[order1] == 3) {
						pr.first = order1;
						pr.second = 0;
						damages = 1;
						undamages = 3;
						flag = true;
						break;
					}
				}
			}
			if (!flag)
				return;
			it_m = tmpMap.find(pr.first);
			if (it_m != tmpMap.end())
				tmpMap.erase(it_m);
			for (it_m = tmpMap.begin(); it_m != tmpMap.end(); it_m++) {
				if ((pr.second > 0) && ((it_m->second) > 0))
					continue;
				cnt = it_m->second;
				if (pr.second > 0)
					cnt = pr.second;
				comb = allocateCombination();
				if (cnt > 0) {
					// 抽取逢人配
					tmpSet.clear();
					flag = false;
					for (int i = 0; i < cnt; i++) {
						cardId = PokerUtilities::getCardId(_cards, tmpSet, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
						if (cardId != -1) {
							comb->addCard(cardId);
							tmpSet.insert(cardId);
						}
						else {
							flag = true;
							break;
						}
					}
					if (flag) {
						LOG_ERROR("error");
						freeCombination(comb);
						continue;
					}
				}
				cnt = 2 - pr.second;
				if (getCardIds(pr.first, cnt, cardIds)) {
					comb->addCards(cardIds);
				}
				else {
					LOG_ERROR("error");
					freeCombination(comb);
					continue;
				}
				flag = false;
				cnt = 3 - it_m->second;
				if (getCardIds(it_m->first, cnt, cardIds)) {
					comb->addCards(cardIds);
					if (getCardById(cardIds.back(), c)) {
						comb->setOfficerPoint(c.getPoint());
						comb->setOfficerSuit(c.getSuit());
					}
					else
						flag = true;
				}
				else
					flag = true;
				if (flag) {
					LOG_ERROR("error");
					freeCombination(comb);
					continue;
				}
				comb->setGenre(genre);
				comb->setDamages(damages);
				comb->setUndamages(undamages);
				comb->setBad(true);
				insertCandidateOther(comb, tmpList1, true);
			}
		}
		else {
			undamages = 1;
			for (it_m = tmpMap.begin(); it_m != tmpMap.end(); it_m++) {
				cnt = it_m->second;
				comb = allocateCombination();
				if (cnt > 0) {
					// 抽取逢人配
					tmpSet.clear();
					flag = false;
					for (int i = 0; i < cnt; i++) {
						cardId = PokerUtilities::getCardId(_cards, tmpSet, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
						if (cardId != -1) {
							comb->addCard(cardId);
							tmpSet.insert(cardId);
						}
						else {
							flag = true;
							break;
						}
					}
					if (flag) {
						LOG_ERROR("error");
						freeCombination(comb);
						continue;
					}
				}
				flag = false;
				cnt = num - it_m->second;
				if (cnt > 0) {
					if (getCardIds(it_m->first, cnt, cardIds)) {
						comb->addCards(cardIds);
						if (getCardById(cardIds.back(), c)) {
							comb->setOfficerPoint(c.getPoint());
							comb->setOfficerSuit(c.getSuit());
						}
						else
							flag = true;
					}
					else
						flag = true;
				}
				else {
					// 这个出牌组合只有逢人配，没其他牌
					comb->setOfficerPoint(rule->getGradePoint());
					comb->setOfficerSuit(static_cast<int>(PokerSuit::Heart));
				}
				if (flag) {
					LOG_ERROR("error");
					freeCombination(comb);
					continue;
				}
				comb->setGenre(genre);
				comb->setDamages(damages);
				comb->setUndamages(undamages);
				if ((it_m->first == 13) && (
					(genre == static_cast<int>(GuanDanGenre::Single)) || 
					(genre == static_cast<int>(GuanDanGenre::Pair1))))
					insertCandidateJoker(comb);
				else {
					comb->setBad(true);
					if (num < _pointOrderNums[it_m->first]) {
						// 先拆大牌再拆小牌
						insertCandidateOther(comb, tmpList2, false);
					}
					else
						insertCandidateOther(comb, tmpList1, true);
				}
			}
		}
		for (const PokerCombination::Ptr& tmp : tmpList1)
			_candidates.push_back(tmp);
		for (const PokerCombination::Ptr& tmp : tmpList2)
			_candidates.push_back(tmp);
	}

	void GuanDanAvatar::gatherBomb(GuanDanRule* rule, int genre, int officerPoint) {
		// 最少使用逢人配原子进行匹配炸弹
		// 这里不考虑王炸，因为如果有王炸肯定已经在候选列表里面了
		int num = 0;
		int bomb = 0;
		int order1 = rule->getBombOrder(genre);
		int order2 = 0;
		int variableCards = 0;
		int point = 0;
		int ret = 0;
		int cardId = 0;
		bool test = false;
		bool flag = false;
		PokerCard c;
		PokerCombination::Ptr comb;
		std::vector<int> cardIds;
		std::unordered_set<int> tmpSet;
		for (int i = 0; i < 13; i++) {
			num = _pointOrderNums[i];
			if (num < 2)
				continue;
			variableCards = 0;
			while (true) {
				test = false;
				switch (num) {
				case 4:
					bomb = static_cast<int>(GuanDanGenre::Bomb4);
					break;
				case 5:
					bomb = static_cast<int>(GuanDanGenre::Bomb5);
					break;
				case 6:
					bomb = static_cast<int>(GuanDanGenre::Bomb6);
					break;
				case 7:
					bomb = static_cast<int>(GuanDanGenre::Bomb7);
					break;
				case 8:
					bomb = static_cast<int>(GuanDanGenre::Bomb8);
					break;
				case 9:
					bomb = static_cast<int>(GuanDanGenre::Bomb9);
					break;
				case 10:
					bomb = static_cast<int>(GuanDanGenre::Bomb10);
					break;
				default:
					test = true;
					break;
				}
				if (!test) {
					point = rule->getPointByOrder(i);
					order2 = rule->getBombOrder(bomb);
					if (order2 < order1)
						test = true; 
					else if (order2 == order1) {
						// 同一种炸弹牌型，比较牌型主牌
						ret = rule->comparePoint(point, officerPoint);
						if (ret != 1)
							test = true;
					}
				}
				if (test) {
					if (variableCards < _variableCards) {
						num += 1;
						variableCards++;
						continue;
					}
				}
				else if (!hasSameCombination(_candidates, bomb, point)) {
					comb = allocateCombination();
					flag = false;
					if (variableCards > 0) {
						// 抽取逢人配
						tmpSet.clear();
						for (int i = 0; i < variableCards; i++) {
							cardId = PokerUtilities::getCardId(_cards, tmpSet, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
							if (cardId != -1) {
								comb->addCard(cardId);
								tmpSet.insert(cardId);
							}
							else {
								flag = true;
								break;
							}
						}
					}
					if (!flag) {
						num -= variableCards;
						if (getCardIds(i, num, cardIds)) {
							comb->addCards(cardIds);
							if (getCardById(cardIds.back(), c)) {
								comb->setOfficerPoint(c.getPoint());
								comb->setOfficerSuit(c.getSuit());
							}
							else
								flag = true;
						}
						else
							flag = true;
						if (!flag) {
							comb->setGenre(bomb);
							comb->setDamages(0);
							comb->setUndamages(1);
							comb->setBad(true);
							_candidates.push_back(comb);
						}
					}
					if (flag)
						freeCombination(comb);
				}
				break;
			} 
		}
		order2 = rule->getBombOrder(static_cast<int>(GuanDanGenre::StraightFlush));
		if (order1 > order2)
			return;
		if (order2 == order1) {
			test = true;
			order1 = rule->getPointOrder(officerPoint);
		}
		int damages = 0;
		int undamages = 0;
		int suit = 0;
		std::unordered_set<int>::const_iterator it_s;
		// 收集同花顺
		for (const GuanDanSearch::Ptr& search : _searches) {
			if (search->getGenre() != static_cast<int>(GuanDanGenre::Straight))
				continue;	// 牌型不同，跳过
			if (search->getAdopt())
				continue;	// 已被采用，跳过
			if (0 == search->getSameSuitNum())
				continue;	// 跳过非同花顺
			if (test) {
				order2 = search->getOfficerPointOrder();
				if (order2 <= order1)
					continue;	// 跳过小于或等于已出牌型的检索牌型
			}
			tmpSet.clear();
			const std::unordered_map<int, int>& graph = search->getPointOrderNums();
			calcDamages(graph, damages, undamages);
			undamages += search->getSameSuitNum() * 2;
			for (int i = 0; i < search->getSameSuitNum(); i++) {
				suit = search->getSameSuit(i);
				it_s = tmpSet.find(suit);
				if (it_s != tmpSet.end())
					continue;
				tmpSet.insert(suit);
				comb = allocateCombination();
				comb->setGenre(static_cast<int>(GuanDanGenre::StraightFlush));
				comb->setDamages(damages);
				comb->setUndamages(undamages);
				comb->setBad(true);
				if (!getCombinationCards1(rule, comb, graph, 1, order2, suit)) {
					freeCombination(comb);
					continue;
				}
				_candidates.push_back(comb);
			}
		}
	}

	void GuanDanAvatar::insertCandidate(const PokerCombination::Ptr& comb) {
		bool test = false;
		int order1 = comb->getCandidateOrder();
		int order2 = 0;
		CombinationVec::const_iterator it = _candidates.begin();
		for (; it != _candidates.end(); it++) {
			order2 = (*it)->getCandidateOrder();
			if (order1 < order2) {
				_candidates.insert(it, comb);
				test = true;
				break;
			}
		}
		if (!test)
			_candidates.push_back(comb);
	}

	void GuanDanAvatar::insertCandidateJoker(const PokerCombination::Ptr& comb) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return;
		bool test = false;
		int order = -1;
		CombinationVec::const_iterator it = _candidates.begin();
		for (; it != _candidates.end(); it++) {
			const PokerCombination::Ptr& tmp = *it;
			order = rule->getBombOrder(tmp->getGenre());
			if (order == -1)
				continue;
			_candidates.insert(it, comb);
			test = true;
			break;
		}
		if (!test)
			_candidates.push_back(comb);
	}

	void GuanDanAvatar::insertCandidateOther(const PokerCombination::Ptr& comb, PokerAvatar::CombinationList& combList, bool flag) {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return;
		bool test = false;
		int ret = 0;
		int point1 = comb->getOfficerPoint();
		int point2 = -1;
		CombinationList::const_iterator it;
		for (it = combList.begin(); it != combList.end(); it++) {
			const PokerCombination::Ptr& tmp = *it;
			point2 = tmp->getOfficerPoint();
			ret = rule->comparePoint(point1, point2);
			if (flag) {
				if (ret == 2) {
					combList.insert(it, comb);
					test = true;
				}
			}
			else if (ret == 1) {
				combList.insert(it, comb);
				test = true;
			}
			if (test)
				break;
		}
		if (!test)
			combList.push_back(comb);
	}

	void GuanDanAvatar::collectStraightFlush() {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return;
		_straightFlushIndex = 0;
		_straightFlushCardIds.clear();
		int num = 0;
		int multiple = 0;
		int point = 0;
		int suit = 0;
		int cardId = 0;
		int variableCards = 0;
		std::vector<int> cardIds;
		std::unordered_set<int> tmpSet;
		const std::unordered_set<int> EmptySet;
		std::unordered_map<int, int>::const_iterator it_g;
		std::list<GuanDanSearch::Ptr>::const_iterator it;
		for (it = _searches.begin(); it != _searches.end(); it++) {
			const GuanDanSearch::Ptr& search = *it;
			if (search->getGenre() != static_cast<int>(GuanDanGenre::Straight))
				continue;
			num = search->getSameSuitNum();
			if (num == 0)
				continue;
			multiple = search->getStraightMultiple();
			const std::unordered_map<int, int>& graph = search->getPointOrderNums();
			for (int i = 0; i < num; i++) {
				suit = search->getSameSuit(i);
				cardIds.clear();
				variableCards = 0;
				for (it_g = graph.begin(); it_g != graph.end(); ++it_g) {
					point = rule->getPointByOrder(it_g->first);
					cardId = PokerUtilities::getCardId(_cards, tmpSet, point, suit);
					if (cardId != -1)
						tmpSet.insert(cardId);
					else if ((it_g->second < multiple) && (variableCards < _variableCards)) {
						// 获取指定牌值及花色的牌失败，用逢人配作配
						cardId = PokerUtilities::getCardId(_cards, tmpSet, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
						// 逢人配已被占用，重复使用逢人配
						if (cardId == -1)
							cardId = PokerUtilities::getCardId(_cards, EmptySet, rule->getGradePoint(), static_cast<int>(PokerSuit::Heart));
						else
							tmpSet.insert(cardId);
						if (cardId != -1)
							variableCards++;
					}
					if (cardId == -1) {
						// 获取指定牌值及花色的牌失败，并且无逢人配，重复使用指定牌值及花色的牌
						cardId = PokerUtilities::getCardId(_cards, EmptySet, point, suit);
						if (cardId == -1) {
							std::string str;
							PokerUtilities::cardArray2String(_cards, str);
							ErrorS << "收集同花顺失败，找不到(" << point << ", " << suit << ")的牌，当前手牌：" << str;
							break;
						}
					}
					cardIds.push_back(cardId);
				}
				if (cardIds.size() == 5) {
					for (const int& tmpId : cardIds)
						_straightFlushCardIds.push_back(tmpId);
				}
				else {
					ErrorS << "error";
				}
			}
		}
	}

	bool GuanDanAvatar::getStraightFlush(std::vector<int>& cardIds) const {
		int size = static_cast<int>(_straightFlushCardIds.size());
		size /= 5;
		if (size == 0)
			return false;
		if ((_straightFlushIndex < 0) || (_straightFlushIndex >= size))
			_straightFlushIndex = 0;
		int idx = 0;
		cardIds.clear();
		for (int i = 0; i < 5; i++) {
			idx = _straightFlushIndex * 5 + i;
			cardIds.push_back(_straightFlushCardIds.at(idx));
		}
		_straightFlushIndex++;
		return true;
	}

	bool GuanDanAvatar::isAllBig() const {
		GuanDanRule* rule = dynamic_cast<GuanDanRule*>(_rule.get());
		if (rule == nullptr)
			return false;
		int orderBomb = -1;
		int order1 = rule->getPointOrder(static_cast<int>(PokerPoint::King));
		int order2 = 0;
		PokerCard card;
		CombinationMap::const_iterator it;
		std::unordered_set<int>::const_iterator it_s;
		for (it = _combinations.begin(); it != _combinations.end(); it++) {
			orderBomb = rule->getBombOrder(it->first);
			if (orderBomb != -1)
				continue;
			const CombinationList& combList = it->second;
			for (const PokerCombination::Ptr& comb : combList) {
				const std::unordered_set<int>& tmpSet = comb->getCards();
				for (it_s = tmpSet.begin(); it_s != tmpSet.end(); it_s++) {
					if (!getCardById(*it_s, card)) {
						LOG_ERROR("error");
						return false;
					}
					if (card.getPoint() == rule->getGradePoint())
						continue;
					order2 = rule->getPointOrder(card.getPoint());
					if (order2 < order1)
						return false;
				}
			}
		}
		// 手牌除了炸弹外全都比Q大，认为手牌全是大牌
		return true;
	}
}