// DouDiZhuCombination.cpp

#include "DouDiZhuCombination.h"

namespace NiuMa
{
	DouDiZhuCombination::DouDiZhuCombination(const int id)
		: _id(id)
		, _genre(0)
		, _officer(0)
		, _damages(0)
		, _candidate(false)
	{}

	DouDiZhuCombination::~DouDiZhuCombination()
	{}

	int DouDiZhuCombination::getId() const {
		return _id;
	}

	void DouDiZhuCombination::setGenre(int g) {
		_genre = g;
	}

	int DouDiZhuCombination::getGenre() const {
		return _genre;
	}

	void DouDiZhuCombination::setOfficer(int o) {
		_officer = o;
	}

	int DouDiZhuCombination::getOfficer() const {
		return _officer;
	}

	void DouDiZhuCombination::setDamages(int d) {
		_damages = d;
	}

	int DouDiZhuCombination::getDamages() const {
		return _damages;
	}

	void DouDiZhuCombination::setCandidate(bool s) {
		_candidate = s;
	}

	bool DouDiZhuCombination::isCandidate() const {
		return _candidate;
	}

	std::unordered_set<int>& DouDiZhuCombination::getCards() {
		return _cards;
	}

	const std::unordered_set<int>& DouDiZhuCombination::getCards() const {
		return _cards;
	}

	void DouDiZhuCombination::getCards(std::vector<int>& ids) const {
		ids.clear();
		std::unordered_set<int>::const_iterator it = _cards.begin();
		while (it != _cards.end()) {
			ids.push_back(*it);
			++it;
		}
	}

	void DouDiZhuCombination::addCard(int id) {
		_cards.insert(id);
	}

	void DouDiZhuCombination::addCards(const std::vector<int>& ids) {
		std::vector<int>::const_iterator it = ids.begin();
		while (it != ids.end()) {
			_cards.insert(*it);
			++it;
		}
	}

	bool DouDiZhuCombination::containsCard(int id) const {
		std::unordered_set<int>::const_iterator it = _cards.find(id);
		return (it != _cards.end());
	}

	bool DouDiZhuCombination::containsCard(const std::vector<int>& ids) const {
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

	const std::unordered_map<int, int>& DouDiZhuCombination::getOrders() const {
		return _orders;
	}

	void DouDiZhuCombination::addOrder(int order, int nums) {
		std::unordered_map<int, int>::iterator it = _orders.find(order);
		if (it != _orders.end())
			(it->second) += nums;
		else
			_orders.insert(std::make_pair(order, nums));
	}

	bool DouDiZhuCombination::containsOrder(int order) const {
		std::unordered_map<int, int>::const_iterator it = _orders.find(order);
		return (it != _orders.end());
	}

	void DouDiZhuCombination::clear() {
		_genre = 0;
		_officer = 0;
		_damages = 0;
		_candidate = false;
		_cards.clear();
		_orders.clear();
	}
}