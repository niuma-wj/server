// PokerCardGenre.cpp

#include "PokerGenre.h"
#include "PokerRule.h"
#include "PokerUtilities.h"

#include <algorithm>

namespace NiuMa
{
	const PokerGenre PokerGenre::NullGenre;

	PokerGenre::PokerGenre()
		: genre(-1)
	{}

	PokerGenre::PokerGenre(const PokerGenre& pcg)
		: genre(pcg.getGenre())
		, officer(pcg.getOfficer())
		, mate(pcg.getMate())
		, cards(pcg.getCards())
	{}

	PokerGenre::~PokerGenre()
	{}

	int PokerGenre::getGenre() const {
		return genre;
	}

	void PokerGenre::setGenre(int g) {
		genre = g;
	}

	int PokerGenre::getCardNums() const {
		return static_cast<int>(cards.size());
	}

	const CardArray& PokerGenre::getCards() const {
		return cards;
	}

	CardArray& PokerGenre::getCards() {
		return cards;
	}

	bool PokerGenre::hasPoint(int point) const {
		return PokerUtilities::hasPoint(cards, point);
	}

	bool PokerGenre::hasSuit(int suit) const {
		return PokerUtilities::hasSuit(cards, suit);
	}

	bool PokerGenre::hasCard(int point, int suit) const {
		return PokerUtilities::hasCard(cards, point, suit);
	}

	const PokerCard& PokerGenre::getOfficer() const {
		return officer;
	}

	void PokerGenre::setOfficer(const PokerCard& c) {
		officer = c;
	}

	const PokerCard& PokerGenre::getMate() const {
		return mate;
	}

	void PokerGenre::setMate(const PokerCard& c) {
		mate = c;
	}

	void PokerGenre::setCards(const CardArray& cards_, const std::shared_ptr<PokerRule>& rule, int genreIn) {
		cards = cards_;

		// 设置后强制排序
		std::sort(cards.begin(), cards.end(), CardComparator(rule));
		if (genreIn != -1)
			genre = genreIn;
		else if (rule)
			genre = rule->predicateCardGenre(*this);
	}

	void PokerGenre::clear() {
		cards.clear();
		officer = PokerCard();
		mate = PokerCard();

		genre = -1;
	}

	void PokerGenre::card2String(std::string& str) const {
		PokerUtilities::cardArray2String(cards, str);
	}

	PokerGenre& PokerGenre::operator=(const PokerGenre& pcg) {
		genre = pcg.getGenre();
		officer = pcg.getOfficer();
		mate = pcg.getMate();
		cards = pcg.getCards();
		
		return *this;
	}

	bool PokerGenre::operator==(const PokerGenre& pcg) const {
		const std::vector<PokerCard>& cards_ = pcg.getCards();
		if (cards.size() != cards_.size())
			return false;

		unsigned int nums = static_cast<unsigned int>(cards.size());
		for (unsigned int i = 0; i < nums; i++) {
			// 因为设置后的牌肯定是经过排序的，所以这里可以直接对比
			if (cards[i] != cards_[i])
				return false;
		}
		return true;
	}

	bool PokerGenre::operator!=(const PokerGenre& pcg) const {
		return !(operator==(pcg));
	}

	int PokerGenre::getFirstPoint() const {
		if (cards.empty())
			return static_cast<int>(PokerPoint::Invalid);

		const PokerCard& card = cards.at(0);
		return card.getPoint();
	}

	int PokerGenre::getFirstSuit() const {
		if (cards.empty())
			return static_cast<int>(PokerSuit::Invalid);

		const PokerCard& card = cards.at(0);
		return card.getSuit();
	}

	bool PokerGenre::samePoint(int ignorePoint, int ignoreSuit) const {
		return PokerUtilities::samePoint(cards, ignorePoint, ignoreSuit);
	}

	bool PokerGenre::sameSuit(int ignorePoint, int ignoreSuit) const {
		return PokerUtilities::sameSuit(cards, ignorePoint, ignoreSuit);
	}

	bool PokerGenre::carryM_N(int M_, int N_) const {
		if (static_cast<int>(cards.size()) != (M_ + N_))
			return false;

		unsigned int nums = static_cast<unsigned int>(M_ + N_);
		int nums1 = 1;
		int nums2 = 0;
		const int pt1 = cards.at(0).getPoint();
		int pt2 = static_cast<int>(PokerPoint::Invalid);
		for (unsigned int i = 1; i < nums; i++) {
			const PokerCard& c = cards.at(i);
			if (pt1 != c.getPoint()) {
				if (nums2 == 0) {
					// 开始出现第二种牌
					nums2 = 1;
					pt2 = c.getPoint();
				}
				else if (pt2 == c.getPoint())
					nums2++;
			}
			else
				nums1++;
		}
		if ((nums1 == M_ && nums2 == N_) || (nums1 == N_ && nums2 == M_))
			return true;

		return false;
	}
}