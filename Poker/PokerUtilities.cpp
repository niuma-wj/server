// PokerUtilities.cpp

#include "PokerRule.h"
#include "PokerUtilities.h"

#include <unordered_map>

namespace NiuMa
{
	PokerUtilities::PokerUtilities()
	{}

	PokerUtilities::~PokerUtilities()
	{}

	int PokerUtilities::getPointNums(const CardArray& cards, int point) {
		if (point == static_cast<int>(PokerPoint::Invalid))
			return 0;
		int nums = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if ((*it).getPoint() == point)
				nums++;
			++it;
		}
		return nums;
	}

	int PokerUtilities::getSuitNums(const CardArray& cards, int suit) {
		if (suit == static_cast<int>(PokerSuit::Invalid))
			return 0;
		int nums = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if ((*it).getSuit() == suit)
				nums++;
			++it;
		}
		return nums;
	}

	int PokerUtilities::getCardNums(const CardArray& cards, int point, int suit) {
		if (point == static_cast<int>(PokerPoint::Invalid))
			return 0;
		if (suit == static_cast<int>(PokerSuit::Invalid))
			return 0;
		int nums = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (((*it).getPoint() == point) && ((*it).getSuit() == suit))
				nums++;
			++it;
		}
		return nums;
	}

	bool ignore(const PokerCard& c, int ignorePoint, int ignoreSuit) {
		bool test = false;
		int point = c.getPoint();
		int suit = c.getSuit();
		if (ignorePoint > 0) {
			if (ignoreSuit > 0) {
				if (point == ignorePoint && suit == ignoreSuit)
					test = true;
			}
			else if (point == ignorePoint)
				test = true;
		}
		else if (ignoreSuit > 0) {
			if (suit == ignoreSuit)
				test = true;
		}
		return test;
	}

	int PokerUtilities::getPointTypes(const CardArray& cards, int ignorePoint, int ignoreSuit) {
		if (cards.empty())
			return 0;
		// 某张牌与其前一张不被忽略的牌的牌值是否相等
		bool test = false;
		int types = 0;
		CardArray::size_type size = cards.size();
		for (CardArray::size_type i = 0; i < size; i++) {
			const PokerCard& c = cards.at(i);
			if (ignore(c, ignorePoint, ignoreSuit))
				continue;
			// 因为cards已从小到打排序，所以每张牌仅需要与其前一张不被忽略的牌做比较，若与其前
			// 一张不被忽略的牌的牌值不相等，则种类数量加1
			test = false;
			if (i > 0) {
				for (CardArray::size_type j = i; j > 0; j--) {
					const PokerCard& c1 = cards.at(j - 1);
					if (ignore(c1, ignorePoint, ignoreSuit))
						continue;
					test = (c1.getPoint() == c.getPoint());
					break;
				}
			}
			if (!test)
				types++;
		}
		if (types == 0) {
			// cards中所有牌都是被忽略的牌
			types = 1;
		}
		return types;
	}

	void addPointGraph(std::vector<std::pair<int, int> >& graph, const std::pair<int, int>& pr, const PointComparator::Ptr& comp) {
		if (comp) {
			bool test = false;
			std::vector<std::pair<int, int> >::const_iterator it = graph.begin();
			while (it != graph.end()) {
				if ((*comp)(pr.first, it->first)) {
					graph.insert(it, pr);
					test = true;
					break;
				}
				++it;
			}
			if (!test)
				graph.push_back(pr);
		}
		else
			graph.push_back(pr);
	}

	void PokerUtilities::getPointGraph(const CardArray& cards, std::vector<std::pair<int, int> >& graph,
		int ignorePoint, int ignoreSuit, const PointComparator::Ptr& comp) {
		bool first = true;
		int point1 = 0;
		int point2 = 0;
		std::pair<int, int> pr;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (ignore(*it, ignorePoint, ignoreSuit)) {
				++it;
				continue;
			}
			point2 = (*it).getPoint();
			if (first) {
				first = false;
				pr.first = point2;
				pr.second = 1;
				point1 = point2;
			}
			else if (point1 != point2) {
				addPointGraph(graph, pr, comp);
				pr.first = point2;
				pr.second = 1;
				point1 = point2;
			}
			else {
				pr.second += 1;
			}
			++it;
		}
		if (!first)
			addPointGraph(graph, pr, comp);
	}

	bool PokerUtilities::samePoint(const CardArray& cards, int ignorePoint, int ignoreSuit) {
		if (cards.empty())
			return true;

		bool first = true;
		int point = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (ignore(*it, ignorePoint, ignoreSuit)) {
				++it;
				continue;
			}
			if (first) {
				first = false;
				point = (*it).getPoint();
			}
			else if (point != (*it).getPoint())
				return false;
			++it;
		}
		return true;
	}

	bool PokerUtilities::sameSuit(const CardArray& cards, int ignorePoint, int ignoreSuit) {
		if (cards.empty())
			return true;

		bool first = true;
		int suit = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (ignore(*it, ignorePoint, ignoreSuit)) {
				++it;
				continue;
			}
			if (first) {
				first = false;
				suit = (*it).getSuit();
			}
			else if (suit != (*it).getSuit())
				return false;
			++it;
		}
		return true;
	}

	bool PokerUtilities::findSamePointN(const CardArray& cards, PokerCard& c, int N_) {
		int nums = static_cast<int>(cards.size());
		if (nums == 0 || N_ == 0 || nums < N_)
			return false;

		bool found = true;
		int count = 0;
		int pt0 = static_cast<int>(PokerPoint::Invalid);
		int pt1 = static_cast<int>(PokerPoint::Invalid);
		for (int i = 0; i < nums; i++) {
			if (i == 0) {
				c = cards[i];
				pt0 = cards[i].getPoint();
			}
			else {
				pt1 = cards[i].getPoint();
				if (pt0 != pt1) {
					c = cards[i];
					pt0 = pt1;
					count = 0;
				}
			}
			count++;
			if (count == N_) {
				found = true;
				break;
			}
		}
		return found;
	}

	bool PokerUtilities::rfindSamePointN(const CardArray& cards, PokerCard& c, int N_) {
		int nums = static_cast<int>(cards.size());
		if (nums == 0 || N_ == 0 || nums < N_)
			return false;

		bool found = false;
		int count = 0;
		int pt0 = static_cast<int>(PokerPoint::Invalid);
		int pt1 = static_cast<int>(PokerPoint::Invalid);
		for (int i = nums; i > 0; i--) {
			if (i == nums) {
				c = cards[i - 1];
				pt0 = cards[i - 1].getPoint();
			}
			else {
				pt1 = cards[i - 1].getPoint();
				if (pt0 != pt1) {
					c = cards[i - 1];
					pt0 = pt1;
					count = 0;
				}
			}
			count++;
			if (count == N_) {
				found = true;
				break;
			}
		}
		return found;
	}

	bool PokerUtilities::extractSamePointN(std::vector<int>& cardPoints, int& point, int N_) {
		int nums = static_cast<int>(cardPoints.size());
		if (nums == 0 || N_ == 0 || nums < N_)
			return false;

		bool test = false;
		int pos = 0;
		int tmp = 0;
		while (pos <= (nums - N_)) {
			int pt0 = cardPoints.at(pos);
			int pt1 = static_cast<int>(PokerPoint::Invalid);
			if (pt0 == static_cast<int>(PokerPoint::Invalid)) {
				pos++;
				continue;
			}
			test = true;
			for (int i = 1; i < N_; i++) {
				tmp = pos + i;
				pt1 = cardPoints.at(tmp);
				if (pt0 != pt1) {
					test = false;
					break;
				}
			}
			if (test) {
				point = pt0;
				for (int i = 0; i < N_; i++)
					cardPoints[pos + i] = static_cast<int>(PokerPoint::Invalid);
				break;
			}
			else
				pos = tmp;
		}
		return test;
	}

	int PokerUtilities::countSameCard(const CardArray& cards, const PokerCard& c) {
		int nums = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (c == *it)
				nums++;
			++it;
		}
		return nums;
	}

	bool PokerUtilities::hasPoint(const CardArray& cards, int point) {
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (it->getPoint() == point)
				return true;
			++it;
		}
		return false;
	}

	bool PokerUtilities::hasSuit(const CardArray& cards, int suit) {
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (it->getSuit() == suit)
				return true;
			++it;
		}
		return false;
	}

	bool PokerUtilities::hasCard(const CardArray& cards, int id) {
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (it->getId() == id)
				return true;
			++it;
		}
		return false;
	}

	bool PokerUtilities::hasCard(const CardArray& cards, int point, int suit) {
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (it->getPoint() == point && it->getSuit() == suit)
				return true;
			++it;
		}
		return false;
	}

	bool PokerUtilities::getFirstCardOfPoint(const CardArray& cards, PokerCard& c, int point) {
		bool found = false;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			const PokerCard& tmp = *it;
			if (tmp.getPoint() == point) {
				c = tmp;
				found = true;
				break;
			}
			++it;
		}
		return found;
	}

	bool PokerUtilities::getLastCardOfPoint(const CardArray& cards, PokerCard& c, int point) {
		bool found = false;
		CardArray::const_reverse_iterator it = cards.rbegin();
		while (it != cards.rend()) {
			const PokerCard& tmp = *it;
			if (tmp.getPoint() == point) {
				c = tmp;
				found = true;
				break;
			}
			++it;
		}
		return found;
	}

	bool PokerUtilities::getCardsByIds(const CardArray& cardsIn, CardArray& cardsOut, const std::vector<int>& ids) {
		if (ids.empty())
			return false;
		cardsOut.clear();
		bool ret = true;
		bool test = false;
		std::vector<int>::const_iterator it = ids.begin();
		for (; it != ids.end(); ++it) {
			test = false;
			for (const PokerCard& c : cardsIn) {
				if (c.getId() == *it) {
					cardsOut.push_back(c);
					test = true;
					break;
				}
			}
			if (!test)
				ret = false;
		}
		return ret;
	}

	bool PokerUtilities::getCardById(const CardArray& cards, int id, PokerCard& c) {
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if ((*it).getId() == id) {
				c = *it;
				return true;
			}
			++it;
		}
		return false;
	}

	void PokerUtilities::getCards(const CardArray& cards, CardArray& results, int point, int suit) {
		bool test1 = false;
		bool test2 = false;
		if (point == -1)
			test1 = true;
		if (suit == -1)
			test2 = true;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			const PokerCard& c = *it;
			if (point != -1)
				test1 = (c.getPoint() == point);
			if (suit != -1)
				test2 = (c.getSuit() == suit);
			if (test1 && test2)
				results.push_back(c);
			it++;
		}
	}

	void PokerUtilities::getCardIds(const CardArray& cards, std::vector<int>& ids, int point, int suit) {
		bool test1 = false;
		bool test2 = false;
		if (point == -1)
			test1 = true;
		if (suit == -1)
			test2 = true;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			const PokerCard& c = *it;
			if (point != -1)
				test1 = (c.getPoint() == point);
			if (suit != -1)
				test2 = (c.getSuit() == suit);
			if (test1 && test2)
				ids.push_back(c.getId());
			it++;
		}
	}

	int PokerUtilities::getCardId(const CardArray& cards, const std::unordered_set<int>& excludedIds, int point, int suit) {
		bool test1 = false;
		bool test2 = false;
		if (point == -1)
			test1 = true;
		if (suit == -1)
			test2 = true;
		std::unordered_set<int>::const_iterator it_s;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			const PokerCard& c = *it;
			if (point != -1)
				test1 = (c.getPoint() == point);
			if (suit != -1)
				test2 = (c.getSuit() == suit);
			if (test1 && test2) {
				it_s = excludedIds.find(it->getId());
				if (it_s == excludedIds.end())
					return it->getId();
			}
			it++;
		}
		return -1;
	}

	bool PokerUtilities::getStraightCards(const CardArray& cards, CardArray& results, int point, int straight, int N_, const std::shared_ptr<PokerRule>& rule) {
		if (!rule)
			return false;

		bool found = false;
		int pt1 = static_cast<int>(PokerPoint::Invalid);
		int pt2 = static_cast<int>(PokerPoint::Invalid);
		int nums1 = 0;
		int nums2 = 0;
		int tmp = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			pt1 = it->getPoint();
			if (found) {
				if (pt1 != pt2) {
					tmp = rule->getPointOrder(pt1);
					tmp -= rule->getPointOrder(pt2);
					if ((tmp != 1) || (nums1 < N_))
						return false;
					else {
						nums1 = 0;
						pt2 = pt1;
					}
				}
			}
			else if (pt1 == point) {
				found = true;
				pt2 = pt1;
			}
			else {
				++it;
				continue;
			}
			if (nums1 < N_) {
				results.push_back(*it);
				nums1++;
				if (nums1 == N_)
					nums2++;
				if (nums2 == straight)
					break;
			}
			++it;
		}
		return true;
	}

	void PokerUtilities::moveCardsOfPoint2FrontBack(CardArray& cards, int point, bool front) {
		CardArray cards1;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			if (it->getPoint() == point) {
				cards1.push_back(*it);
				it = cards.erase(it);
			}
			else
				++it;
		}
		if (cards1.empty())
			return;
		if (front)
			cards.insert(cards.begin(), cards1.begin(), cards1.end());
		else
			cards.insert(cards.end(), cards1.begin(), cards1.end());
	}

	void PokerUtilities::cardArraySubtract(CardArray& cards1, const CardArray& cards2) {
		bool found = false;
		CardArray cards;
		cards.reserve(cards1.size());
		CardArray::const_iterator it1 = cards1.begin();
		CardArray::const_iterator it2;
		while (it1 != cards1.end()) {
			it2 = cards2.begin();
			while (it2 != cards2.end()) {
				if (it1->getId() == it2->getId()) {
					found = true;
					break;
				}
				++it2;
			}
			if (found)
				found = false;
			else
				cards.push_back(*it1);
			++it1;
		}
		cards1 = cards;
	}

	void PokerUtilities::cardArray2String(const CardArray& cards, std::string& str) {
		str.clear();
		std::string strTmp;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			it->toString(strTmp);
			if (!str.empty())
				str += ",";
			str += strTmp;
			++it;
		}
	}

	void PokerUtilities::getJokerSuitNums(const CardArray& cards, int& big, int& little) {
		big = 0;
		little = 0;
		bool test = false;
		unsigned int nums = static_cast<unsigned int>(cards.size());
		for (unsigned int i = nums; i > 0; i--) {
			if (cards[i - 1].getPoint() == static_cast<int>(PokerPoint::Joker)) {
				test = true;
				if (cards[i - 1].getSuit() == static_cast<int>(PokerSuit::Big))
					big++;
				else if (cards[i - 1].getSuit() == static_cast<int>(PokerSuit::Little))
					little++;
			}
			else if (test)
				break;
		}
	}

	void PokerUtilities::sortCardsByPoints(const CardArray& cardsIn, CardArray& cardsOut) {
		cardsOut.clear();
		cardsOut.reserve(cardsIn.size());

		int point = 0;
		std::unordered_map<int, int> mapPoints;
		std::unordered_map<int, unsigned int> mapIndices;
		std::unordered_map<int, int>::iterator it1;
		CardArray::const_iterator it2 = cardsIn.begin();
		while (it2 != cardsIn.end()) {
			point = (*it2).getPoint();
			it1 = mapPoints.find(point);
			if (it1 != mapPoints.end())
				(it1->second) = (it1->second) + 1;
			else {
				mapPoints.insert(std::make_pair(point, 1));
				mapIndices.insert(std::make_pair(point, static_cast<unsigned int>(it2 - cardsIn.begin())));
			}
			++it2;
		}
		int tmp = 0;
		unsigned int idx1 = 0;
		unsigned int idx2 = 0;
		bool test1 = false;
		bool test2 = false;
		std::vector<int> points;
		std::vector<int>::iterator it3;
		points.reserve(mapPoints.size());
		it1 = mapPoints.begin();
		while (it1 != mapPoints.end()) {
			test1 = false;
			test2 = false;
			it3 = points.begin();
			while (it3 != points.end()) {
				tmp = mapPoints[*it3];
				if ((it1->second) > tmp)
					test2 = true;
				else if ((it1->second) == tmp) {
					idx1 = mapIndices[it1->first];
					idx2 = mapIndices[*it3];
					if (idx1 > idx2)
						test2 = true;
				}
				if (test2) {
					points.insert(it3, it1->first);
					test1 = true;
					break;
				}
				++it3;
			}
			if (!test1)
				points.push_back(it1->first);
			++it1;
		}
		CardArray::const_reverse_iterator it4;
		it3 = points.begin();
		while (it3 != points.end()) {
			test1 = false;
			it4 = cardsIn.rbegin();
			while (it4 != cardsIn.rend()) {
				if ((*it4).getPoint() == *it3) {
					cardsOut.push_back(*it4);
					test1 = true;
				}
				else if (test1)
					break;
				++it4;
			}
			++it3;
		}
	}

	void PokerUtilities::getBlackRedNums(const CardArray& cards, int& black, int& red) {
		black = 0;
		red = 0;
		int suit = 0;
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			suit = (*it).getSuit();
			if (suit == static_cast<int>(PokerSuit::Diamond) ||
				suit == static_cast<int>(PokerSuit::Heart) ||
				suit == static_cast<int>(PokerSuit::Big))
				red++;
			else
				black++;
			++it;
		}
	}

	bool PokerUtilities::getBiggestCard(const CardArray& cards, PokerCard& c, int ignorePoint, int ignoreSuit) {
		bool ret = false;
		CardArray::const_reverse_iterator rit = cards.rbegin();
		while (rit != cards.rend()) {
			if (ignore(*rit, ignorePoint, ignoreSuit)) {
				++rit;
				continue;
			}
			c = *rit;
			ret = true;
			break;
		}
		return ret;
	}
}