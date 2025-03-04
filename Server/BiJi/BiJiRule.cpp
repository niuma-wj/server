// BiJiRule.cpp

#include "PokerUtilities.h"
#include "BiJiRule.h"

namespace NiuMa
{
	class BiJiCardOrderTable : public CardOrderTable
	{
	public:
		BiJiCardOrderTable()
		{
			_pointOrders[0] = static_cast<int>(PokerPoint::Joker);
			_pointOrders[1] = static_cast<int>(PokerPoint::Two);
			_pointOrders[2] = static_cast<int>(PokerPoint::Three);
			_pointOrders[3] = static_cast<int>(PokerPoint::Four);
			_pointOrders[4] = static_cast<int>(PokerPoint::Five);
			_pointOrders[5] = static_cast<int>(PokerPoint::Six);
			_pointOrders[6] = static_cast<int>(PokerPoint::Seven);
			_pointOrders[7] = static_cast<int>(PokerPoint::Eight);
			_pointOrders[8] = static_cast<int>(PokerPoint::Nine);
			_pointOrders[9] = static_cast<int>(PokerPoint::Ten);
			_pointOrders[10] = static_cast<int>(PokerPoint::Jack);
			_pointOrders[11] = static_cast<int>(PokerPoint::Queen);
			_pointOrders[12] = static_cast<int>(PokerPoint::King);
			_pointOrders[13] = static_cast<int>(PokerPoint::Ace);
		}

		virtual ~BiJiCardOrderTable()
		{}
	};

	BiJiRule::BiJiRule()
	{
		_orderTable = new BiJiCardOrderTable();
	}

	BiJiRule::~BiJiRule()
	{}

	bool isStraight(const int arrNums[]) {
		if (arrNums == NULL)
			return false;
		int arrTemp[14] = { 0 };
		for (int i = 0; i < 14; i++) {
			if (arrNums[i] > 1)
				return false;
			arrTemp[i] = arrNums[i];
		}
		arrTemp[0] = arrNums[13];
		int cnt = 0;
		for (int i = 0; i < 14; i++) {
			if (arrTemp[i] != 0) {
				cnt++;
				if (cnt > 2)
					return true;
			}
			else
				cnt = 0;
		}
		return false;
	}

	int BiJiRule::predicateCardGenre(PokerGenre& pcg) const {
		// 比鸡只能三张牌组合成一首牌
		if (pcg.getCardNums() != 3)
			return static_cast<int>(BiJiGenre::Invalid);

		CardArray& cards = pcg.getCards();
		pcg.setOfficer(cards.back());
		if (pcg.samePoint())
			return static_cast<int>(BiJiGenre::Triple);	// 三张
		int arrNums[14] = { 0 };
		unsigned int nums = static_cast<unsigned int>(cards.size());
		int nOrder = 0;
		for (unsigned int i = 0; i < nums; i++) {
			const PokerCard& c = cards[i];
			nOrder = _orderTable->getPointOrder(c.getPoint());
			if (nOrder < 0 || nOrder > 13)
				return static_cast<int>(BiJiGenre::Invalid);
			arrNums[nOrder]++;
		}
		int cs0 = cards[0].getSuit();
		bool bSuit = pcg.sameSuit();
		if (!bSuit) {
			int cs1 = cards[1].getSuit();
			int cs2 = cards[2].getSuit();
			if (cs0 == static_cast<int>(PokerSuit::Big)) {
				if ((cs1 == static_cast<int>(PokerSuit::Heart) && cs2 == static_cast<int>(PokerSuit::Heart)) ||
					(cs1 == static_cast<int>(PokerSuit::Diamond) && cs2 == static_cast<int>(PokerSuit::Diamond)))
					bSuit = true;
			}
			else if (cs0 == static_cast<int>(PokerSuit::Little)) {
				if ((cs1 == static_cast<int>(PokerSuit::Spade) && cs2 == static_cast<int>(PokerSuit::Spade)) ||
					(cs1 == static_cast<int>(PokerSuit::Club) && cs2 == static_cast<int>(PokerSuit::Club)))
					bSuit = true;
			}
		}
		if ((cs0 != static_cast<int>(PokerSuit::Big)) &&
			(cs0 != static_cast<int>(PokerSuit::Little)) &&
			isStraight(arrNums)) {
			int ct0 = cards[0].getPoint();
			int ct1 = cards[1].getPoint();
			int ct2 = cards[2].getPoint();
			if ((ct0 == static_cast<int>(PokerPoint::Two)) &&
				(ct1 == static_cast<int>(PokerPoint::Three)) &&
				(ct2 == static_cast<int>(PokerPoint::Ace))) {
				// A23顺子最小
				PokerCard c = cards[2];
				cards[2] = cards[1];
				cards[1] = cards[0];
				cards[0] = c;
				pcg.setOfficer(c);
			}
			if (bSuit)
				return static_cast<int>(BiJiGenre::FlushStraight);	// 同花顺
			else
				return static_cast<int>(BiJiGenre::Straight);	// 顺子
		}
		for (int i = 0; i < 14; i++) {
			if (arrNums[i] > 1) {
				PokerCard c;
				PokerUtilities::getLastCardOfPoint(cards, c, _orderTable->getPoint(i));
				pcg.setOfficer(c);
				return static_cast<int>(BiJiGenre::Pair);	// 对子
			}
		}
		if (bSuit)
			return static_cast<int>(BiJiGenre::Flush);	// 同花
		return static_cast<int>(BiJiGenre::Single);	// 单张(乌龙)
	}

	int BiJiRule::compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const {
		const int arrGernres[6] = {
			static_cast<int>(BiJiGenre::Single),				// 单张(乌龙)
			static_cast<int>(BiJiGenre::Pair),					// 对子
			static_cast<int>(BiJiGenre::Straight),				// 顺子
			static_cast<int>(BiJiGenre::Flush),					// 同花
			static_cast<int>(BiJiGenre::FlushStraight),			// 同花顺
			static_cast<int>(BiJiGenre::Triple)					// 三张
		};
		int nPos1 = -1;
		int nPos2 = -1;
		int genre1 = pcg1.getGenre();
		int genre2 = pcg2.getGenre();
		for (int i = 0; i < 6; i++) {
			if (genre1 == arrGernres[i])
				nPos1 = i;
			if (genre2 == arrGernres[i])
				nPos2 = i;
		}
		if (nPos1 > nPos2)
			return 1;
		else if (nPos1 < nPos2)
			return 2;
		else if (nPos1 == -1)
			return 0;
		// 牌型相同
		if (nPos1 == 1) {
			// 对子的情况较特殊，大小王是最大的对子
			const PokerCard& c1 = pcg1.getOfficer();
			const PokerCard& c2 = pcg2.getOfficer();
			if (c1.getPoint() == static_cast<int>(PokerPoint::Joker))
				return 1;
			else if (c2.getPoint() == static_cast<int>(PokerPoint::Joker))
				return 2;
			if (c1.getPoint() != c2.getPoint()) {
				int nRet = compareCard(c1, c2);
				if (nRet == 1)
					return 1;
				else if (nRet == 2)
					return 2;
			}
		}
		const CardArray& card1 = pcg1.getCards();
		const CardArray& card2 = pcg2.getCards();
		for (unsigned int i = 0; i < 3; i++) {
			const PokerCard& c1 = card1[2 - i];
			const PokerCard& c2 = card2[2 - i];
			nPos1 = _orderTable->getPointOrder(c1.getPoint());
			nPos2 = _orderTable->getPointOrder(c2.getPoint());
			if (nPos1 > nPos2)
				return 1;
			else if (nPos1 < nPos2)
				return 2;
		}
		for (unsigned int i = 0; i < 3; i++) {
			const PokerCard& c1 = card1[2 - i];
			const PokerCard& c2 = card2[2 - i];
			nPos1 = _orderTable->getSuitOrder(c1.getSuit());
			nPos2 = _orderTable->getSuitOrder(c2.getSuit());
			if (nPos1 > nPos2)
				return 1;
			else if (nPos1 < nPos2)
				return 2;
		}
		return 0;
	}
}