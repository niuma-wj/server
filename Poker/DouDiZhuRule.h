// DouDiZhuRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_DOU_DI_ZHU_RULE_H_
#define _NIU_MA_DOU_DI_ZHU_RULE_H_

#include "PokerRule.h"

namespace NiuMa
{
	/**
	 * 斗地主类游戏规则
	 */
	class DouDiZhuRule : public PokerRule
	{
	public:
		DouDiZhuRule();
		virtual ~DouDiZhuRule();

	public:
		// 判断牌型是否为炸弹牌型
		bool isBomb(const PokerGenre& pcg) const;

	public:
		// 获得炸弹牌型在炸弹顺序表中的位置，若找不到指定的炸弹牌型则返回-1
		virtual int getBombOrder(int genre) const = 0;

		// 获得顺序表某一位置对应的炸弹牌型
		virtual int getBombByOrder(int order) const = 0;

		// 判断两个牌型pcg1是否大于pcg2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const override;
	};
}

#endif