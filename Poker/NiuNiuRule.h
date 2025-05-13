// NiuNiuRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.13

#ifndef _NIU_MA_NIU_NIU_RULE_H_
#define _NIU_MA_NIU_NIU_RULE_H_

#include "PokerRule.h"

namespace NiuMa
{
	// 牌型
	enum class NiuNiuGenre : int
	{
		Invalid,				// 无效牌型
		Niu0,					// 没牛
		Niu1,					// 牛一
		Niu2,					// 牛二
		Niu3,					// 牛三
		Niu4,					// 牛四
		Niu5,					// 牛五
		Niu6,					// 牛六
		Niu7,					// 牛七
		Niu8,					// 牛八
		Niu9,					// 牛九
		NiuNiu = 0x0010,		// 牛牛
		ShunZi = 0x0020,		// 顺子牛
		WuHua = 0x0040,			// 五花牛
		TongHua = 0x0060,		// 同花牛
		HuLu = 0x0080,			// 葫芦牛
		ZhaDan = 0x00A0,		// 炸弹牛
		WuXiao = 0x00C0,		// 五小牛
		KaiXin = 0x00E0			// 开心牛
	};

	// 牛牛游戏规则(支持两种牛牛玩法：经典牛牛、百人牛牛)
	// 经典牛牛与百人牛牛的区别在于：经典牛牛支持顺子牛、五花牛、同花牛、葫芦牛、
	// 炸弹牛、五小牛、开心牛，而百人牛牛没有大小王牌，且仅仅支持五花牛、炸弹牛
	class NiuNiuRule : public PokerRule
	{
	public:
		NiuNiuRule(bool niu100);
		virtual ~NiuNiuRule();

	protected:
		const bool _niu100;			// 是否为百人牛牛
		static const int GENRE_ORDER_NIU100[13];		// 百人牛牛牌型大小顺序表(从小到大)
		
	public:
		// 判定牌型
		virtual int predicateCardGenre(PokerGenre& pcg) const override;

		// 判断两个牌型pcg1是否大于pcg2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const override;

		// 顺子牌型排除的牌(例如一般顺子都不能带王牌)
		virtual bool straightExcluded(const PokerCard& c) const override;

	private:
		// 判定经典牛牛牌型
		int predicateCardGenre1(PokerGenre& pcg) const;

		// 判定百人牛牛牌型
		int predicateCardGenre2(PokerGenre& pcg) const;

		// 余数1-9对应的非牛牌型
		int getNotNiuGenre(int nRemainder) const;

		// 返回牌型在百人牛牛顺序表中的位置
		static int getGenreOrderNiu100(int genre);
	};
}

#endif // _NIU_MA_NIU_NIU_RULE_H_