// GuanDanRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.05.30

#ifndef _NIU_MA_GUAN_DAN_RULE_H_
#define _NIU_MA_GUAN_DAN_RULE_H_

#include "DouDiZhuRule.h"

namespace NiuMa
{
	// 掼蛋牌型
	enum class GuanDanGenre : int
	{
		// 无效牌型
		Invalid = 0,
		// 单张
		Single,
		// 顺子(如45678)
		Straight,
		// 对子
		Pair1,
		// 3连对（木板，如223344）
		Pair3,
		// 三张
		Triple1,
		// 三顺2（钢板，如777888）
		Triple2,
		// 三带2
		ThreeWith2,
		// 4炸
		Bomb4,
		// 5炸
		Bomb5,
		// 同花顺
		StraightFlush,
		// 6炸
		Bomb6,
		// 7炸
		Bomb7,
		// 8炸
		Bomb8,
		// 9炸
		Bomb9,
		// 10炸
		Bomb10,
		// 王炸（天王炸，即2大王2小王）
		BombJoker
	};

	/**
	 * 掼蛋规则
	 * 这里需要特别注意的一个地方：
	 * 对于两个牌值的比较不能简单比较这两个牌值在牌值顺序表中的位置，所有牌值在顺序表中的
	 * 位置索引都是固定不变的(与级牌无关)，掼蛋的规则是大王>小王>级牌>A>K>...>2，而级牌在
	 * 每局游戏里面是会发生变动的。
	 * 另一个需要特别注意的地方：
	 * 掼蛋在组成顺子(木板、钢板)的时候是不考虑级牌牌值大小顺序的，例如假设一局里面3是级
	 * 牌，此局内34567依然是有效顺子，同理223344也是有效木板，222333和333444都是有效钢板
	 * 且222333比333444要小。
	 */
	class GuanDanRule : public DouDiZhuRule
	{
	public:
		GuanDanRule();
		virtual ~GuanDanRule();

	public:
		// 牌型表
		static const int GENRE_TABLE[16];

		// 炸弹大小顺序表(越排后越大)
		static const int BOMB_ORDERS[9];

		static const int GENRE_CARD_NUMS[16];	// 牌型对应的牌张数

		// A2345特殊顺子(木板、钢板)牌值(点数)顺序表
		// 因为在掼蛋游戏中，A2345和10JQKA都是合法顺子，而JQKA2和KA234都不是合法顺子
		// 此外，AA2233是合法木板，AAA222也是合法钢板，且都是最小的木板及钢板
		static const int A2345_ORDERS[5];

		/**
		 * 获取指定牌值在A2345牌值顺序表中的位置索引
		 * @param point 牌值
		 * @return 在A2345牌值顺序表中的位置索引，不在则返回-1
		 */
		static int getA2345Order(int point);

	public:
		/**
		 * 设置级牌牌值(点数)
		 * @param point 级牌牌值(点数)
		 */
		void setGradePoint(int point);

		/**
		 * 获取级牌牌值(点数)
		 * @return 级牌牌值(点数)
		 */
		int getGradePoint() const;

		/**
		 * 设置最新已出的牌型
		 * @param genre 牌型
		 */
		void setLatestGenre(int genre);

		/**
		 * 获取最新已出的牌型
		 * @return 最新已出的牌型
		 */
		int getLatestGenre() const;

		/**
		 * 判定指定的牌是否为级牌
		 * @param c 指定牌
		 * @return 若为级牌返回true，否则返回false
		 */
		bool isGradeCard(const PokerCard& c) const;

		/**
		 * 判定指定的牌是否为逢人配
		 * @param c 指定牌
		 * @return 若为逢人配返回true，否则返回false
		 */
		bool isVariableCard(const PokerCard& c) const;

	public:
		// 返回总共有多少副牌
		virtual int getPackNums() const override;

		// 判定牌型
		virtual int predicateCardGenre(PokerGenre& pcg) const override;

		// 获取牌型的牌张数
		virtual int getGenreCardNums(int genre) const override;

		// 判定两个牌值pt1是否大于pt2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		// 牌值顺序：王 > 级牌 > A > K > Q > J > 10 > 9 > 8 > 7 > 6 > 5 > 4 > 3 > 2
		virtual int comparePoint(int pt1, int pt2) const override;

		// 判定两个牌c1是否大于c2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareCard(const PokerCard& c1, const PokerCard& c2) const override;

		// 判断两个牌型pcg1是否大于pcg2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const override;

		// 获得炸弹牌型在炸弹顺序表中的位置
		virtual int getBombOrder(int genre) const override;

		// 获得顺序表某一位置对应的炸弹牌型
		virtual int getBombByOrder(int order) const override;

		// 连对牌型排除的牌
		virtual bool straightPairExcluded(const PokerCard& c) const override;

		// 三顺牌型排除的牌
		virtual bool straightTripleExcluded(const PokerCard& c) const override;

		/**
		 * 牌数组是否为顺子
		 * @param cards 牌数组，已按从小到大排序
		 */
		virtual bool straight(const CardArray& cards) const override;

	private:
		/**
		 * 判定五张牌为顺子(同花顺)牌型
		 * @param pcg 牌型
		 * @param num1 逢人配数量
		 * @param num2 大小王数量
		 * @param graph 忽略掉逢人配的牌值曲线图
		 * @return 返回牌型
		 */
		int predicateStraight(PokerGenre& pcg, int num1, int num2, const std::vector<std::pair<int, int> >& graph) const;

		/**
		 * 判定六张牌为钢板或者木板
		 * @param pcg 牌型（不包含大小王）
		 * @param graph 忽略掉逢人配的牌值曲线图
		 * @return 返回牌型
		 */
		int predicateSteelWood(PokerGenre& pcg, const std::vector<std::pair<int, int> >& graph) const;

	private:
		// 当前级牌牌值(点数)，一开始为2
		int _gradePoint;

		// 牌桌上除"不要"外当前最新已出的牌型，默认为GuanDanGenre::Invalid，一局开始时，或当所有玩家都不要出牌玩家再次出牌，
		// 或者友家借风获得出牌权时，该字段被设置为默认值
		int _latestGenre;
	};
}

#endif // !_NIU_MA_GUAN_DAN_RULE_H_