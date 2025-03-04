// PokerDealer.cpp

#include "Base/Log.h"
#include "PokerDealer.h"
#include "PokerRule.h"
#include "Base/BaseUtils.h"

#include <list>
#include <stdexcept>

namespace NiuMa
{
	// 对外隐藏牌池的定义，以免牌池被在不适合的时机改动
	class PokerCardPool
	{
	public:
		PokerCardPool(const CardArray& allCards)
			: _allCards(allCards)
		{}

		virtual ~PokerCardPool()
		{}

		typedef std::shared_ptr<PokerCardPool> Ptr;

	private:
		const CardArray _allCards;

	public:
		int getSize() const {
			return static_cast<int>(_allCards.size());
		}

		PokerCard getCard(int index) const {
			if (index < 0 || index >= static_cast<int>(_allCards.size())) {
				throw std::out_of_range("The input index exceed the actual size of card pool.");
				return PokerCard();
			}
			return _allCards.at(index);
		}

		void getCard(PokerCard& c, int index) const {
			if (index < 0 || index >= static_cast<int>(_allCards.size()))
				throw std::out_of_range("The input index exceed the actual size of card pool.");
			else
				c = _allCards.at(index);
		}

		bool getFirstCard(PokerCard& c) const {
			CardArray::const_iterator it = _allCards.begin();
			while (it != _allCards.end()) {
				if (*it == c) {
					c = *it;
					return true;
				}
				++it;
			}
			return false;
		}
	};

	DealFilter::DealFilter()
	{}

	DealFilter::~DealFilter()
	{}

	PokerDealer::PokerDealer(const PokerRule::Ptr& rule)
		: _packNums(rule->getPackNums())
		, _rule(rule)
		, _cardPool(NULL)
	{
		initializeCardPool();
	}

	PokerDealer::~PokerDealer()
	{}

	void PokerDealer::initializeCardPool()
	{
		int id = 0;
		CardArray allCards;
		// 每副牌有54张
		allCards.reserve(_packNums * 54);
		for (int i = static_cast<int>(PokerPoint::Ace); i < static_cast<int>(PokerPoint::Joker); i++) {
			for (int j = static_cast<int>(PokerSuit::Diamond); j < static_cast<int>(PokerSuit::Little); j++) {
				for (int k = 0; k < _packNums; k++) {
					PokerCard c(static_cast<PokerPoint>(i), static_cast<PokerSuit>(j), id);
					if (!(_rule->isDisapprovedCard(c))) {
						allCards.push_back(c);
						id++;
					}
				}
			}
		}
		// 填入大小王
		for (int i = static_cast<int>(PokerSuit::Little); i < static_cast<int>(PokerSuit::Total); i++) {
			for (int j = 0; j < _packNums; j++) {
				PokerCard c(PokerPoint::Joker, static_cast<PokerSuit>(i), id);
				if (!(_rule->isDisapprovedCard(c))) {
					allCards.push_back(c);
					id++;
				}
			}
		}
		_cardPool = std::make_shared<PokerCardPool>(allCards);
		shuffle();
	}

	int PokerDealer::getPackNums() const {
		return _packNums;
	}

	int PokerDealer::getCardPoolSize() const {
		if (!_cardPool)
			return 0;

		return _cardPool->getSize();
	}

	int PokerDealer::getCardLeft() const {
		return static_cast<int>(_indicesLeft.size());
	}

	void PokerDealer::getCard(PokerCard& c, int id) const {
		if (_cardPool)
			_cardPool->getCard(c, id);
	}

	bool PokerDealer::getFirstCard(PokerCard& c) const {
		if (_cardPool)
			return _cardPool->getFirstCard(c);

		return false;
	}

	void PokerDealer::shuffle() {
		int poolSize = getCardPoolSize();
		if (poolSize == 0)
			return;
		_indicesLeft.clear();
#if 1
		// 随机从索引列表里面获取索引
		int index = 0;
		int size = 0;
		std::list<int>::iterator it;
		std::list<int> lstIndies;
		for (int i = 0; i < poolSize; i++)
			lstIndies.push_back(i);
		//std::string prefix("Random index: ");
		//std::string text;
		for (int i = 0; i < poolSize; i++) {
			size = static_cast<int>(lstIndies.size());
			if (size == 1) {
				index = lstIndies.front();
				lstIndies.clear();
			}
			else {
				index = BaseUtils::randInt(0, static_cast<int>(size));
				//text = prefix + std::to_string(index);
				//LOG_DEBUG(text);
				it = lstIndies.begin();
				for (int j = 0; j < index; j++)
					it++;
				index = *it;
				lstIndies.erase(it);
			}
			_indicesLeft.push_back(index);
		}
#else
		int tmp = 0;
		int pos = 0;
		int index = 0;
		while (pos < poolSize)  {
			for (int i = 0; i < 13; i++) {
				index = i * 12 + (tmp * 2 + 0);
				_indicesLeft.push_back(index);
				index = i * 12 + (tmp * 2 + 1);
				_indicesLeft.push_back(index);
			}
			index = 13 * 12 + tmp;
			_indicesLeft.push_back(index);
			tmp++;
		}
#endif
	}

	bool PokerDealer::handOutCards(std::vector<CardArray>& cardHeaps, int headNums) {	
		if (headNums == 0)
			return false;
		if (headNums > getCardLeft())
			return false;

		cardHeaps.clear();
		cardHeaps.reserve(headNums);
		for (int i = 0; i < headNums; i++)
			cardHeaps.push_back(CardArray());

		int heap = 0;
		int index = 0;
		while (!_indicesLeft.empty()) {
			CardArray& arrCards = cardHeaps.at(heap++);
			if (heap == headNums)
				heap = 0;

			index = _indicesLeft.front();
			_indicesLeft.pop_front();
			arrCards.push_back(_cardPool->getCard(index));
		}
		return true;
	}

	bool PokerDealer::handOutCards(CardArray& cards, int nums, const DealFilter::Ptr& filter) {
		if (nums == 0)
			return false;
		if (nums > getCardLeft())
			return false;

		cards.clear();
		cards.reserve(nums);
		PokerCard c;
		bool test = false;
		int index = 0;
		std::list<int>::const_iterator it;
		for (int i = 0; i < nums; i++) {
			if (filter) {
				test = false;
				it = _indicesLeft.begin();
				while (it != _indicesLeft.end()) {
					index = *it;
					c = _cardPool->getCard(index);
					if (filter->isOk(c)) {
						_indicesLeft.erase(it);
						test = true;
						break;
					}
					++it;
				}
				if (!test)
					return false;
			}
			else
			{
				index = _indicesLeft.front();
				_indicesLeft.pop_front();
				c = _cardPool->getCard(index);
			}
			cards.push_back(c);
		}
		return true;
	}

	bool PokerDealer::handOutCard(PokerCard& c, const DealFilter::Ptr& filter) {
		if (_indicesLeft.empty())
			return false;
		int index = 0;
		if (filter) {
			bool test = false;
			std::list<int>::const_iterator it = _indicesLeft.begin();
			while (it != _indicesLeft.end()) {
				index = *it;
				_cardPool->getCard(c, index);
				if (filter->isOk(c)) {
					_indicesLeft.erase(it);
					test = true;
					break;
				}
				++it;
			}
			if (!test)
				return false;
		}
		else {
			index = _indicesLeft.front();
			_indicesLeft.pop_front();
			_cardPool->getCard(c, index);
		}
		return true;
	}
}