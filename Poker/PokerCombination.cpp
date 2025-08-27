// PokerCombination.cpp

#include "PokerCombination.h"

namespace NiuMa
{
	PokerCombination::PokerCombination(int id)
		: _id(id)
		, _genre(0)
		, _officerPoint(0)
		, _officerSuit(0)
		, _damages(0)
		, _undamages(0)
		, _candidateOrder(-1)
		, _candidate(false)
	{}

	PokerCombination::~PokerCombination()
	{}

	int PokerCombination::getId() const {
		return _id;
	}

	void PokerCombination::setGenre(int g) {
		_genre = g;
	}

	int PokerCombination::getGenre() const {
		return _genre;
	}

	void PokerCombination::setOfficerPoint(int point) {
		_officerPoint = point;
	}

	int PokerCombination::getOfficerPoint() const {
		return _officerPoint;
	}

	void PokerCombination::setOfficerSuit(int suit) {
		_officerSuit = suit;
	}

	int PokerCombination::getOfficerSuit() const {
		return _officerSuit;
	}

	void PokerCombination::setDamages(int d) {
		_damages = d;
	}

	int PokerCombination::getDamages() const {
		return _damages;
	}

	void PokerCombination::setUndamages(int u) {
		_undamages = u;
	}

	int PokerCombination::getUndamages() const {
		return _undamages;
	}

	int PokerCombination::getNet() const {
		return (_undamages - _damages);
	}

	void PokerCombination::setCandidateOrder(int order) {
		_candidateOrder = order;
	}

	int PokerCombination::getCandidateOrder() const {
		return _candidateOrder;
	}

	void PokerCombination::setCandidate(bool s) {
		_candidate = s;
	}

	bool PokerCombination::isCandidate() const {
		return _candidate;
	}

	std::unordered_set<int>& PokerCombination::getCards() {
		return _cards;
	}

	const std::unordered_set<int>& PokerCombination::getCards() const {
		return _cards;
	}

	void PokerCombination::getCards(std::vector<int>& ids) const {
		ids.clear();
		std::unordered_set<int>::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			ids.push_back(*it);
			++it;
		}
	}

	void PokerCombination::addCard(int id) {
		_cards.insert(id);
	}

	void PokerCombination::addCards(const std::vector<int>& ids) {
		std::vector<int>::const_iterator it = ids.begin();
		while (it != ids.end()) {
			_cards.insert(*it);
			++it;
		}
	}

	bool PokerCombination::containsCard(int id) const {
		std::unordered_set<int>::const_iterator it = _cards.find(id);
		return (it != _cards.end());
	}

	bool PokerCombination::containsCard(const std::vector<int>& ids) const {
		std::vector<int>::const_iterator it1 = ids.begin();
		std::unordered_set<int>::const_iterator it2;
		while (it1 != ids.end()) {
			it2 = _cards.find(*it1);
			if (it2 != _cards.end())
				return true;
			it1++;
		}
		return false;
	}

	const std::unordered_map<int, int>& PokerCombination::getOrders() const {
		return _orders;
	}

	void PokerCombination::addOrder(int order, int nums) {
		std::unordered_map<int, int>::iterator it = _orders.find(order);
		if (it != _orders.end())
			(it->second) += nums;
		else
			_orders.insert(std::make_pair(order, nums));
	}

	bool PokerCombination::containsOrder(int order) const {
		std::unordered_map<int, int>::const_iterator it = _orders.find(order);
		return (it != _orders.end());
	}

	void PokerCombination::clear() {
		_genre = 0;
		_officerPoint = 0;
		_officerSuit = 0;
		_damages = 0;
		_undamages = 0;
		_candidateOrder = -1;
		_candidate = false;
		_cards.clear();
		_orders.clear();
	}
}