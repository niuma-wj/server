// NiuNiuRule.cpp

#include "NiuNiuRule.h"

namespace NiuMa
{
	class NiuNiuCardOrderTable : public CardOrderTable
	{
	public:
		NiuNiuCardOrderTable()
		{
			_pointOrders[0] = static_cast<int>(PokerPoint::Ace);
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
			_pointOrders[13] = static_cast<int>(PokerPoint::Joker);
		}

		virtual ~NiuNiuCardOrderTable()
		{}
	};

	const int NiuNiuRule::GENRE_ORDER_NIU100[13] =
	{
		static_cast<int>(NiuNiuGenre::Niu0),
		static_cast<int>(NiuNiuGenre::Niu1),
		static_cast<int>(NiuNiuGenre::Niu2),
		static_cast<int>(NiuNiuGenre::Niu3),
		static_cast<int>(NiuNiuGenre::Niu4),
		static_cast<int>(NiuNiuGenre::Niu5),
		static_cast<int>(NiuNiuGenre::Niu6),
		static_cast<int>(NiuNiuGenre::Niu7),
		static_cast<int>(NiuNiuGenre::Niu8),
		static_cast<int>(NiuNiuGenre::Niu9),
		static_cast<int>(NiuNiuGenre::NiuNiu),
		static_cast<int>(NiuNiuGenre::ZhaDan),
		static_cast<int>(NiuNiuGenre::WuHua)
	};

	NiuNiuRule::NiuNiuRule(bool niu100)
		: _niu100(niu100)
	{
		_orderTable = new NiuNiuCardOrderTable();
	}

	NiuNiuRule::~NiuNiuRule()
	{}

	int NiuNiuRule::predicateCardGenre(PokerGenre& pcg) const
	{
		if (_niu100)
			return predicateCardGenre2(pcg);
		else
			return predicateCardGenre1(pcg);
	}

	int NiuNiuRule::predicateCardGenre1(PokerGenre& pcg) const {
		if (pcg.getCardNums() != 5)
			return static_cast<int>(NiuNiuGenre::Invalid);

		int arrPoints[5] = { 0 };
		int nJokers = 0;
		int nSum = 0;
		bool wuHua = true;
		CardArray& cards = pcg.getCards();
		int pt = static_cast<int>(PokerPoint::Invalid);
		for (unsigned int i = 0; i < 5; i++) {
			const PokerCard& c = cards[i];
			pt = c.getPoint();
			if (pt < static_cast<int>(PokerPoint::Jack)) {
				arrPoints[i] = pt;
				wuHua = false;
			}
			else if (pt < static_cast<int>(PokerPoint::Joker))
				arrPoints[i] = 10;
			else {
				wuHua = false;
				if (pt == static_cast<int>(PokerPoint::Joker))
					nJokers++;
			}
		}
		for (unsigned int i = 5; i > 0; i--) {
			const PokerCard& c = cards[i - 1];
			if (c.getPoint() != static_cast<int>(PokerPoint::Joker)) {
				pcg.setOfficer(c);
				break;
			}
		}
		// 先判定特殊牌型
		bool bShunZi = straight(pcg.getCards());
		bool bTongHua = pcg.sameSuit();
		bool bTest = true;
		if (bShunZi && bTongHua)	// 同花顺，开心牛
			return static_cast<int>(NiuNiuGenre::KaiXin);
		if (nJokers == 0) {
			for (unsigned int i = 0; i < 5; i++) {
				if (arrPoints[i] > 4) {
					bTest = false;
					break;
				}
				nSum += arrPoints[i];
				if (nSum > 10) {
					bTest = false;
					break;
				}
			}
			if (bTest)			// 五小牛
				return static_cast<int>(NiuNiuGenre::WuXiao);
		}
		if (pcg.carryM_N(4, 1))	// 炸弹牛
			return static_cast<int>(NiuNiuGenre::ZhaDan);
		if (pcg.carryM_N(3, 2))	// 葫芦牛
			return static_cast<int>(NiuNiuGenre::HuLu);
		if (bTongHua)			// 同花牛
			return static_cast<int>(NiuNiuGenre::TongHua);
		if (wuHua)				// 五花牛
			return static_cast<int>(NiuNiuGenre::WuHua);
		if (bShunZi)
			return static_cast<int>(NiuNiuGenre::ShunZi);		// 顺子牛
		if (nJokers == 2)		// 有两个王牌，必定是牛牛
			return static_cast<int>(NiuNiuGenre::NiuNiu);
		unsigned int ids[5] = { 0, 0, 0, 0, 0 };
		unsigned int tmp = 0;
		int nRemainder = 0;
		bool bNiu = false;
		for (; ids[0] < 3; ids[0]++) {
			for (ids[1] = ids[0] + 1; ids[1] < 4; ids[1]++) {
				for (ids[2] = ids[1] + 1; ids[2] < 5; ids[2]++) {
					nSum = arrPoints[ids[0]] + arrPoints[ids[1]] + arrPoints[ids[2]];
					if ((nSum % 10) == 0) {
						bNiu = true;
						break;
					}
				}
				if (bNiu)
					break;
			}
			if (bNiu)
				break;
		}
		if (!bNiu) {
			if (nJokers == 0)	// 没有王牌，则必定是没牛
				return static_cast<int>(NiuNiuGenre::Niu0);
			else {
				// 有一张王牌，执行到这里说明任意两张或者三张非王牌牌牌值之和都不是10的整数
				// 倍(因王牌牌值此前为0故可得此结论)。查找最大的牌型，这里只需要找牌值之和除
				// 以10的余数最大的两张非王牌，这个最大的余数就对应了牌型，因为另外两张非王
				// 牌与王牌组合起来必定是牛
				bTest = true;
				for (unsigned int i = 0; i < 4; i++) {
					if (arrPoints[i] == 0)
						continue;
					for (unsigned int j = i + 1; j < 5; i++) {
						if (arrPoints[j] == 0)
							continue;
						nSum = (arrPoints[i] + arrPoints[j]) % 10;
						if (bTest) {
							bTest = false;
							nRemainder = nSum;
							ids[3] = i;
							ids[4] = j;
						}
						else if (nRemainder < nSum) {
							nRemainder = nSum;
							ids[3] = i;
							ids[4] = j;
						}
					}
				}
				for (unsigned int i = 0; i < 5; i++) {
					if (i != ids[3] && i != ids[4]) {
						ids[tmp] = i;
						tmp++;
					}
				}
			}
		}
		else {
			tmp = 0;
			for (unsigned int i = 0; i < 5; i++) {
				if (i != ids[0] && i != ids[1] && i != ids[2]) {
					ids[3 + tmp] = i;
					tmp++;
				}
			}
			if (nJokers == 1) {
				// 有一张王牌，则必定是牛牛
				bTest = true;
				if (arrPoints[ids[0]] == 0)
					tmp = ids[0];
				else if (arrPoints[ids[1]] == 0)
					tmp = ids[1];
				else if (arrPoints[ids[2]] == 0)
					tmp = ids[2];
				else
					bTest = false;
				if (bTest) {
					ids[0] = ids[3];
					ids[1] = ids[4];
					ids[2] = tmp;
					tmp = 0;
					for (unsigned int i = 0; i < 5; i++) {
						if (i != ids[0] && i != ids[1] && i != ids[2]) {
							ids[3 + tmp] = i;
							tmp++;
						}
					}
				}
				nRemainder = 0;
			}
			else
				nRemainder = (arrPoints[ids[3]] + arrPoints[ids[4]]) % 10;
		}
		// 重新排列
		CardArray arrCards;
		arrCards.reserve(5);
		arrCards.push_back(cards[ids[3]]);
		arrCards.push_back(cards[ids[4]]);
		arrCards.push_back(cards[ids[0]]);
		arrCards.push_back(cards[ids[1]]);
		arrCards.push_back(cards[ids[2]]);
		cards = arrCards;
		if (nRemainder == 0)
			return static_cast<int>(NiuNiuGenre::NiuNiu);
		else
			return getNotNiuGenre(nRemainder);
	}

	int NiuNiuRule::predicateCardGenre2(PokerGenre& pcg) const {
		if (pcg.carryM_N(4, 1))	// 炸弹牛
			return static_cast<int>(NiuNiuGenre::ZhaDan);
		int arrPoints[5] = { 0 };
		int nSum = 0;
		bool test = true;
		CardArray& cards = pcg.getCards();
		int pt = static_cast<int>(PokerPoint::Invalid);
		for (unsigned int i = 0; i < 5; i++) {
			const PokerCard& c = cards[i];
			pt = c.getPoint();
			if (pt < static_cast<int>(PokerPoint::Jack)) {
				arrPoints[i] = pt;
				test = false;
			}
			else if (pt < static_cast<int>(PokerPoint::Joker))
				arrPoints[i] = 10;
			else
				test = false;
		}
		pcg.setOfficer(cards[4]);
		if (test)			// 五花牛
			return static_cast<int>(NiuNiuGenre::WuHua);
		unsigned int ids[5] = { 0, 0, 0, 0, 0 };
		unsigned int tmp = 0;
		int nRemainder = 0;
		bool bNiu = false;
		for (; ids[0] < 3; ids[0]++) {
			for (ids[1] = ids[0] + 1; ids[1] < 4; ids[1]++) {
				for (ids[2] = ids[1] + 1; ids[2] < 5; ids[2]++) {
					nSum = arrPoints[ids[0]] + arrPoints[ids[1]] + arrPoints[ids[2]];
					if ((nSum % 10) == 0) {
						bNiu = true;
						break;
					}
				}
				if (bNiu)
					break;
			}
			if (bNiu)
				break;
		}
		if (!bNiu)
			return static_cast<int>(NiuNiuGenre::Niu0);	// 没牛
		tmp = 0;
		for (unsigned int i = 0; i < 5; i++) {
			if (i != ids[0] && i != ids[1] && i != ids[2]) {
				ids[3 + tmp] = i;
				tmp++;
			}
		}
		nRemainder = (arrPoints[ids[3]] + arrPoints[ids[4]]) % 10;
		// 重新排列
		CardArray arrCards;
		arrCards.reserve(5);
		arrCards.push_back(cards[ids[0]]);
		arrCards.push_back(cards[ids[1]]);
		arrCards.push_back(cards[ids[2]]);
		arrCards.push_back(cards[ids[3]]);
		arrCards.push_back(cards[ids[4]]);
		cards = arrCards;
		if (nRemainder == 0)
			return static_cast<int>(NiuNiuGenre::NiuNiu);
		else
			return getNotNiuGenre(nRemainder);
	}

	int NiuNiuRule::compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const {
		int genre1 = pcg1.getGenre();
		int genre2 = pcg2.getGenre();
		if (_niu100) {
			int order1 = getGenreOrderNiu100(genre1);
			int order2 = getGenreOrderNiu100(genre2);
			if (order1 > order2)
				return 1;
			else if (order1 < order2)
				return 2;
		}
		else {
			if (genre1 > genre2)
				return 1;
			else if (genre1 < genre2)
				return 2;
		}
		return compareCard(pcg1.getOfficer(), pcg2.getOfficer());
	}

	bool NiuNiuRule::straightExcluded(const PokerCard& c) const {
		// 顺子不能出现王
		if (c.getPoint() == static_cast<int>(PokerPoint::Joker))
			return true;

		return PokerRule::straightExcluded(c);
	}

	int NiuNiuRule::getNotNiuGenre(int nRemainder) const {
		if (nRemainder == 1)
			return static_cast<int>(NiuNiuGenre::Niu1);
		else if (nRemainder == 2)
			return static_cast<int>(NiuNiuGenre::Niu2);
		else if (nRemainder == 3)
			return static_cast<int>(NiuNiuGenre::Niu3);
		else if (nRemainder == 4)
			return static_cast<int>(NiuNiuGenre::Niu4);
		else if (nRemainder == 5)
			return static_cast<int>(NiuNiuGenre::Niu5);
		else if (nRemainder == 6)
			return static_cast<int>(NiuNiuGenre::Niu6);
		else if (nRemainder == 7)
			return static_cast<int>(NiuNiuGenre::Niu7);
		else if (nRemainder == 8)
			return static_cast<int>(NiuNiuGenre::Niu8);
		else if (nRemainder == 9)
			return static_cast<int>(NiuNiuGenre::Niu9);

		return static_cast<int>(NiuNiuGenre::Invalid);
	}

	int NiuNiuRule::getGenreOrderNiu100(int genre) {
		for (int i = 0; i < 13; i++) {
			if (GENRE_ORDER_NIU100[i] == genre)
				return i;
		}
		return -1;
	}
}