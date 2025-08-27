// PokerAvatar.cpp

#include "Base/Log.h"
#include "PokerAvatar.h"
#include "PokerUtilities.h"

#include <algorithm>

namespace NiuMa
{
	PokerAvatar::PokerAvatar(const PokerRule::Ptr& rule, const std::string& playerId, int seat, bool robot)
		: GameAvatar(playerId, robot)
		, _rule(rule)
		, _combId(1)
		, _candidatePos(0)
		, _cardsUpdated(true)
		, _played(false)
	{
		setSeat(seat);

		for (int i = 0; i < 14; i++)
			_pointOrderNums[i] = 0;
	}

	PokerAvatar::~PokerAvatar() {}

	void PokerAvatar::setCards(const CardArray& cards) {
		_cards = cards;
		_cardsUpdated = true;
	}

	const CardArray& PokerAvatar::getCards() const {
		return _cards;
	}

	void PokerAvatar::sortCards() {
		CardComparator comp(_rule);
		// 按升序排序，即排序后cards中的元素是从小到大
		std::sort(_cards.begin(), _cards.end(), comp);
	}

	int PokerAvatar::getCardNums() const {
		return static_cast<int>(_cards.size());
	}

	void PokerAvatar::setOuttedGenre(const PokerGenre& g) {
		_outtedGenre = g;
		_played = true;
	}

	const PokerGenre& PokerAvatar::getOuttedGenre() const {
		return _outtedGenre;
	}

	bool PokerAvatar::hasCard(int id) const {
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			if ((*it).getId() == id)
				return true;
			++it;
		}
		return false;
	}

	bool PokerAvatar::getCardsByIds(const std::vector<int>& ids, CardArray& cards) const {
		return PokerUtilities::getCardsByIds(_cards, cards, ids);
	}

	bool PokerAvatar::getCardById(int id, PokerCard& c) const {
		return PokerUtilities::getCardById(_cards, id, c);
	}

	void PokerAvatar::addCard(const PokerCard& c) {
		_cards.emplace_back(c);
		_cardsUpdated = true;
	}

	void PokerAvatar::removeCardById(int id) {
		bool test = false;
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			if ((*it).getId() == id) {
				_cards.erase(it);
				test = true;
				break;
			}
			++it;
		}
		if (test)
			_cardsUpdated = true;
	}

	void PokerAvatar::removeCardsByIds(const std::vector<int>& ids) {
		if (ids.empty())
			return ;
		bool test = false;
		unsigned int nums = 0;
		std::vector<int>::const_iterator it2;
		CardArray::const_iterator it1 = _cards.begin();
		while (it1 != _cards.end()) {
			test = false;
			it2 = ids.begin();
			while (it2 != ids.end()) {
				if ((*it1).getId() == *it2) {
					test = true;
					break;
				}
				++it2;
			}
			if (test) {
				it1 = _cards.erase(it1);
				nums++;
				if (nums == ids.size())
					break;
			}
			else
				++it1;
		}
		if (nums > 0)
			_cardsUpdated = true;
	}

	void PokerAvatar::cardIdsSortedByPoints(std::vector<int>& ids) const {
		int nums1 = 0;
		int nums2 = 0;
		bool test = false;
		std::vector<int> orders;
		std::vector<int>::iterator it;
		for (int i = 13; i > -1; i--) {
			nums1 = _pointOrderNums[i];
			if (nums1 == 0)
				continue;
			test = false;
			it = orders.begin();
			while (it != orders.end()) {
				nums2 = _pointOrderNums[*it];
				if (nums1 > nums2) {
					orders.insert(it, i);
					test = true;
					break;
				}
				++it;
			}
			if (!test)
				orders.push_back(i);
		}
		int point = 0;
		CardArray::const_reverse_iterator it1;
		ids.clear();
		ids.reserve(_cards.size());
		it = orders.begin();
		while (it != orders.end()) {
			point = _rule->getPointByOrder(*it);
			test = false;
			it1 = _cards.rbegin();
			while (it1 != _cards.rend()) {
				if ((*it1).getPoint() == point) {
					test = true;
					ids.push_back((*it1).getId());
				}
				else if (test)
					break;
				++it1;
			}
			++it;
		}
	}

	PokerCombination::Ptr PokerAvatar::allocateCombination() {
		PokerCombination::Ptr comb;
		if (_freeCombs.empty()) {
			comb = std::make_shared<PokerCombination>(_combId++);
		}
		else {
			comb = _freeCombs.back();
			comb->clear();
			_freeCombs.pop_back();
		}
		return comb;
	}

	void PokerAvatar::freeCombination(const PokerCombination::Ptr& comb) {
		_freeCombs.push_back(comb);
	}

	void PokerAvatar::clearAnalysis() {
		CombinationList::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		while (it2 != _combinations.end()) {
			CombinationList& combList = (it2->second);
			for (it1 = combList.begin(); it1 != combList.end(); ++it1)
				freeCombination(*it1);
			combList.clear();
			++it2;
		}
		for (int i = 0; i < 14; i++) {
			_pointOrderCards[i].clear();
			_pointOrderNums[i] = 0;
		}
		_jokerCards[0].clear();
		_jokerCards[1].clear();
		_candidates.clear();
		_candidatePos = 0;
	}

	void PokerAvatar::clearCandidates() {
		CombinationList::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		while (it2 != _combinations.end()) {
			CombinationList& combList = (it2->second);
			for (it1 = combList.begin(); it1 != combList.end(); ++it1)
				(*it1)->setCandidate(false);
			++it2;
		}
		_candidates.clear();
		_candidatePos = 0;
	}

	bool PokerAvatar::isCardsUpdated() const {
		return _cardsUpdated;
	}

	void PokerAvatar::analyzeCombinations() {
		if (!_cardsUpdated) {
			clearCandidates();
			return;
		}
		clearAnalysis();
		int point = 0;
		int suit = 0;
		int order = 0;
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			if (analyzeIgnore(*it)) {
				++it;
				continue;
			}
			point = (*it).getPoint();
			order = _rule->getPointOrder(point);
			_pointOrderCards[order].push_back((*it).getId());
			_pointOrderNums[order] += 1;
			if (point == static_cast<int>(PokerPoint::Joker)) {
				suit = (*it).getSuit();
				if (suit == static_cast<int>(PokerSuit::Little))
					_jokerCards[0].push_back((*it).getId());
				else
					_jokerCards[1].push_back((*it).getId());
			}
			++it;
		}
		combineAllGenres();

		_cardsUpdated = false;
	}

	bool PokerAvatar::analyzeIgnore(const PokerCard& c) {
		return false;
	}

	int PokerAvatar::getCombinationNums() const {
		int ret = 0;
		CombinationMap::const_iterator it;
		for (it = _combinations.begin(); it != _combinations.end(); it++) {
			const CombinationList& combList = it->second;
			ret += static_cast<int>(combList.size());
		}
		return ret;
	}

	void PokerAvatar::insertCombination(const PokerCombination::Ptr& comb, bool orderByPoint) {
		CombinationMap::iterator it = _combinations.find(comb->getGenre());
		if (it != _combinations.end()) {
			insertCombination(it->second, comb, orderByPoint);
		}
		else {
			_combinations.insert(std::make_pair(comb->getGenre(), CombinationList()));
			it = _combinations.find(comb->getGenre());
			CombinationList& combList = it->second;
			combList.push_back(comb);
		}
	}

	bool PokerAvatar::insertCombination(PokerAvatar::CombinationList& combList, const PokerCombination::Ptr& comb, bool orderByPoint) const {
		bool test = false;
		PokerCard c1;
		PokerCard c2;
		int ret = 0;
		int order1 = 0;
		int order2 = 0;
		if (orderByPoint)
			order1 = _rule->getPointOrder(comb->getOfficerPoint());
		else {
			c1.setPoint(comb->getOfficerPoint());
			c1.setSuit(comb->getOfficerSuit());
		}
		std::list<PokerCombination::Ptr>::const_iterator it;
		for (it = combList.begin(); it != combList.end(); it++) {
			const PokerCombination::Ptr& tmp = *it;
			if (tmp->getNet() > comb->getNet())
				continue;
			else if (tmp->getNet() < comb->getNet()) {
				combList.insert(it, comb);
				test = true;
				break;
			}
			else {
				if (orderByPoint) {
					order2 = _rule->getPointOrder(tmp->getOfficerPoint());
					if (order1 < order2) {
						combList.insert(it, comb);
						test = true;
						break;
					}
				}
				else {
					c2.setPoint(tmp->getOfficerPoint());
					c2.setSuit(tmp->getOfficerSuit());
					ret = _rule->compareCard(c1, c2);
					if (ret == 2) {
						combList.insert(it, comb);
						test = true;
						break;
					}
				}
			}
		}
		if (!test)
			combList.push_back(comb);
		return true;
	}

	bool PokerAvatar::getCardIds(int order, int nums, std::vector<int>& ids) const {
		if (order < 0 || order > 13)
			return false;
		if (static_cast<int>(_pointOrderCards[order].size()) < nums)
			return false;
		ids.clear();
		for (int i = 0; i < nums; i++)
			ids.push_back(_pointOrderCards[order].at(i));
		return true;
	}

	int PokerAvatar::getCardId(const std::unordered_set<int>& excludedIds, int order) {
		if (order < 0 || order > 13)
			return -1;
		const std::vector<int>& cardIds = _pointOrderCards[order];
		if (cardIds.empty())
			return -1;
		std::unordered_set<int>::const_iterator it_s;
		std::vector<int>::const_iterator it = cardIds.begin();
		for (; it != cardIds.end(); ++it) {
			it_s = excludedIds.find(*it);
			if (it_s == excludedIds.end())
				return *it;
		}
		return -1;
	}

	void PokerAvatar::candidateCombinations(int situation) {
		candidateCombinationsImpl(situation);
	}

	void PokerAvatar::candidateCombinations(const PokerGenre& pg, int situation) {
		candidateCombinationsImpl(pg, situation);
	}

	bool PokerAvatar::hasCandidate() const {
		return !(_candidates.empty());
	}

	bool PokerAvatar::getCandidateCards(std::vector<int>& ids) {
		int size = static_cast<int>(_candidates.size());
		if (size == 0)
			return false;
		if ((_candidatePos < 0) || (_candidatePos >= size))
			_candidatePos = 0;
		PokerCombination::Ptr comb = _candidates.at(_candidatePos);
		comb->getCards(ids);
		_candidatePos++;
		return true;
	}

	PokerCombination::Ptr PokerAvatar::getFirstCandidate() const {
		if (_candidates.empty())
			return PokerCombination::Ptr();
		return _candidates.front();
	}

	void PokerAvatar::resetCandidatePos() {
		_candidatePos = 0;
	}

	bool PokerAvatar::isPlayed() const {
		return _played;
	}

	void PokerAvatar::resetPlayed() {
		_played = false;
	}

	void PokerAvatar::clear() {
		_cards.clear();
		_outtedGenre.clear();

		_cardsUpdated = true;
		_played = false;
	}

	SortByCombines::SortByCombines(const std::unordered_map<int, int>& map)
		: _cardCombines(map)
	{}

	SortByCombines::~SortByCombines()
	{}

	bool SortByCombines::operator()(const int& id1, const int& id2) const {
		int nums1 = 0;
		int nums2 = 0;
		std::unordered_map<int, int>::const_iterator it = _cardCombines.find(id1);
		if (it != _cardCombines.end())
			nums1 = it->second;
		it = _cardCombines.find(id2);
		if (it != _cardCombines.end())
			nums2 = it->second;
		if (nums1 < nums2)
			return true;
		else if (nums1 > nums2)
			return false;
		if (id1 < id2)
			return true;
		return false;
	}
}