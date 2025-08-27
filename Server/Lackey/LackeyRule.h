// LackeyRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.31

#ifndef _NIU_MA_LACKEY_RULE_H_
#define _NIU_MA_LACKEY_RULE_H_

#include "PokerDealer.h"
#include "DouDiZhuRule.h"

namespace NiuMa
{
	// 逮狗腿牌型
	enum class LackeyGenre : int
	{
		Invalid = 0,	// 无效牌型
		Single,			// 单张
		Pair1,			// 对子
		Pair3,			// 3连对
		Pair4,			// 4连对
		Pair5,			// 5连对
		Pair6,			// 6连对
		Pair7,			// 7连对
		Pair8,			// 8连对
		Pair9,			// 9连对
		Pair10,			// 10连对
		Pair11,			// 11连对
		Pair12,			// 12连对
		Triple1,		// 三条
		Triple2,		// 三顺2
		Triple3,		// 三顺3
		Triple4,		// 三顺4
		Triple5,		// 三顺5
		Triple6,		// 三顺6
		Triple7,		// 三顺7
		Triple8,		// 三顺8
		Triple9,		// 三顺9
		Triple10,		// 三顺10
		Triple11,		// 三顺11
		Triple12,		// 三顺12(地主手牌38张，3-A为张数最大的3顺)
		Butterfly1,		// 蝴蝶1(三带二)
		Butterfly2,		// 蝴蝶2
		Butterfly3,		// 蝴蝶3
		Butterfly4,		// 蝴蝶4
		Butterfly5,		// 蝴蝶5
		Butterfly6,		// 蝴蝶6
		Butterfly7,		// 蝴蝶7
		Bomb4,			// 4炸
		Bomb5,			// 5炸
		Bomb6,			// 6炸
		Bomb7,			// 7炸
		Bomb8,			// 8炸
		Bomb9,			// 9炸
		Bomb10,			// 10炸
		Bomb11,			// 11炸
		Bomb12,			// 12炸
		Bomb3L,			// 3小王
		Bomb3B,			// 3大王
		Bomb2B2L,		// 2大王2小王
		Bomb1B3L,		// 1大王3小王
		Bomb3B1L,		// 3大王1小王
		Bomb2B3L,		// 2大王3小王
		Bomb3B2L,		// 3大王2小王
		Bomb3B3L,		// 3大王3小王
		BombLackey		// 狗腿炸，地主1打4时单张狗腿牌为最大的炸弹
	};

	// 逮狗腿规则
	class LackeyRule : public DouDiZhuRule
	{
	public:
		LackeyRule();
		virtual ~LackeyRule();

	public:
		static const int CANDIDATE_ORDERS[49];	// 候选牌型顺序表

		static const int BOMB_ORDERS[18];		// 炸弹大小顺序表(越排后越大)

		static const int GENRE_CARD_NUMS[49];	// 牌型对应的牌张数

		// 计算喜钱
		static int calcXiQian(int genre, const CardArray& cards);

	public:
		// 返回总共有多少副牌
		virtual int getPackNums() const override;

		// 判定牌型
		virtual int predicateCardGenre(PokerGenre& pcg) const override;

		// 获取牌型的牌张数
		virtual int getGenreCardNums(int genre) const override;

		// 判定两个牌c1是否大于c2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareCard(const PokerCard& c1, const PokerCard& c2) const override;

		// 获得炸弹牌型在炸弹顺序表中的位置
		virtual int getBombOrder(int genre) const override;

		// 获得顺序表某一位置对应的炸弹牌型
		virtual int getBombByOrder(int order) const override;

		// 连对牌型排除的牌
		virtual bool straightPairExcluded(const PokerCard& c) const override;

		// 三顺牌型排除的牌
		virtual bool straightTripleExcluded(const PokerCard& c) const override;

		// 蝴蝶牌型排除的牌
		virtual bool butterflyExcluded(const PokerCard& c) const override;

	public:
		// 获得默认狗腿牌ID
		int getDefaultLackeyCard(const PokerDealer& dealer) const;

	private:
		// 默认狗腿牌ID(默认的狗腿牌为红桃8)
		mutable int _defaultLackeyCard;
	};
}

#endif // !_NIU_MA_LACKEY_RULE_H_