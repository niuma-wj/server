// PokerRule.cpp

#include "PokerRule.h"
#include "PokerUtilities.h"

namespace NiuMa
{
	CardOrderTable::CardOrderTable()
	{
		_pointOrders[0] = static_cast<int>(PokerPoint::Three);
		_pointOrders[1] = static_cast<int>(PokerPoint::Four);
		_pointOrders[2] = static_cast<int>(PokerPoint::Five);
		_pointOrders[3] = static_cast<int>(PokerPoint::Six);
		_pointOrders[4] = static_cast<int>(PokerPoint::Seven);
		_pointOrders[5] = static_cast<int>(PokerPoint::Eight);
		_pointOrders[6] = static_cast<int>(PokerPoint::Nine);
		_pointOrders[7] = static_cast<int>(PokerPoint::Ten);
		_pointOrders[8] = static_cast<int>(PokerPoint::Jack);
		_pointOrders[9] = static_cast<int>(PokerPoint::Queen);
		_pointOrders[10] = static_cast<int>(PokerPoint::King);
		_pointOrders[11] = static_cast<int>(PokerPoint::Ace);
		_pointOrders[12] = static_cast<int>(PokerPoint::Two);
		_pointOrders[13] = static_cast<int>(PokerPoint::Joker);

		_suitOrders[0] = static_cast<int>(PokerSuit::Diamond);
		_suitOrders[1] = static_cast<int>(PokerSuit::Club);
		_suitOrders[2] = static_cast<int>(PokerSuit::Heart);
		_suitOrders[3] = static_cast<int>(PokerSuit::Spade);
		_suitOrders[4] = static_cast<int>(PokerSuit::Little);
		_suitOrders[5] = static_cast<int>(PokerSuit::Big);
	}

	CardOrderTable::~CardOrderTable()
	{}

	void CardOrderTable::setPointOrder(int point, int order) {
		if (order < 0 || order > 13)
			return;
		if (point < static_cast<int>(PokerPoint::Ace) ||
			point > static_cast<int>(PokerPoint::Joker))
			return;
		_pointOrders[order] = point;
	}

	// 返回传入的牌值在牌值大小顺序表中的位置
	int CardOrderTable::getPointOrder(int point) const {
		if (point == static_cast<int>(PokerPoint::Invalid))
			return -1;
		for (int i = 0; i < 14; i++) {
			if (point == _pointOrders[i])
				return i;
		}
		return -1;
	}

	void CardOrderTable::setSuitOrder(int suit, int order) {
		if (order < 0 || order > 5)
			return;
		if (suit < static_cast<int>(PokerSuit::Diamond) ||
			suit > static_cast<int>(PokerSuit::Big))
			return;
		_suitOrders[order] = suit;
	}

	int CardOrderTable::getSuitOrder(int suit) const {
		if (suit == static_cast<int>(PokerSuit::Invalid))
			return -1;
		for (int i = 0; i < 6; i++) {
			if (suit == _suitOrders[i])
				return i;
		}
		return -1;
	}

	int CardOrderTable::getPointByOrder(int order) const {
		if (order < 0 || order > 13)
			return static_cast<int>(PokerPoint::Invalid);

		return _pointOrders[order];
	}

	int CardOrderTable::getSuitByOrder(int order) const {
		if (order < 0 || order > 5)
			return static_cast<int>(PokerSuit::Invalid);

		return _suitOrders[order];
	}

	PokerRule::PokerRule()
		: _orderTable(NULL)
	{}

	PokerRule::~PokerRule() {
		if (_orderTable != NULL) {
			delete _orderTable;
			_orderTable = NULL;
		}
	}

	void PokerRule::initialise()
	{}

	int PokerRule::getPackNums() const {
		return 1;
	}

	int PokerRule::getPointNums() const {
		return 14;
	}

	void PokerRule::sortPointOrder() {}

	void PokerRule::sortSuitOrder() {}

	bool PokerRule::isDisapprovedCard(const PokerCard& c) const {
		return false;
	}

	bool PokerRule::isDisapprovedGenre(const PokerGenre& pcg) const {
		return false;
	}

	bool PokerRule::hasDisapprovedGenre(const CardArray& cards) const {
		return false;
	}

	int PokerRule::predicateCardGenre(PokerGenre& pcg) const {
		return 0;
	}

	int PokerRule::getGenreCardNums(int genre) const {
		return 0;
	}

	bool PokerRule::isValidGenre(int genre) const {
		if (genre <= 0)
			return false;

		return true;
	}

	int PokerRule::comparePoint(int pt1, int pt2) const {
		if (pt1 == pt2 || pt1 == static_cast<int>(PokerPoint::Invalid) || pt2 == static_cast<int>(PokerPoint::Invalid))
			return 0;

		int order1 = getPointOrder(pt1);
		int order2 = getPointOrder(pt2);
		if (order1 == -1 || order2 == -1)
			return 0;
		if (order1 > order2)
			return 1;
		else if (order2 > order1)
			return 2;

		return 0;
	}

	int PokerRule::compareSuit(int suit1, int suit2) const {
		if (suit1 == suit2 || suit1 == static_cast<int>(PokerSuit::Invalid) || suit2 == static_cast<int>(PokerSuit::Invalid))
			return 0;

		int order1 = getSuitOrder(suit1);
		int order2 = getSuitOrder(suit2);
		if (order1 == -1 || order2 == -1)
			return 0;
		if (order1 > order2)
			return 1;
		else if (order2 > order1)
			return 2;

		return 0;
	}

	int PokerRule::compareCard(const PokerCard& c1, const PokerCard& c2) const {
		int ret = comparePoint(c1.getPoint(), c2.getPoint());
		if (ret != 0)
			return ret;
		return compareSuit(c1.getSuit(), c2.getSuit());
	}

	bool PokerRule::straightExcluded(const PokerCard& c) const {
		return false;
	}

	bool PokerRule::straightPairExcluded(const PokerCard& c) const {
		return false;
	}

	bool PokerRule::straightTripleExcluded(const PokerCard& c) const {
		return false;
	}

	bool PokerRule::butterflyExcluded(const PokerCard& c) const {
		return false;
	}

	int PokerRule::getPointOrder(int point) const {
		if (_orderTable != NULL)
			return _orderTable->getPointOrder(point);

		return -1;
	}

	int PokerRule::getSuitOrder(int suit) const {
		if (_orderTable != NULL)
			return _orderTable->getSuitOrder(suit);

		return -1;
	}

	int PokerRule::getPointByOrder(int order) const {
		if (_orderTable != NULL)
			return _orderTable->getPointByOrder(order);

		return static_cast<int>(PokerPoint::Invalid);
	}

	int PokerRule::getSuitByOrder(int order) const {
		if (_orderTable != NULL)
			return _orderTable->getSuitByOrder(order);

		return static_cast<int>(PokerSuit::Invalid);
	}

	bool PokerRule::straight(const CardArray& cards) const {
		if (cards.size() < 5)
			return false;

		int pt = cards.at(0).getPoint();
		// 排除顺子里面不能包含的牌
		if (straightExcluded(cards.at(0)))
			return false;
		int order0 = getPointOrder(pt);
		int order1 = -1;
		if (order0 == -1)
			return false;
		unsigned int nums = static_cast<unsigned int>(cards.size());
		for (unsigned int i = 1; i < nums; i++) {
			if (straightExcluded(cards.at(i)))
				return false;
			pt = cards.at(i).getPoint();
			order1 = getPointOrder(pt);
			if (order1 - order0 != 1)
				return false;
			order0 = order1;
		}
		return true;
	}

	bool PokerRule::straightPair(const CardArray& cards) const {
		unsigned int nums = static_cast<unsigned int>(cards.size());
		if (nums == 0 || (nums & 0x1) != 0)	// 奇数直接返回
			return false;

		int pt0 = cards.at(0).getPoint();
		int pt1 = cards.at(1).getPoint();
		if (pt0 != pt1 || straightPairExcluded(cards.at(0)))
			return false;
		int order0 = getPointOrder(pt0);
		int order1 = -1;
		if (order0 == -1)
			return false;
		for (unsigned int i = 2; i < nums; i += 2) {
			pt0 = cards.at(i + 0).getPoint();
			pt1 = cards.at(i + 1).getPoint();
			if (pt0 != pt1 || straightPairExcluded(cards.at(i + 0)))
				return false;
			order1 = getPointOrder(pt0);
			if (order1 - order0 != 1)
				return false;
			order0 = order1;
		}
		return true;
	}

	bool PokerRule::straightTriple(const CardArray& cards) const {
		unsigned int nums = static_cast<unsigned int>(cards.size());
		if (nums == 0 || (nums % 3) != 0)	// 不是3的倍数直接返回
			return false;

		int pt0 = cards.at(0).getPoint();
		int pt1 = cards.at(1).getPoint();
		int pt2 = cards.at(2).getPoint();
		if (pt0 != pt1 || pt0 != pt2 || straightTripleExcluded(cards.at(0)))
			return false;
		int order0 = getPointOrder(pt0);
		int order1 = -1;
		if (order0 == -1)
			return false;
		for (unsigned int i = 3; i < nums; i += 3) {
			pt0 = cards.at(i + 0).getPoint();
			pt1 = cards.at(i + 1).getPoint();
			pt2 = cards.at(i + 2).getPoint();
			if (pt0 != pt1 || pt0 != pt2 || straightTripleExcluded(cards.at(i + 0)))
				return false;
			order1 = getPointOrder(pt0);
			if (order1 - order0 != 1)
				return false;
			order0 = order1;
		}
		return true;
	}

	bool PokerRule::butterfly(const CardArray& cards) const {
		const unsigned int nums = static_cast<unsigned int>(cards.size());
		// 10、15、20、25、30张牌才有可能是蝴蝶
		if (nums != 10 && nums != 15 && nums != 20 && nums != 25 && nums != 30)
			return false;

		const unsigned int straight = nums / 5;
		std::vector<int> cardPoints;
		cardPoints.reserve(nums);
		for (unsigned int i = 0; i < nums; i++)
			cardPoints.push_back(cards.at(i).getPoint());

		int pt0 = static_cast<int>(PokerPoint::Invalid);
		int pt1 = static_cast<int>(PokerPoint::Invalid);
		PokerCard c;
		bool bRet = false;
		int order0 = -1;
		int order1 = -1;
		for (unsigned int i = 0; i < straight; i++) {
			if (i == 0) {
				bRet = PokerUtilities::extractSamePointN(cardPoints, pt0, 3);
				if (pt0 == static_cast<int>(PokerPoint::Joker))
					c = PokerCard(PokerPoint::Joker, PokerSuit::Big);
				else
					c = PokerCard(static_cast<PokerPoint>(pt0), PokerSuit::Spade);
				if (!bRet || butterflyExcluded(c))
					return false;
			}
			else {
				bRet = PokerUtilities::extractSamePointN(cardPoints, pt1, 3);
				if (pt1 == static_cast<int>(PokerPoint::Joker))
					c = PokerCard(PokerPoint::Joker, PokerSuit::Big);
				else
					c = PokerCard(static_cast<PokerPoint>(pt1), PokerSuit::Spade);
				if (!bRet || butterflyExcluded(c))
					return false;
				order0 = getPointOrder(pt0);
				order1 = getPointOrder(pt1);
				if (order0 == -1 || order1 == -1)
					return false;
				if (order1 - order0 != 1)	// 非三顺
					return false;
				pt0 = pt1;
			}
		}
		if (straight == 2) {
			// 必须要连对
			bRet = PokerUtilities::extractSamePointN(cardPoints, pt0, 2);
			if (pt0 == static_cast<int>(PokerPoint::Joker))
				c = PokerCard(PokerPoint::Joker, PokerSuit::Big);
			else
				c = PokerCard(static_cast<PokerPoint>(pt0), PokerSuit::Spade);
			if (!bRet || butterflyExcluded(c))
				return false;
			bRet = PokerUtilities::extractSamePointN(cardPoints, pt1, 2);
			if (pt1 == static_cast<int>(PokerPoint::Joker))
				c = PokerCard(PokerPoint::Joker, PokerSuit::Big);
			else
				c = PokerCard(static_cast<PokerPoint>(pt1), PokerSuit::Spade);
			if (!bRet || butterflyExcluded(c))
				return false;
			order0 = getPointOrder(pt0);
			order1 = getPointOrder(pt1);
			if (order0 == -1 || order1 == -1)
				return false;
			if (order1 - order0 != 1)	// 非连对
				return false;
		}
		else {
			// 任意对子都可以
			for (unsigned int i = 0; i < straight; i++) {
				if (!PokerUtilities::extractSamePointN(cardPoints, pt0, 2))
					return false;
			}
		}
		return true;
	}

	PointOrderComparator::PointOrderComparator(const PokerRule* rule)
		: _rule(rule)
	{}

	bool PointOrderComparator::compareImpl(int a, int b) const {
		int order1 = _rule->getPointOrder(a);
		int order2 = _rule->getPointOrder(b);
		if (order1 == -1 || order2 == -1)
			return false;
		if (order1 > order2)
			return false;
		else if (order2 > order1)
			return true;
		return false;
	}

	CardComparator::CardComparator(const PokerRule::Ptr& rule)
		: _rule(rule)
	{}

	CardComparator::~CardComparator()
	{}

	bool CardComparator::operator()(const PokerCard& a, const PokerCard& b) const {
		int ret = _rule->compareCard(a, b);
		if (ret == 2)
			return true;
		else if (ret == 1)
			return false;
		if (a.getId() < b.getId())
			return true;
		return false;
	}
}