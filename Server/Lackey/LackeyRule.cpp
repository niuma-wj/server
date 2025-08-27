// LackeyRule.cpp

#include "Base/Log.h"
#include "PokerUtilities.h"
#include "LackeyRule.h"

namespace NiuMa
{
	// 斗地主类的游戏存在一个难题即：自动出牌和提示组牌，这两者其实是同一个
	// 问题，都是自动出牌策略问题。任意牌型都是由一张或多张牌组成的组合，组
	// 合中牌的数量定义为组合张数，此外定义一个“破坏系数”概念，对任意组合
	// C，C中任意牌的点数p，假设手牌中点数为p的牌的总数为n，C中点数为p的牌的
	// 总数为m，若n > m：若手牌中点数为p的牌能组成炸弹，则p为C贡献的破坏系数
	// 为2，否则p为C贡献的的破坏系数为1，若n == m，则p为C贡献的破坏系数为0。
	// 组合中所有牌贡献的破坏系数总和即为组合的破坏系数，破坏系数及组合张数
	// 作为候选组合优先级的两个参考因素，破坏系数越低越优先级越高，相同破坏
	// 系数组合张数越小的优先级越高。对于狗腿炸，与其相同点数的牌的破坏系数
	// 相互独立。
	// 自动出牌优先策略：
	// A 当前非炸弹牌型：
	//	1、出更大且最小的同牌型，破坏系数为0。
	//	2、出更大且最小的炸弹牌型，破坏系数为0。
	//	3、出更大且最小的同牌型，破坏系数最小。
	// B 当前炸弹牌型：
	//	1、出更大且最小的炸弹牌型，破坏系数为0。

	const int LackeyRule::CANDIDATE_ORDERS[49] = {
		static_cast<int>(LackeyGenre::Single),
		static_cast<int>(LackeyGenre::Triple12),
		static_cast<int>(LackeyGenre::Butterfly7),
		static_cast<int>(LackeyGenre::Triple11),
		static_cast<int>(LackeyGenre::Butterfly6),
		static_cast<int>(LackeyGenre::Triple10),
		static_cast<int>(LackeyGenre::Triple9),
		static_cast<int>(LackeyGenre::Butterfly5),
		static_cast<int>(LackeyGenre::Triple8),
		static_cast<int>(LackeyGenre::Pair12),
		static_cast<int>(LackeyGenre::Pair11),
		static_cast<int>(LackeyGenre::Triple7),
		static_cast<int>(LackeyGenre::Butterfly4),
		static_cast<int>(LackeyGenre::Pair10),
		static_cast<int>(LackeyGenre::Triple6),
		static_cast<int>(LackeyGenre::Pair9),
		static_cast<int>(LackeyGenre::Pair8),
		static_cast<int>(LackeyGenre::Butterfly3),
		static_cast<int>(LackeyGenre::Triple5),
		static_cast<int>(LackeyGenre::Pair7),
		static_cast<int>(LackeyGenre::Triple4),
		static_cast<int>(LackeyGenre::Pair6),
		static_cast<int>(LackeyGenre::Butterfly2),
		static_cast<int>(LackeyGenre::Pair5),
		static_cast<int>(LackeyGenre::Triple3),
		static_cast<int>(LackeyGenre::Pair4),
		static_cast<int>(LackeyGenre::Triple2),
		static_cast<int>(LackeyGenre::Pair3),
		static_cast<int>(LackeyGenre::Butterfly1),
		static_cast<int>(LackeyGenre::Pair1),
		static_cast<int>(LackeyGenre::Triple1),
		static_cast<int>(LackeyGenre::Bomb4),			// 8
		static_cast<int>(LackeyGenre::Bomb5),			// 10
		static_cast<int>(LackeyGenre::Bomb3L),			// 6
		static_cast<int>(LackeyGenre::Bomb6),			// 12
		static_cast<int>(LackeyGenre::Bomb3B),			// 6
		static_cast<int>(LackeyGenre::Bomb7),			// 14
		static_cast<int>(LackeyGenre::Bomb2B2L),		// 8
		static_cast<int>(LackeyGenre::Bomb8),			// 16
		static_cast<int>(LackeyGenre::Bomb1B3L),		// 8
		static_cast<int>(LackeyGenre::Bomb9),			// 18
		static_cast<int>(LackeyGenre::Bomb3B1L),		// 8
		static_cast<int>(LackeyGenre::Bomb10),			// 20
		static_cast<int>(LackeyGenre::Bomb2B3L),		// 10
		static_cast<int>(LackeyGenre::Bomb11),			// 22
		static_cast<int>(LackeyGenre::Bomb3B2L),		// 10
		static_cast<int>(LackeyGenre::Bomb12),			// 24
		static_cast<int>(LackeyGenre::Bomb3B3L),		// 12
		static_cast<int>(LackeyGenre::BombLackey)		// 25
	};

	const int LackeyRule::BOMB_ORDERS[18] = {
		static_cast<int>(LackeyGenre::Bomb4),
		static_cast<int>(LackeyGenre::Bomb5),
		static_cast<int>(LackeyGenre::Bomb3L),
		static_cast<int>(LackeyGenre::Bomb6),
		static_cast<int>(LackeyGenre::Bomb3B),
		static_cast<int>(LackeyGenre::Bomb7),
		static_cast<int>(LackeyGenre::Bomb2B2L),
		static_cast<int>(LackeyGenre::Bomb8),
		static_cast<int>(LackeyGenre::Bomb1B3L),
		static_cast<int>(LackeyGenre::Bomb9),
		static_cast<int>(LackeyGenre::Bomb3B1L),
		static_cast<int>(LackeyGenre::Bomb10),
		static_cast<int>(LackeyGenre::Bomb2B3L),
		static_cast<int>(LackeyGenre::Bomb11),
		static_cast<int>(LackeyGenre::Bomb3B2L),
		static_cast<int>(LackeyGenre::Bomb12),
		static_cast<int>(LackeyGenre::Bomb3B3L),
		static_cast<int>(LackeyGenre::BombLackey)
	};

	const int LackeyRule::GENRE_CARD_NUMS[49] = { 1,	// 单张(1)
		2, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24,		// 连对(11)
		3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36,	// 三顺(12)
		5, 10, 15, 20, 25, 30, 35,						// 蝴蝶(7)
		4, 5, 6, 7, 8, 9, 10, 11, 12,					// N炸(9)
		3, 3, 4, 4, 4, 5, 5, 6,							// 王炸(8)
		1												// 狗腿炸(1)
	};

	LackeyRule::LackeyRule()
		: _defaultLackeyCard(-1)
	{
		_orderTable = new CardOrderTable();
	}

	LackeyRule::~LackeyRule()
	{}

	int LackeyRule::calcXiQian(int genre, const CardArray& cards) {
		int black = 0;
		int red = 0;
		int xiQian = 0;
		LackeyGenre eGenre = static_cast<LackeyGenre>(genre);
		switch (eGenre) {
		case LackeyGenre::Bomb5:
			PokerUtilities::getBlackRedNums(cards, black, red);
			if (black == 5 || red == 5)
				xiQian = 1;
			break;
		case LackeyGenre::Bomb6:
			PokerUtilities::getBlackRedNums(cards, black, red);
			if (black == 6 || red == 6)
				xiQian = 2;
			break;
		case LackeyGenre::Bomb7:
			PokerUtilities::getBlackRedNums(cards, black, red);
			if (black == 3 || red == 3)
				xiQian = 1;
			else if (black == 2 || red == 2)
				xiQian = 2;
			else if (black == 1 || red == 1)
				xiQian = 3;
			break;
		case LackeyGenre::Bomb8:
			PokerUtilities::getBlackRedNums(cards, black, red);
			if (black == 4 || red == 4)
				xiQian = 2;
			if (black == 3 || red == 3)
				xiQian = 3;
			if (black == 2 || red == 2)
				xiQian = 4;
			break;
		case LackeyGenre::Bomb9:
			PokerUtilities::getBlackRedNums(cards, black, red);
			if (black == 4 || red == 4)
				xiQian = 4;
			if (black == 3 || red == 3)
				xiQian = 5;
			break;
		case LackeyGenre::Bomb10:
			xiQian = 6;
			break;
		case LackeyGenre::Bomb11:
			xiQian = 8;
			break;
		case LackeyGenre::Bomb12:
			xiQian = 10;
			break;
			// 3王炸1倍
		case LackeyGenre::Bomb3L:
		case LackeyGenre::Bomb3B:
			xiQian = 1;
			break;
			// 4王炸2倍
		case LackeyGenre::Bomb2B2L:
		case LackeyGenre::Bomb1B3L:
		case LackeyGenre::Bomb3B1L:
			xiQian = 2;
			break;
			// 5王炸4倍
		case LackeyGenre::Bomb2B3L:
		case LackeyGenre::Bomb3B2L:
			xiQian = 4;
			break;
			// 6王炸6倍
		case LackeyGenre::Bomb3B3L:
			xiQian = 6;
			break;
		default:
			break;
		}
		return xiQian;
	}

	int LackeyRule::getPackNums() const {
		return 3;
	}

	int LackeyRule::predicateCardGenre(PokerGenre& pcg) const {
		const CardArray& cards = pcg.getCards();
		int genre = static_cast<int>(LackeyGenre::Invalid);
		int nums = pcg.getCardNums();
		int point = 0;
		int suit = 0;
		int big = 0;
		int little = 0;
		PokerCard c;
		std::string str;
		if (nums == 1) {
			genre = static_cast<int>(LackeyGenre::Single);
			pcg.setOfficer(cards[0]);
		}
		else if (nums == 2) {
			if (pcg.samePoint()) {
				point = pcg.getFirstPoint();
				// 大小王不能构成对子，两个小王或者两个大王则可以
				if ((point != static_cast<int>(PokerPoint::Joker)) || pcg.sameSuit()) {
					genre = static_cast<int>(LackeyGenre::Pair1);
					pcg.setOfficer(cards[1]);
				}
			}
		}
		else if (nums == 3) {
			if (pcg.samePoint()) {
				point = pcg.getFirstPoint();
				if (point == static_cast<int>(PokerPoint::Joker)) {
					if (pcg.sameSuit()) {
						suit = pcg.getFirstSuit();
						if (suit == static_cast<int>(PokerSuit::Little))
							genre = static_cast<int>(LackeyGenre::Bomb3L);
						else
							genre = static_cast<int>(LackeyGenre::Bomb3B);
					}
				}
				else {
					// 2小王1大王或1小王2大王不构成3张
					genre = static_cast<int>(LackeyGenre::Triple1);
					pcg.setOfficer(cards[2]);
				}
			}
		}
		else if (nums == 4) {
			if (pcg.samePoint()) {
				point = pcg.getFirstPoint();
				if (point == static_cast<int>(PokerPoint::Joker)) {
					PokerUtilities::getJokerSuitNums(cards, big, little);
					if (little == 1 && big == 3)
						genre = static_cast<int>(LackeyGenre::Bomb3B1L);
					else if (little == 3 && big == 1)
						genre = static_cast<int>(LackeyGenre::Bomb1B3L);
					else if (little == 2 && big == 2)
						genre = static_cast<int>(LackeyGenre::Bomb2B2L);
					else
						LOG_ERROR("4王炸牌型的花色数量不正确。");
				}
				else {
					genre = static_cast<int>(LackeyGenre::Bomb4);
					pcg.setOfficer(cards[3]);
				}
			}
		}
		else if (nums == 5) {
			if (pcg.samePoint()) {
				point = pcg.getFirstPoint();
				if (point == static_cast<int>(PokerPoint::Joker)) {
					PokerUtilities::getJokerSuitNums(cards, big, little);
					if (little == 2 && big == 3)
						genre = static_cast<int>(LackeyGenre::Bomb3B2L);
					else if (little == 3 && big == 2)
						genre = static_cast<int>(LackeyGenre::Bomb2B3L);
					else
						LOG_ERROR("5王炸牌型的花色数量不正确。");
				}
				else {
					genre = static_cast<int>(LackeyGenre::Bomb5);
					pcg.setOfficer(cards[4]);
				}
			}
			else if (pcg.carryM_N(3, 2)) {
				if (!PokerUtilities::hasSuit(cards, static_cast<int>(PokerSuit::Big)) ||
					!PokerUtilities::hasSuit(cards, static_cast<int>(PokerSuit::Little))) {
					// 对于3带2牌型，不能同时含有大王和小王两种花色
					if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
						genre = static_cast<int>(LackeyGenre::Butterfly1);
						pcg.setOfficer(c);
					}
					else {
						pcg.card2String(str);
						ErrorS << "3带二错误，牌数组: " << str;
					}
				}
			}
		}
		else if (nums == 6) {
			if (pcg.samePoint()) {
				point = pcg.getFirstPoint();
				if (point == static_cast<int>(PokerPoint::Joker))
					genre = static_cast<int>(LackeyGenre::Bomb3B3L);
				else
					genre = static_cast<int>(LackeyGenre::Bomb6);
			}
			else if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair3);
				pcg.setOfficer(cards[5]);
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple2);
				pcg.setOfficer(cards[5]);
			}
		}
		else if (nums == 7) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb7);
		}
		else if (nums == 8) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb8);
			else if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair4);
				pcg.setOfficer(cards[7]);
			}
		}
		else if (nums == 9) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb9);
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple3);
				pcg.setOfficer(cards[8]);
			}
		}
		else if (nums == 10) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb10);
			else if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair5);
				pcg.setOfficer(cards[9]);
			}
			else if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly2);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶2错误，牌数组: " << str;
				}
			}
		}
		else if (nums == 11) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb11);
		}
		else if (nums == 12) {
			if (pcg.samePoint())
				genre = static_cast<int>(LackeyGenre::Bomb12);
			else if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair6);
				pcg.setOfficer(cards[11]);
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple4);
				pcg.setOfficer(cards[11]);
			}
		}
		else if (nums == 14) {
			if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair7);
				pcg.setOfficer(cards[13]);
			}
		}
		else if (nums == 15) {
			if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly3);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶3错误，牌数组: " << str;
				}
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple5);
				pcg.setOfficer(cards[14]);
			}
		}
		else if (nums == 16) {
			if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair8);
				pcg.setOfficer(cards[15]);
			}
		}
		else if (nums == 18) {
			if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair9);
				pcg.setOfficer(cards[17]);
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple6);
				pcg.setOfficer(cards[17]);
			}
		}
		else if (nums == 20) {
			if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly4);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶4错误，牌数组: " << str;
				}
			}
			else if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair10);
				pcg.setOfficer(cards[19]);
			}
		}
		else if (nums == 21) {
			if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple7);
				pcg.setOfficer(cards[20]);
			}
		}
		else if (nums == 22) {
			if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair11);
				pcg.setOfficer(cards[21]);
			}
		}
		else if (nums == 24) {
			if (straightPair(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Pair12);
				pcg.setOfficer(cards[23]);
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple8);
				pcg.setOfficer(cards[23]);
			}
		}
		else if (nums == 25) {
			if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly5);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶5错误，牌数组: " << str;
				}
			}
		}
		else if (nums == 27) {
			if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple9);
				pcg.setOfficer(cards[26]);
			}
		}
		else if (nums == 30) {
			if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly6);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶6错误，牌数组: " << str;
				}
			}
			else if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple10);
				pcg.setOfficer(cards[29]);
			}
		}
		else if (nums == 33) {
			if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple11);
				pcg.setOfficer(cards[32]);
			}
		}
		else if (nums == 35) {
			if (butterfly(pcg.getCards())) {
				if (PokerUtilities::rfindSamePointN(cards, c, 3)) {
					genre = static_cast<int>(LackeyGenre::Butterfly7);
					pcg.setOfficer(c);
				}
				else {
					pcg.card2String(str);
					ErrorS << "蝴蝶7错误，牌数组: " << str;
				}
			}
		}
		else if (nums == 36) {
			if (straightTriple(pcg.getCards())) {
				genre = static_cast<int>(LackeyGenre::Triple12);
				pcg.setOfficer(cards[35]);
			}
		}
		return genre;
	}

	int LackeyRule::getGenreCardNums(int genre) const {
		if (genre < static_cast<int>(LackeyGenre::Single) || genre > static_cast<int>(LackeyGenre::BombLackey))
			return 0;
		int index = genre - static_cast<int>(LackeyGenre::Single);
		return GENRE_CARD_NUMS[index];
	}

	int LackeyRule::compareCard(const PokerCard& c1, const PokerCard& c2) const {
		int point1 = c1.getPoint();
		int point2 = c2.getPoint();
		if (point1 != point2 || point1 == static_cast<int>(PokerPoint::Joker))
			return DouDiZhuRule::compareCard(c1, c2);

		// 同点数时只有王需要比花色，否则大小相同
		return 0;
	}

	int LackeyRule::getBombOrder(int genre) const {
		for (int i = 0; i < 18; i++) {
			if (BOMB_ORDERS[i] == genre)
				return i;
		}
		return -1;
	}

	int LackeyRule::getBombByOrder(int order) const {
		if (order < 0 || order > 17)
			return static_cast<int>(LackeyGenre::Invalid);
		return BOMB_ORDERS[order];
	}

	bool LackeyRule::straightPairExcluded(const PokerCard& c) const {
		// 连对不能出现2和王
		int pt = c.getPoint();
		if (pt == static_cast<int>(PokerPoint::Two) ||
			pt == static_cast<int>(PokerPoint::Joker))
			return true;

		return DouDiZhuRule::straightPairExcluded(c);
	}

	bool LackeyRule::straightTripleExcluded(const PokerCard& c) const {
		// 三顺不能出现2和王
		int pt = c.getPoint();
		if (pt == static_cast<int>(PokerPoint::Two) ||
			pt == static_cast<int>(PokerPoint::Joker))
			return true;

		return DouDiZhuRule::straightTripleExcluded(c);
	}

	bool LackeyRule::butterflyExcluded(const PokerCard& c) const {
		// 蝴蝶不能出现2和王
		int pt = c.getPoint();
		if (pt == static_cast<int>(PokerPoint::Two) ||
			pt == static_cast<int>(PokerPoint::Joker))
			return true;

		return DouDiZhuRule::butterflyExcluded(c);
	}

	int LackeyRule::getDefaultLackeyCard(const PokerDealer& dealer) const {
		if (_defaultLackeyCard == -1) {
			PokerCard c(PokerPoint::Eight, PokerSuit::Heart);
			if (dealer.getFirstCard(c))
				_defaultLackeyCard = c.getId();
		}
		return _defaultLackeyCard;
	}
}