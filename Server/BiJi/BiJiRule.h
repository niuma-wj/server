// BiJiRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.25

#ifndef _NIU_MA_BI_JI_RULE_H_
#define _NIU_MA_BI_JI_RULE_H_

#include "PokerRule.h"

namespace NiuMa
{
	/**
	 * 比鸡牌型
	 */
	enum class BiJiGenre : int
	{
		// 无效牌型
		Invalid,

		// 单张(乌龙)
		Single,

		// 对子
		Pair,

		// 顺子
		Straight,

		// 同花
		Flush,

		// 同花顺
		FlushStraight,

		// 三张
		Triple
	};

	/**
	 * 奖励类型
	 */
	enum class BiJiRewardType : int
	{
		None = 0,

		// 通关
		TongGuan = 0x01,

		// 全三条
		QuanSanTiao = 0x02,

		// 全顺子
		QuanShuanZi = 0x04,

		// 全黑色
		QuanHeiSe = 0x08,

		// 全红色
		QuanHongSe = 0x10,

		// 双豹子
		ShuangBaoZi = 0x20,

		// 双同花顺
		ShuangTongHuaShun = 0x40,

		// 4张
		SiZhang = 0x80
	};

	/**
	 * 比鸡规则
	 */ 
	class BiJiRule : public PokerRule
	{
	public:
		BiJiRule();
		virtual ~BiJiRule();

	public:
		// 判定牌型
		virtual int predicateCardGenre(PokerGenre& pcg) const;

		// 判断两个牌型pcg1是否大于pcg2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const;
	};
}

#endif