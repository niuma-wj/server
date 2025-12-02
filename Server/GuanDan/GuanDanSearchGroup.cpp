// GuanDanSearchGroup.cpp

#include "GuanDanSearchGroup.h"
#include "GuanDanRule.h"

namespace NiuMa
{
	GuanDanSearchGroup::GuanDanSearchGroup(int id)
		: GuanDanSearchData(id)
	{}

	GuanDanSearchGroup::~GuanDanSearchGroup()
	{}

	bool GuanDanSearchGroup::containsSearch(int id) const {
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it = _searches.find(id);
		return (it != _searches.end());
	}

	void GuanDanSearchGroup::addSearch(const GuanDanSearch::Ptr& search) {
		if (!search)
			return;
		_searches.insert(std::make_pair(search->getId(), search));
	}

	const std::unordered_map<int, GuanDanSearch::Ptr>& GuanDanSearchGroup::getSearches() const {
		return _searches;
	}

	void GuanDanSearchGroup::removeThreeWith2() {
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it = _searches.begin();
		while (it != _searches.end()) {
			const GuanDanSearch::Ptr& search = it->second;
			if (search->getGenre() == static_cast<int>(GuanDanGenre::ThreeWith2)) {
				subtractGraph(search->getPointOrderNums());
				it = _searches.erase(it);
			}
			else
				it++;
		}
	}

	void GuanDanSearchGroup::setAdopt() {
		std::unordered_map<int, GuanDanSearch::Ptr>::const_iterator it;
		for (it = _searches.begin(); it != _searches.end(); it++) {
			const GuanDanSearch::Ptr& search = it->second;
			search->setAdopt();
		}
	}

	void GuanDanSearchGroup::clear() {
		GuanDanSearchData::clear();

		_searches.clear();
	}
}