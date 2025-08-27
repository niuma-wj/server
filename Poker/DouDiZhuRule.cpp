// DouDiZhuRule.cpp

#include "DouDiZhuRule.h"

namespace NiuMa
{
	DouDiZhuRule::DouDiZhuRule()
	{}

	DouDiZhuRule::~DouDiZhuRule()
	{}

	bool DouDiZhuRule::isBomb(const PokerGenre& pcg) const {
		int ret = getBombOrder(pcg.getGenre());
		return (ret > -1);
	}

	int DouDiZhuRule::compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const {
		int genre1 = pcg1.getGenre();
		int genre2 = pcg2.getGenre();
		int bomb1 = getBombOrder(genre1);
		int bomb2 = getBombOrder(genre2);
		if (bomb1 == -1) {
			if (bomb2 != -1)
				return 2;
			// 两个非炸弹必须相同牌型且张数相同才能比较
			if (genre1 != genre2)
				return 0;
			if (pcg1.getCardNums() != pcg2.getCardNums())
				return 0;
			return compareCard(pcg1.getOfficer(), pcg2.getOfficer());
		}
		else if (bomb2 == -1)
			return 1;
		if (bomb1 > bomb2)
			return 1;
		else if (bomb1 < bomb2)
			return 2;
		return compareCard(pcg1.getOfficer(), pcg2.getOfficer());
	}
}