// GuanDanRule.cpp

#include "Base/Log.h"
#include "GuanDanRule.h"
#include "PokerUtilities.h"

namespace NiuMa
{
	const int GuanDanRule::GENRE_TABLE[16] = {
		static_cast<int>(GuanDanGenre::Single),
		static_cast<int>(GuanDanGenre::Straight),
		static_cast<int>(GuanDanGenre::Pair1),
		static_cast<int>(GuanDanGenre::Pair3),
		static_cast<int>(GuanDanGenre::Triple1),
		static_cast<int>(GuanDanGenre::Triple2),
		static_cast<int>(GuanDanGenre::ThreeWith2),
		static_cast<int>(GuanDanGenre::Bomb4),
		static_cast<int>(GuanDanGenre::Bomb5),
		static_cast<int>(GuanDanGenre::StraightFlush),
		static_cast<int>(GuanDanGenre::Bomb6),
		static_cast<int>(GuanDanGenre::Bomb7),
		static_cast<int>(GuanDanGenre::Bomb8),
		static_cast<int>(GuanDanGenre::Bomb9),
		static_cast<int>(GuanDanGenre::Bomb10),
		static_cast<int>(GuanDanGenre::BombJoker)
	};

	const int GuanDanRule::BOMB_ORDERS[9] = {
		static_cast<int>(GuanDanGenre::Bomb4),
		static_cast<int>(GuanDanGenre::Bomb5),
		static_cast<int>(GuanDanGenre::StraightFlush),
		static_cast<int>(GuanDanGenre::Bomb6),
		static_cast<int>(GuanDanGenre::Bomb7),
		static_cast<int>(GuanDanGenre::Bomb8),
		static_cast<int>(GuanDanGenre::Bomb9),
		static_cast<int>(GuanDanGenre::Bomb10),
		static_cast<int>(GuanDanGenre::BombJoker)
	};

	const int GuanDanRule::GENRE_CARD_NUMS[16] = { 1, 5, 2, 6, 3, 6, 5, 4, 5, 5, 6, 7, 8, 9, 10, 4 };

	const int GuanDanRule::A2345_ORDERS[5] = {
		static_cast<int>(PokerPoint::Ace),
		static_cast<int>(PokerPoint::Two),
		static_cast<int>(PokerPoint::Three),
		static_cast<int>(PokerPoint::Four),
		static_cast<int>(PokerPoint::Five)
	};

	GuanDanRule::GuanDanRule()
		: _gradePoint(static_cast<int>(PokerPoint::Two))
		, _latestGenre(static_cast<int>(GuanDanGenre::Invalid))
	{
		_orderTable = new CardOrderTable();
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Two), 0);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Three), 1);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Four), 2);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Five), 3);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Six), 4);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Seven), 5);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Eight), 6);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Nine), 7);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Ten), 8);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Jack), 9);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Queen), 10);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::King), 11);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Ace), 12);
		_orderTable->setPointOrder(static_cast<int>(PokerPoint::Joker), 13);
	}

	GuanDanRule::~GuanDanRule()
	{}

	int GuanDanRule::getA2345Order(int point) {
		for (int i = 0; i < 5; i++) {
			if (point == A2345_ORDERS[i])
				return i;
		}
		return -1;
	}

	void GuanDanRule::setGradePoint(int point) {
		_gradePoint = point;
	}

	int GuanDanRule::getGradePoint() const {
		return _gradePoint;
	}

	void GuanDanRule::setLatestGenre(int genre) {
		_latestGenre = genre;
	}

	int GuanDanRule::getLatestGenre() const {
		return _latestGenre;
	}

	bool GuanDanRule::isGradeCard(const PokerCard& c) const {
		return (_gradePoint == c.getPoint());
	}

	bool GuanDanRule::isVariableCard(const PokerCard& c) const {
		if (_gradePoint != c.getPoint())
			return false;
		if (c.getSuit() != static_cast<int>(PokerSuit::Heart))
			return false;
		return true;
	}

	int GuanDanRule::getPackNums() const {
		return 2;
	}

	int GuanDanRule::predicateCardGenre(PokerGenre& pcg) const {
		int genre = static_cast<int>(GuanDanGenre::Invalid);
		int nums = pcg.getCardNums();
		if (nums == 0)
			return genre;
		int types = 0;
		PointComparator::Ptr comp(new PointOrderComparator(this));
		std::vector<std::pair<int, int> > graph;
		std::string str;
		const CardArray& cards = pcg.getCards();
		if (nums == 1) {
			genre = static_cast<int>(GuanDanGenre::Single);
			pcg.setOfficer(cards[0]);
		}
		else if (nums == 2) {
			if (pcg.samePoint()) {
				// 两张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Pair1);
				pcg.setOfficer(cards[1]);
			}
			else if (pcg.hasPoint(static_cast<int>(PokerPoint::Joker))) {
				// 两张牌点数不同，且其中一张为大王或小王，直接返回牌型非法
				return genre;
			}
			else if (pcg.hasCard(_gradePoint, static_cast<int>(PokerSuit::Heart))) {
				// 两张牌都不为大小王，且其中一张为逢人配，则可组成对子
				genre = static_cast<int>(GuanDanGenre::Pair1);
				pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
			}
		}
		else if (nums == 3) {
			if (pcg.samePoint()) {
				// 三张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Triple1);
				pcg.setOfficer(cards[2]);
			}
			else if (pcg.hasPoint(static_cast<int>(PokerPoint::Joker))) {
				// 三张牌点数不同，且其中一或两张为大王或小王，直接返回牌型非法
				return genre;
			}
			else {
				types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
				if (types == 1) {
					// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
					genre = static_cast<int>(GuanDanGenre::Triple1);
					pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
				}
			}
		}
		else if (nums == 4) {
			if (pcg.samePoint()) {
				// 四张牌点数相同
				if (cards[0].getPoint() == static_cast<int>(PokerPoint::Joker)) {
					// 王炸
					genre = static_cast<int>(GuanDanGenre::BombJoker);
				}
				else {
					// 普通四炸
					genre = static_cast<int>(GuanDanGenre::Bomb4);
				}
				pcg.setOfficer(cards[3]);
			}
			else if (pcg.hasPoint(static_cast<int>(PokerPoint::Joker))) {
				// 四张牌点数不同，且其中包含大王或小王，直接返回牌型非法
				return genre;
			}
			else {
				types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
				if (types == 1) {
					// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
					genre = static_cast<int>(GuanDanGenre::Bomb4);
					pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
				}
			}
		}
		else if (nums == 5) {
			if (pcg.samePoint()) {
				// 五张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Bomb5);
				pcg.setOfficer(cards[4]);
			}
			else {
				PokerUtilities::getPointGraph(cards, graph, _gradePoint, static_cast<int>(PokerSuit::Heart), comp);
				int tmp1 = PokerUtilities::getCardNums(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
				int tmp2 = PokerUtilities::getPointNums(cards, static_cast<int>(PokerPoint::Joker));
				int size = static_cast<int>(graph.size());
				if (size == 1) {
					// 忽略掉逢人配后，牌值种类数量为1，则说明cards中的另一种牌必定为逢人配，即此时tmp1必定大于0
					const std::pair<int, int>& pr = graph.at(0);
					if ((pr.second < 3) || (tmp1 < 1)) {
						// 因为逢人配只有2张，即tmp1最大为2，所以pr.second不能小于3
						pcg.card2String(str);
						ErrorS << "牌型错误，牌数组: " << str;
						return genre;
					}
					if (pr.second == 3) {
						if (tmp1 != 2) {
							// pr.second为3，则tmp1必定为2
							pcg.card2String(str);
							ErrorS << "牌型错误，牌数组: " << str;
							return genre;
						}
						if (pr.first == static_cast<int>(PokerPoint::Joker)) {
							// 逢人配不能与王组成5炸，只能是三带二
							genre = static_cast<int>(GuanDanGenre::ThreeWith2);
							pcg.setOfficer(cards[4]);
							return genre;
						}
					}
					else if ((pr.second == 4) && (pr.first == static_cast<int>(PokerPoint::Joker))) {
						// 四张王与一张逢人配不能组成有效牌型
						return genre;
					}
					// 逢人配与其他牌组成5炸
					genre = static_cast<int>(GuanDanGenre::Bomb5);
					pcg.setOfficer(PokerCard(static_cast<PokerPoint>(pr.first), PokerSuit::Spade));
				}
				else if (size == 2) {
					// 忽略掉逢人配后，牌值种类数量为2
					const std::pair<int, int>& pr1 = graph.at(0);
					const std::pair<int, int>& pr2 = graph.at(1);
					if (pr1.second > 3 || pr2.second > 3) {
						// 4带1，不支持该牌型
						return genre;
					}
					if (!(tmp2 == 3 || tmp2 == 2 || tmp2 == 0)) {
						// 王的张数必须为3、2或者0才有可能组成三带二
						return genre;
					}
					int point = -1;
					PokerCard card;
					if (pr1.second == 3)
						point = pr1.first;
					else if (pr2.second == 3)
						point = pr2.first;
					if (point == -1) {
						// 此时tmp1是1或者2
						if (tmp1 == 0) {
							pcg.card2String(str);
							ErrorS << "牌型错误，牌数组: " << str;
							return genre;
						}
						// 逢人配不能与王作配，且优先与大牌作配
						point = pr2.first;
						if (point == static_cast<int>(PokerPoint::Joker))
							point = pr1.first;
						// 三带二
						genre = static_cast<int>(GuanDanGenre::ThreeWith2);
						pcg.setOfficer(PokerCard(static_cast<PokerPoint>(point), PokerSuit::Spade));
					}
					else {
						if (PokerUtilities::getLastCardOfPoint(cards, card, point)) {
							// 三带二
							genre = static_cast<int>(GuanDanGenre::ThreeWith2);
							pcg.setOfficer(card);
						}
						else {
							pcg.card2String(str);
							ErrorS << "牌型错误，牌数组: " << str;
							return genre;
						}
					}
				}
				else {
					// 忽略掉逢人配后，牌值种类数量为3或以上，只能是顺子或者同花顺
					genre = predicateStraight(pcg, tmp1, tmp2, graph);
				}
			}
		}
		else if (nums == 6) {
			if (pcg.samePoint()) {
				// 六张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Bomb6);
				pcg.setOfficer(cards[5]);
			}
			else if (pcg.hasPoint(static_cast<int>(PokerPoint::Joker))) {
				// 六张牌点数不同，且其中包含大王或小王，直接返回牌型非法
				// 因为王不能组成钢板和木板
				return genre;
			}
			else {
				PokerUtilities::getPointGraph(cards, graph, _gradePoint, static_cast<int>(PokerSuit::Heart), comp);
				genre = predicateSteelWood(pcg, graph);
			}
		}
		else if (nums == 7) {
			if (pcg.samePoint()) {
				// 七张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Bomb7);
				pcg.setOfficer(cards[6]);
			}
			else {
				types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
				if (types == 1) {
					// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
					genre = static_cast<int>(GuanDanGenre::Bomb7);
					pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
				}
			}
		}
		else if (nums == 8) {
			if (pcg.samePoint()) {
				// 八张牌点数相同
				genre = static_cast<int>(GuanDanGenre::Bomb8);
				pcg.setOfficer(cards[7]);
			}
			else {
				types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
				if (types == 1) {
					// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
					genre = static_cast<int>(GuanDanGenre::Bomb8);
					pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
				}
			}
		}
		else if (nums == 9) {
			types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
			if (types == 1) {
				// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
				genre = static_cast<int>(GuanDanGenre::Bomb9);
				pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
			}
		}
		else if (nums == 10) {
			types = PokerUtilities::getPointTypes(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
			if (types == 1) {
				// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
				genre = static_cast<int>(GuanDanGenre::Bomb10);
				pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
			}
		}
		return genre;
	}

	int GuanDanRule::predicateStraight(PokerGenre& pcg, int num1, int num2, const std::vector<std::pair<int, int> >& graph) const {
		// 这里需要特别注意的地方：A2345和10JQKA都是合法顺子，而JQKA2和KA234都不是合法顺子
		int genre = static_cast<int>(GuanDanGenre::Invalid);
		if (num2 > 0) {
			// 顺子不能包含王
			return genre;
		}
		bool test = false;
		// 是否能匹配正常大小顺序的顺子，即从23456~10JQKA这些顺子
		bool test1 = false;
		// 是否能匹配特殊顺子A2345
		bool test2 = false;
		int size = static_cast<int>(graph.size());
		int order0 = -1;
		int order1 = -1;
		int slots = 0;
		for (int i = 0; i < size; i++) {
			const std::pair<int, int>& pr = graph.at(i);
			if (pr.second != 1) {
				test = true;
				break;
			}
			order1 = getPointOrder(pr.first);
			if (i > 0)
				slots += (order1 - order0 - 1);
			order0 = order1;
		}
		if (test) {
			// 不能组成顺子
			return genre;
		}
		int point = 0;
		if (slots > num1) {
			// 正常大小顺序的顺子匹配不成功，尝试匹配特殊顺子A2345
			std::vector<int> orders;
			for (int i = 0; i < size; i++) {
				const std::pair<int, int>& pr = graph.at(i);
				order1 = getA2345Order(pr.first);
				if (order1 == -1) {
					test = true;
					break;
				}
				orders.push_back(order1);
			}
			if (!test) {
				slots = 0;
				// 排序
				std::sort(orders.begin(), orders.end());
				for (int i = 0; i < size; i++) {
					order1 = orders.at(i);
					if (i > 0)
						slots += (order1 - order0 - 1);
					order0 = order1;
				}
				test2 = (slots <= num1);
				if (test2)
					point = A2345_ORDERS[orders.back()];
			}
		}
		else {
			test1 = true;
			point = graph.back().first;
		}
		if (test || !(test1 || test2)) {
			// 不能组成顺子
			return genre;
		}
		std::string str;
		const CardArray& cards = pcg.getCards();
		PokerCard card;
		if (!PokerUtilities::getLastCardOfPoint(cards, card, point)) {
			pcg.card2String(str);
			ErrorS << "牌型错误，牌数组: " << str;
			return genre;
		}
		// 判定是否为同花顺
		test = PokerUtilities::sameSuit(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
		if (test1) {
			// 正常大小顺序的顺子匹配成功
			if ((point != static_cast<int>(PokerPoint::Ace)) && (num1 > slots)) {
				// 顺子中最后一张牌不为A，且逢人配的数量还能继续往后配
				order0 = getPointOrder(static_cast<int>(PokerPoint::Ace));
				order1 += (num1 - slots);
				if (order1 > order0)
					order1 = order0;
				point = getPointByOrder(order1);
				if (test)
					card = PokerCard(static_cast<PokerPoint>(point), static_cast<PokerSuit>(card.getSuit()));
				else
					card = PokerCard(static_cast<PokerPoint>(point), PokerSuit::Spade);
			}
		}
		else {
			// 特殊顺子A2345匹配成功
			if (point != static_cast<int>(PokerPoint::Five)) {
				// 顺子中最后一张牌不为5，则必定需要用逢人配来进行配5，例如cards中的牌为：23Axx、34Axx、234Ax等，其中x代表逢人配
				if (test)
					card = PokerCard(PokerPoint::Five, static_cast<PokerSuit>(card.getSuit()));
				else
					card = PokerCard(PokerPoint::Five, PokerSuit::Spade);
			}
		}
		// 顺子
		if (test)
			genre = static_cast<int>(GuanDanGenre::StraightFlush);
		else
			genre = static_cast<int>(GuanDanGenre::Straight);
		pcg.setOfficer(card);
		return genre;
	}

	int GuanDanRule::predicateSteelWood(PokerGenre& pcg, const std::vector<std::pair<int, int> >& graph) const {
		int genre = static_cast<int>(GuanDanGenre::Invalid);
		std::string str;
		const CardArray& cards = pcg.getCards();
		int tmp1 = PokerUtilities::getCardNums(cards, _gradePoint, static_cast<int>(PokerSuit::Heart));
		int size = static_cast<int>(graph.size());
		if (size == 1) {
			// 忽略掉逢人配后，牌值的种类数量为1，说明cards中仅包含逢人配和另一种牌值的牌
			genre = static_cast<int>(GuanDanGenre::Bomb6);
			pcg.setOfficer(PokerCard(static_cast<PokerPoint>(cards[0].getPoint()), PokerSuit::Spade));
		}
		else if (size == 2) {
			// 忽略掉逢人配后，牌值的种类数量为2，判定是否是木板或者钢板
			// 当牌数组为3344xx，其中x代表逢人配时，这组牌既能组成钢板也能组成木板，此时应该怎样选择呢？
			// 若_latestGenre为木板，则判定为木板，否则优先判定为钢板
			const std::pair<int, int>& pr1 = graph.at(0);
			const std::pair<int, int>& pr2 = graph.at(1);
			int maxNum = pr1.second;
			if (pr2.second > maxNum)
				maxNum = pr2.second;
			if (maxNum > 3) {
				// 数量最大的非逢人配牌的数量为4或者5，另一种非逢人配牌的数量为1或者2，无法组成有效牌型
				return genre;
			}
			if (maxNum == 3) {
				// 数量最大的非逢人配牌的数量为3，只可能是钢板
				int point = 0;
				bool flag = false;
				if ((pr1.first == static_cast<int>(PokerPoint::Two)) &&
					(pr2.first == static_cast<int>(PokerPoint::Ace))) {
					// AAA222
					if (pr1.second < 3)
						flag = true;	// 需要使用逢人配来配2
					point = pr1.first;
				}
				else {
					int order1 = getPointOrder(pr1.first);
					int order2 = getPointOrder(pr2.first);
					if (order2 != (order1 + 1)) {
						// 无法组成钢板
						return genre;
					}
					if (pr2.second < 3)
						flag = true;	// 需要使用逢人配来配pr2.first
					point = pr2.first;
				}
				PokerCard card;
				if (flag) {
					// 需要使用逢人配来配置主牌
					card.setPoint(point);
					card.setSuit(static_cast<int>(PokerSuit::Spade));
				}
				else if (!PokerUtilities::getLastCardOfPoint(cards, card, point)) {
					pcg.card2String(str);
					ErrorS << "牌型错误，牌数组: " << str;
					return genre;
				}
				// 钢板
				genre = static_cast<int>(GuanDanGenre::Triple2);
				pcg.setOfficer(card);
			}
			else if (maxNum == 2) {
				// 两种非逢人配牌的数量都为2，且有两张逢人配，可能是钢板或者木板，根据_latestGenre的值
				// 来选择判定为钢板或者木板
				if (pr1.second != 2 || pr2.second != 2 || tmp1 != 2) {
					pcg.card2String(str);
					ErrorS << "牌型错误，牌数组: " << str;
					return genre;
				}
				bool test = false;
				int order1 = getPointOrder(pr1.first);
				int order2 = getPointOrder(pr2.first);
				int distance = order2 - order1;
				if ((distance > 2) && (pr2.first == static_cast<int>(PokerPoint::Ace))) {
					// 无法组成正常大小顺序的木板，尝试组成特殊木板，即：AA2233
					order1 = getA2345Order(pr2.first);
					order2 = getA2345Order(pr1.first);
					if (order2 != -1) {
						distance = order2 - order1;
						test = true;
					}
				}
				if (distance > 2) {
					// 无法组成钢板和木板
					return genre;
				}
				int point = 0;
				PokerCard card;
				if ((distance == 1) && (_latestGenre != static_cast<int>(GuanDanGenre::Pair3))) {
					// 组成钢板
					if (test) {
						// 组成特殊钢板，即AAA222
						point = static_cast<int>(PokerPoint::Two);
					}
					else {
						// 组成正常大小顺序的钢板
						point = pr2.first;
					}
					card.setPoint(point);
					card.setSuit(static_cast<int>(PokerSuit::Spade));
					// 钢板
					genre = static_cast<int>(GuanDanGenre::Triple2);
				}
				else {
					// 组成木板
					// 逢人配是否配木板最后一对牌
					bool flag = false;
					if (test) {
						// 组成特殊木板，即AA2233
						// 当distance为1时，木板为AA22xx；当distance为2时，木板为AAxx33，其中x为逢人配
						point = static_cast<int>(PokerPoint::Three);
						flag = (distance == 1);
					}
					else {
						// 组成正常大小顺序的木板
						point = pr2.first;
						if ((pr2.first != static_cast<int>(PokerPoint::Ace)) && (distance == 1)) {
							point = getPointByOrder(order2 + 1);
							flag = true;
						}
					}
					if (flag) {
						card.setPoint(point);
						card.setSuit(static_cast<int>(PokerSuit::Spade));
					}
					else if (!PokerUtilities::getLastCardOfPoint(cards, card, point)) {
						pcg.card2String(str);
						ErrorS << "牌型错误，牌数组: " << str;
						return genre;
					}
					// 木板
					genre = static_cast<int>(GuanDanGenre::Pair3);
				}
				pcg.setOfficer(card);
			}
		}
		else if (size == 3) {
			// 忽略掉逢人配后，牌值的种类数量为3，判定是否是木板
			const std::pair<int, int>& pr1 = graph.at(0);
			const std::pair<int, int>& pr2 = graph.at(1);
			const std::pair<int, int>& pr3 = graph.at(2);
			if (pr1.second > 2 || pr2.second > 2 || pr3.second > 2) {
				// 无法组成木板
				return genre;
			}
			int order1 = getPointOrder(pr1.first);
			int order2 = getPointOrder(pr2.first);
			int order3 = getPointOrder(pr3.first);
			int point = 0;
			// 逢人配是否配木板最后一对牌
			bool flag = false;
			bool test = ((order2 == (order1 + 1)) && (order3 == (order2 + 1)));
			if (test) {
				point = pr3.first;
				flag = (pr3.second < 2);
			}
			else {
				// 组成正常大小顺序的木板失败，尝试组成特殊木板AA2233
				if ((pr1.first == static_cast<int>(PokerPoint::Two)) &&
					(pr2.first == static_cast<int>(PokerPoint::Three)) &&
					(pr3.first == static_cast<int>(PokerPoint::Ace))) {
					test = true;
					point = pr2.first;
					flag = (pr2.second < 2);
				}
			}
			if (!test) {
				// 无法组成木板
				return genre;
			}
			PokerCard card;
			if (flag) {
				card.setPoint(point);
				card.setSuit(static_cast<int>(PokerSuit::Spade));
			}
			else if (!PokerUtilities::getLastCardOfPoint(cards, card, point)) {
				pcg.card2String(str);
				ErrorS << "牌型错误，牌数组: " << str;
				return genre;
			}
			// 木板
			genre = static_cast<int>(GuanDanGenre::Pair3);
			pcg.setOfficer(card);
		}
		return genre;
	}

	int GuanDanRule::getGenreCardNums(int genre) const {
		if (genre < static_cast<int>(GuanDanGenre::Single) || genre > static_cast<int>(GuanDanGenre::BombJoker))
			return 0;
		int index = genre - static_cast<int>(GuanDanGenre::Single);
		return GENRE_CARD_NUMS[index];
	}

	int GuanDanRule::comparePoint(int pt1, int pt2) const {
		if (pt1 == pt2)
			return 0;
		if (pt1 == static_cast<int>(PokerPoint::Joker))
			return 1;
		if (pt2 == static_cast<int>(PokerPoint::Joker))
			return 2;
		if (pt1 == _gradePoint)
			return 1;
		if (pt2 == _gradePoint)
			return 2;
		return DouDiZhuRule::comparePoint(pt1, pt2);
	}

	int GuanDanRule::compareCard(const PokerCard& c1, const PokerCard& c2) const {
		int point1 = c1.getPoint();
		int point2 = c2.getPoint();
		if (point1 != point2 || point1 == static_cast<int>(PokerPoint::Joker))
			return DouDiZhuRule::compareCard(c1, c2);

		// 同点数时只有王需要比花色，否则大小相同
		return 0;
	}

	int GuanDanRule::compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const {
		if (pcg1.getGenre() != pcg2.getGenre())
			return DouDiZhuRule::compareGenre(pcg1, pcg2);
		int genre = pcg1.getGenre();
		if (genre == static_cast<int>(GuanDanGenre::Straight) ||
			genre == static_cast<int>(GuanDanGenre::Pair3) ||
			genre == static_cast<int>(GuanDanGenre::Triple2) ||
			genre == static_cast<int>(GuanDanGenre::StraightFlush)) {
			int order1 = getPointOrder(pcg1.getOfficer().getPoint());
			int order2 = getPointOrder(pcg2.getOfficer().getPoint());
			if (order1 < order2)
				return 2;
			else if (order1 > order2)
				return 1;
			else
				return 0;
		}
		return DouDiZhuRule::compareGenre(pcg1, pcg2);
	}

	int GuanDanRule::getBombOrder(int genre) const {
		for (int i = 0; i < 9; i++) {
			if (BOMB_ORDERS[i] == genre)
				return i;
		}
		return -1;
	}

	int GuanDanRule::getBombByOrder(int order) const {
		if (order < 0 || order > 8)
			return static_cast<int>(GuanDanGenre::Invalid);
		return BOMB_ORDERS[order];
	}

	bool GuanDanRule::straightPairExcluded(const PokerCard& c) const {
		// 连对不能出现王
		int pt = c.getPoint();
		if (pt == static_cast<int>(PokerPoint::Joker))
			return true;

		return DouDiZhuRule::straightPairExcluded(c);
	}

	bool GuanDanRule::straightTripleExcluded(const PokerCard& c) const {
		// 三顺不能出现王
		int pt = c.getPoint();
		if (pt == static_cast<int>(PokerPoint::Joker))
			return true;

		return DouDiZhuRule::straightTripleExcluded(c);
	}

	bool GuanDanRule::straight(const CardArray& cards) const {
		int size = static_cast<int>(cards.size());
		if (size != 5)
			return false;
		bool test = true;
		int order0 = -1;
		int order1 = -1;
		std::vector<int> orders;
		for (int i = 0; i < size; i++) {
			const PokerCard& c = cards.at(i);
			order1 = getA2345Order(c.getPoint());
			if (order1 == -1) {
				test = false;
				break;
			}
			orders.push_back(order1);
		}
		if (!test)
			return DouDiZhuRule::straight(cards);
		std::sort(orders.begin(), orders.end());
		for (int i = 0; i < 5; i++) {
			order1 = orders.at(i);
			if (i > 0) {
				if (order1 != (order0 + 1)) {
					test = false;
					break;
				}
			}
			order0 = order1;
		}
		return test;
	}
}