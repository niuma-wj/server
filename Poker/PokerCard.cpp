// PokerCard.cpp

#include "PokerCard.h"

namespace NiuMa
{
	PokerCard::PokerCard(PokerPoint point_, PokerSuit suit_, int id_)
		: point(static_cast<int>(point_))
		, suit(static_cast<int>(suit_))
		, id(id_)
	{}

	PokerCard::PokerCard(const PokerCard& c)
		: point(c.getPoint())
		, suit(c.getSuit())
		, id(c.getId())
	{}

	PokerCard::~PokerCard()
	{}

	int PokerCard::getId() const {
		return id;
	}

	int PokerCard::getPoint() const {
		return point;
	}

	void PokerCard::setPoint(int p) {
		point = p;
	}

	int PokerCard::getSuit() const {
		return suit;
	}

	void PokerCard::setSuit(int s) {
		suit = s;
	}

	bool PokerCard::isValid() const {
		if (point == static_cast<int>(PokerPoint::Invalid) || suit == static_cast<int>(PokerSuit::Invalid))
			return false;

		return true;
	}

	void PokerCard::toString(std::string& name) const {
		PokerSuit eSuit = static_cast<PokerSuit>(suit);
		switch (eSuit) {
		case PokerSuit::Diamond:
			name = "方块";
			break;
		case PokerSuit::Club:
			name = "梅花";
			break;
		case PokerSuit::Heart:
			name = "红桃";
			break;
		case PokerSuit::Spade:
			name = "黑桃";
			break;
		case PokerSuit::Little:
			name = "小";
			break;
		case PokerSuit::Big:
			name = "大";
			break;
		default:
			break;
		}
		PokerPoint ePoint = static_cast<PokerPoint>(point);
		switch (ePoint) {
		case PokerPoint::Ace:
			name += "A";
			break;
		case PokerPoint::Two:
			name += "2";
			break;
		case PokerPoint::Three:
			name += "3";
			break;
		case PokerPoint::Four:
			name += "4";
			break;
		case PokerPoint::Five:
			name += "5";
			break;
		case PokerPoint::Six:
			name += "6";
			break;
		case PokerPoint::Seven:
			name += "7";
			break;
		case PokerPoint::Eight:
			name += "8";
			break;
		case PokerPoint::Nine:
			name += "9";
			break;
		case PokerPoint::Ten:
			name += "10";
			break;
		case PokerPoint::Jack:
			name += "J";
			break;
		case PokerPoint::Queen:
			name += "Q";
			break;
		case PokerPoint::King:
			name += "K";
			break;
		case PokerPoint::Joker:
			name += "王";
			break;
		default:
			break;
		}
	}

	int PokerCard::toInt32() const {
		int val = point;
		val <<= 8;
		val |= suit;
		return val;
	}

	void PokerCard::fromInt32(int val) {
		suit = (val & 0xff);
		point = val >> 8;
		point &= 0xff;
	}

	PokerCard& PokerCard::operator=(const PokerCard& c) {
		point = c.getPoint();
		suit = c.getSuit();
		id = c.getId();

		return *this;
	}

	bool PokerCard::operator==(const PokerCard& c) const {
		if (point == c.getPoint() && suit == c.getSuit())
			return true;

		return false;
	}

	bool PokerCard::operator!=(const PokerCard& c) const {
		if (point != c.getPoint() || suit != c.getSuit())
			return true;

		return false;
	}

	void getCardIds(const CardArray& cards, std::vector<int>& ids) {
		ids.clear();

		ids.reserve(cards.size());
		CardArray::const_iterator it = cards.begin();
		while (it != cards.end()) {
			ids.push_back(it->getId());
			++it;
		}
	}

	bool PointComparator::operator()(int a, int b) const {
		return compareImpl(a, b);
	}
}