// GuanDanSearch.cpp

#include "GuanDanSearch.h"

namespace NiuMa
{
	GuanDanSearchData::GuanDanSearchData(int id)
		: _id(id)
		, _damages(0)
		, _undamages(0)
		, _variableCards(0)
	{}

	GuanDanSearchData::~GuanDanSearchData()
	{}

	int GuanDanSearchData::getId() const {
		return _id;
	}

	int GuanDanSearchData::getDamages() const {
		return _damages;
	}

	void GuanDanSearchData::setDamages(int damages) {
		_damages = damages;
	}

	int GuanDanSearchData::getUndamages() const {
		return _undamages;
	}

	void GuanDanSearchData::setUndamages(int undamages) {
		_undamages = undamages;
	}

	int GuanDanSearchData::getNet() const {
		return (_undamages - _damages);
	}

	int GuanDanSearchData::getVariableCards() const {
		return _variableCards;
	}

	void GuanDanSearchData::setVariableCards(int num) {
		_variableCards = num;
	}

	const std::unordered_map<int, int>& GuanDanSearchData::getPointOrderNums() const {
		return _pointOrderNums;
	}

	void GuanDanSearchData::setPointOrderNums(const std::unordered_map<int, int>& graph) {
		_pointOrderNums = graph;
	}

	int GuanDanSearchData::getPointOrderNum(int order) const {
		std::unordered_map<int, int>::const_iterator it = _pointOrderNums.find(order);
		if (it != _pointOrderNums.end())
			return it->second;
		return 0;
	}

	void GuanDanSearchData::addPointOrderNum(int order, int num) {
		std::unordered_map<int, int>::iterator it = _pointOrderNums.find(order);
		if (it != _pointOrderNums.end())
			it->second = it->second + num;
		else
			_pointOrderNums.insert(std::pair<int, int>(order, num));
	}

	void GuanDanSearchData::subtractGraph(const std::unordered_map<int, int>& graph) {
		std::unordered_map<int, int>::const_iterator it1;
		std::unordered_map<int, int>::iterator it2;
		for (it1 = graph.begin(); it1 != graph.end(); it1++) {
			if (it1->second < 1)
				continue;
			it2 = _pointOrderNums.find(it1->first);
			if (it2 == _pointOrderNums.end())
				continue;
			it2->second = it2->second - it1->second;
			if (it2->second < 1)
				_pointOrderNums.erase(it2);
		}
	}

	void GuanDanSearchData::clear() {
		_damages = 0;
		_undamages = 0;
		_variableCards = 0;
		_pointOrderNums.clear();
	}

	GuanDanSearch::GuanDanSearch(int id)
		: GuanDanSearchData(id)
		, _genre(0)
		, _officerPointOrder(-1)
		, _straightMultiple(0)
		, _sameSuitNum(0)
		, _adopt(false)
		, _bad(false)
	{
		for (int i = 0; i < 3; i++)
			_sameSuits[i] = 0;
	}

	GuanDanSearch::~GuanDanSearch()
	{}

	int GuanDanSearch::getGenre() const {
		return _genre;
	}

	void GuanDanSearch::setGenre(int genre) {
		_genre = genre;
	}

	void GuanDanSearch::setOfficerPointOrder(int order) {
		_officerPointOrder = order;
	}

	int GuanDanSearch::getOfficerPointOrder() const {
		return _officerPointOrder;
	}

	int GuanDanSearch::getStraightMultiple() const {
		return _straightMultiple;
	}

	void GuanDanSearch::setStraightMultiple(int multiple) {
		_straightMultiple = multiple;
	}

	int GuanDanSearch::getSameSuitNum() const {
		return _sameSuitNum;
	}

	void GuanDanSearch::addSameSuit(int suit) {
		if (_sameSuitNum > 2)
			return;
		_sameSuits[_sameSuitNum] = suit;
		_sameSuitNum++;
	}

	int GuanDanSearch::getSameSuit(int idx) const {
		if (idx < 0 || idx >= _sameSuitNum)
			return -1;
		return _sameSuits[idx];
	}

	void GuanDanSearch::rollbackSameSuit(int idx) {
		for (int i = idx; i < 3; i++) {
			if (i < 2)
				_sameSuits[i] = _sameSuits[i + 1];
			else
				_sameSuits[i] = 0;
		}
		_sameSuitNum--;
	}

	void GuanDanSearch::setAdopt() {
		_adopt = true;
	}

	bool GuanDanSearch::getAdopt() const {
		return _adopt;
	}

	void GuanDanSearch::setBad() {
		_bad = true;
	}

	bool GuanDanSearch::isBad() const {
		return _bad;
	}

	void GuanDanSearch::clear() {
		GuanDanSearchData::clear();

		_genre = 0;
		_officerPointOrder = -1;
		_straightMultiple = 0;
		_sameSuitNum = 0;
		for (int i = 0; i < 3; i++)
			_sameSuits[i] = 0;
		_adopt = false;
		_bad = false;
	}
}