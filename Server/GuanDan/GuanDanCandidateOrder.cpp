// GuanDanCandidateOrder.cpp

#include "GuanDanRule.h"
#include "GuanDanCandidateOrder.h"

namespace NiuMa
{
	template<> GuanDanCandidateOrder* Singleton<GuanDanCandidateOrder>::_inst = nullptr;

	void GuanDanCandidateOrder::makeOrder1() {
		int key = 0;
		int genre = 0;
		int point = 0;
		// 2~10单张、对子
		for (int j = 0; j < 9; j++) {
			point = static_cast<int>(PokerPoint::Two) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Single);
				else
					genre = static_cast<int>(GuanDanGenre::Pair1);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// 5~10顺子
		genre = static_cast<int>(GuanDanGenre::Straight);
		for (int j = 0; j < 6; j++) {
			point = static_cast<int>(PokerPoint::Five) + j;
			key = genre << 8;
			key |= point;
			_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
		}
		// 2~10三张、三带二
		for (int j = 0; j < 9; j++) {
			point = static_cast<int>(PokerPoint::Two) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Triple1);
				else
					genre = static_cast<int>(GuanDanGenre::ThreeWith2);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// 2钢板
		genre = static_cast<int>(GuanDanGenre::Triple2);
		point = static_cast<int>(PokerPoint::Two);
		key = genre << 8;
		key |= point;
		_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
		// 3~10木板、钢板
		for (int j = 0; j < 8; j++) {
			point = static_cast<int>(PokerPoint::Three) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Pair3);
				else
					genre = static_cast<int>(GuanDanGenre::Triple2);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// J~A单张、对子
		for (int j = 0; j < 4; j++) {
			if (j == 3)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Jack) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Single);
				else
					genre = static_cast<int>(GuanDanGenre::Pair1);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// J~A顺子
		genre = static_cast<int>(GuanDanGenre::Straight);
		for (int j = 0; j < 4; j++) {
			if (j == 3)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Jack) + j;
			key = genre << 8;
			key |= point;
			_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
		}
		// J~A三张、三带二
		for (int j = 0; j < 4; j++) {
			if (j == 3)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Jack) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Triple1);
				else
					genre = static_cast<int>(GuanDanGenre::ThreeWith2);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// J~A木板、钢板
		for (int j = 0; j < 4; j++) {
			if (j == 3)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Jack) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Pair3);
				else
					genre = static_cast<int>(GuanDanGenre::Triple2);
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
		// 级牌~王单张、对子、三张、三带二
		int pointArr[2] = {
			static_cast<int>(PokerPoint::Joker) + 1,	// 级牌
			static_cast<int>(PokerPoint::Joker)			// 王
		};
		int genreArr[4] = {
			static_cast<int>(GuanDanGenre::Single),
			static_cast<int>(GuanDanGenre::Pair1),
			static_cast<int>(GuanDanGenre::Triple1),
			static_cast<int>(GuanDanGenre::ThreeWith2)
		};
		for (int j = 0; j < 2; j++) {
			point = pointArr[j];
			for (int i = 0; i < 4; i++) {
				genre = genreArr[i];
				key = genre << 8;
				key |= point;
				_orders1.insert(std::make_pair(key, static_cast<int>(_orders1.size())));
			}
		}
	}

	void GuanDanCandidateOrder::makeOrder2() {
		// 5~A顺子
		int key = 0;
		int genre = static_cast<int>(GuanDanGenre::Straight);
		int point = 0;
		for (int j = 0; j < 10; j++) {
			if (j == 9)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Five) + j;
			key = genre << 8;
			key |= point;
			_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
		}
		// 2~A三带二
		genre = static_cast<int>(GuanDanGenre::ThreeWith2);
		for (int j = 0; j < 13; j++) {
			if (j == 12)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Two) + j;
			key = genre << 8;
			key |= point;
			_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
		}
		// 2钢板
		genre = static_cast<int>(GuanDanGenre::Triple2);
		point = static_cast<int>(PokerPoint::Two);
		key = genre << 8;
		key |= point;
		_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
		// 3~A木板、钢板
		for (int j = 0; j < 12; j++) {
			if (j == 11)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = static_cast<int>(PokerPoint::Three) + j;
			for (int i = 0; i < 2; i++) {
				if (i == 0)
					genre = static_cast<int>(GuanDanGenre::Pair3);
				else
					genre = static_cast<int>(GuanDanGenre::Triple2);
				key = genre << 8;
				key |= point;
				_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
			}
		}
		// A~2三张
		genre = static_cast<int>(GuanDanGenre::Triple1);
		for (int j = 14; j > 1; j--) {
			if (j == 14)
				point = static_cast<int>(PokerPoint::Ace);
			else
				point = j;
			key = genre << 8;
			key |= point;
			_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
		}
		// 级牌~王单张、对子、三张、三带二
		int pointArr[2] = {
			static_cast<int>(PokerPoint::Joker) + 1,	// 级牌
			static_cast<int>(PokerPoint::Joker)			// 王
		};
		int genreArr[4] = {
			static_cast<int>(GuanDanGenre::Single),
			static_cast<int>(GuanDanGenre::Pair1),
			static_cast<int>(GuanDanGenre::Triple1),
			static_cast<int>(GuanDanGenre::ThreeWith2)
		};
		for (int j = 0; j < 2; j++) {
			point = pointArr[j];
			for (int i = 0; i < 4; i++) {
				genre = genreArr[i];
				key = genre << 8;
				key |= point;
				_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
			}
		}
		// A~2对子，单张
		for (int i = 0; i < 2; i++) {
			if (i == 0)
				genre = static_cast<int>(GuanDanGenre::Pair1);
			else
				genre = static_cast<int>(GuanDanGenre::Single);
			for (int j = 14; j > 1; j--) {
				if (j == 14)
					point = static_cast<int>(PokerPoint::Ace);
				else
					point = j;
				key = genre << 8;
				key |= point;
				_orders2.insert(std::make_pair(key, static_cast<int>(_orders2.size())));
			}
		}
	}

	int GuanDanCandidateOrder::makeKey(int genre, int officerPoint, int gradePoint) const {
		int point = 0;
		if (officerPoint == gradePoint)
			point = static_cast<int>(PokerPoint::Joker) + 1;
		else
			point = officerPoint;
		int key = (genre << 8) | point;
		return key;
	}

	int GuanDanCandidateOrder::getOrderImpl(int key, const std::unordered_map<int, int>& orderMap) const {
		std::unordered_map<int, int>::const_iterator it = orderMap.find(key);
		if (it != orderMap.end())
			return (it->second);
		return -1;
	}

	int GuanDanCandidateOrder::getOrder1(int genre, int officerPoint, int gradePoint) const {
		int key = makeKey(genre, officerPoint, gradePoint);
		return getOrderImpl(key, _orders1);
	}

	int GuanDanCandidateOrder::getOrder2(int genre, int officerPoint, int gradePoint) const {
		int key = makeKey(genre, officerPoint, gradePoint);
		return getOrderImpl(key, _orders2);
	}
}