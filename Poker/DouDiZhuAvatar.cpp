// DouDiZhuAvatar.cpp

#include "DouDiZhuAvatar.h"

#include <algorithm>

namespace NiuMa
{
	DouDiZhuAvatar::DouDiZhuAvatar(const PokerRule::Ptr& rule, const std::string& playerId, int seat, bool robot)
		: GameAvatar(playerId, robot)
		, _rule(rule)
		, _combId(1)
		, _candidatePos(0)
		, _cardsUpdated(true)
		, _played(false)
	{
		setSeat(seat);

		for (unsigned int i = 0; i < 14; i++)
			_pointNums[i] = 0;
	}

	DouDiZhuAvatar::~DouDiZhuAvatar() {
		_allCombs.clear();
	}

	void DouDiZhuAvatar::setCards(const CardArray& cards) {
		_cards = cards;
		_cardsUpdated = true;
	}

	const CardArray& DouDiZhuAvatar::getCards() const {
		return _cards;
	}

	int DouDiZhuAvatar::getCardNums() const {
		return static_cast<int>(_cards.size());
	}

	void DouDiZhuAvatar::setOuttedGenre(const PokerGenre& g) {
		_outtedGenre = g;
		_played = true;
	}

	const PokerGenre& DouDiZhuAvatar::getOuttedGenre() const {
		return _outtedGenre;
	}

	bool DouDiZhuAvatar::hasCard(int id) const {
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			if ((*it).getId() == id)
				return true;
			++it;
		}
		return false;
	}

	bool DouDiZhuAvatar::getCardsByIds(const std::vector<int>& ids, CardArray& cards) const {
		if (ids.empty())
			return false;
		cards.clear();
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
			if (!test) {
				++it1;
				continue;
			}
			cards.push_back(*it1);
			nums++;
			if (nums == ids.size())
				break;
			++it1;
		}
		return (nums == ids.size());
	}

	bool DouDiZhuAvatar::getCardById(int id, PokerCard& c) const {
		CardArray::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			if ((*it).getId() == id) {
				c = *it;
				return true;
			}
			++it;
		}
		return false;
	}

	void DouDiZhuAvatar::removeCardsByIds(const std::vector<int>& ids) {
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

	void DouDiZhuAvatar::cardIdsSortedByPoints(std::vector<int>& ids) const {
		int nums1 = 0;
		int nums2 = 0;
		bool test = false;
		std::vector<int> orders;
		std::vector<int>::iterator it;
		for (int i = 13; i > -1; i--) {
			nums1 = _pointNums[i];
			if (nums1 == 0)
				continue;
			test = false;
			it = orders.begin();
			while (it != orders.end()) {
				nums2 = _pointNums[*it];
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

	DouDiZhuCombination::Ptr DouDiZhuAvatar::allocateCombination() {
		DouDiZhuCombination::Ptr comb;
		if (_freeCombs.empty()) {
			comb = std::make_shared<DouDiZhuCombination>(_combId++);
			_allCombs.push_back(comb);
		}
		else {
			comb = _freeCombs.back();
			comb->clear();
			_freeCombs.pop_back();
		}
		return comb;
	}

	void DouDiZhuAvatar::freeCombination(const DouDiZhuCombination::Ptr& comb) {
		_freeCombs.push_back(comb);
	}

	void DouDiZhuAvatar::clearAnalysis() {
		CombinationVec::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		while (it2 != _combinations.end()) {
			CombinationVec& vec = (it2->second);
			it1 = vec.begin();
			while (it1 != vec.end()) {
				freeCombination(*it1);
				++it1;
			}
			vec.clear();
			++it2;
		}
		for (unsigned int i = 0; i < 14; i++) {
			_pointCards[i].clear();
			_pointNums[i] = 0;
		}
		_jokerCards[0].clear();
		_jokerCards[1].clear();
		_candidates.clear();
		_candidatePos = 0;
	}

	void DouDiZhuAvatar::clearCandidates() {
		CombinationVec::iterator it1;
		CombinationMap::iterator it2 = _combinations.begin();
		while (it2 != _combinations.end()) {
			CombinationVec& vec = (it2->second);
			it1 = vec.begin();
			while (it1 != vec.end()) {
				(*it1)->setCandidate(false);
				++it1;
			}
			++it2;
		}
		_candidates.clear();
		_candidatePos = 0;
	}

	void DouDiZhuAvatar::analyzeCombinations() {
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
			point = (*it).getPoint();
			order = _rule->getPointOrder(point);
			_pointCards[order].push_back((*it).getId());
			_pointNums[order] += 1;
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

	void DouDiZhuAvatar::addCombination(int genre, const DouDiZhuCombination::Ptr& comb) {
		CombinationMap::iterator it = _combinations.find(genre);
		if (it != _combinations.end()) {
			CombinationVec& vec = it->second;
			vec.push_back(comb);
		}
		else {
			_combinations.insert(std::make_pair(genre, CombinationVec()));
			it = _combinations.find(genre);
			CombinationVec& vec = it->second;
			vec.push_back(comb);
		}
	}

	bool DouDiZhuAvatar::getCardIds(int order, int nums, std::vector<int>& ids) const {
		if (order < 0 || order > 13)
			return false;
		if (static_cast<int>(_pointCards[order].size()) < nums)
			return false;
		ids.clear();
		for (int i = 0; i < nums; i++)
			ids.push_back(_pointCards[order].at(i));
		return true;
	}

	void DouDiZhuAvatar::candidateCombinations()
	{
		candidateCombinationsImpl();
	}

	void DouDiZhuAvatar::candidateCombinations(const PokerGenre& pg)
	{
		candidateCombinationsImpl(pg);
	}

	bool DouDiZhuAvatar::hasCandidate() const
	{
		return !(_candidates.empty());
	}

	bool DouDiZhuAvatar::getCandidateCards(std::vector<int>& ids) {
		if (_candidates.empty())
			return false;
		if (_candidatePos >= static_cast<int>(_candidates.size()))
			_candidatePos = 0;
		DouDiZhuCombination::Ptr comb = _candidates.at(_candidatePos);
		comb->getCards(ids);
		_candidatePos++;
		return true;
	}

	DouDiZhuCombination::Ptr DouDiZhuAvatar::getFirstCandidate() const {
		if (_candidates.empty())
			return nullptr;
		return _candidates.front();
	}

	bool DouDiZhuAvatar::isPlayed() const {
		return _played;
	}

	void DouDiZhuAvatar::clear() {
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