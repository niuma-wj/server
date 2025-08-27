// PokerRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_POKER_RULE_H_
#define _NIU_MA_POKER_RULE_H_

#include "PokerGenre.h"

namespace NiuMa
{
	class CardOrderTable
	{
	public:
		CardOrderTable();
		virtual ~CardOrderTable();

	protected:
		// 牌值大小顺序表，越靠后的越大，不同的玩法规则要自己填写该顺序表
		int _pointOrders[14];

		// 花色大小顺序表，越靠后的越大，前4个为普通花色大小，后2个为王牌花色大小，不同的玩法规则要自己填写该顺序表
		int _suitOrders[6];

	public:
		/**
		 * 设置牌值(点数)的在顺序表中的位置
		 * @param point 牌值(点数)
		 * @param order 顺序
		 */
		void setPointOrder(int point, int order);

		// 返回传入的牌值在牌值大小顺序表中的位置
		int getPointOrder(int point) const;

		/**
		 * 设置花色的在顺序表中的位置
		 * @param suit 花色
		 * @param order 顺序
		 */
		void setSuitOrder(int suit, int order);

		// 返回传入的花色在花色大小顺序表中的位置
		int getSuitOrder(int suit) const;

		// 返回指定大小顺序的牌值
		int getPointByOrder(int order) const;

		// 返回指定大小顺序的花色
		int getSuitByOrder(int order) const;
	};

	class PokerRule : public std::enable_shared_from_this<PokerRule>
	{
	public:
		PokerRule();
		virtual ~PokerRule();

		typedef std::shared_ptr<PokerRule> Ptr;

	public:
		// 初始化
		virtual void initialise();

		// 返回总共有多少副牌
		virtual int getPackNums() const;

		// 返回牌值类型数量
		virtual int getPointNums() const;

		// 对牌值的大小顺序进行重排序
		virtual void sortPointOrder();

		// 对花色的大小顺序进行重排序
		virtual void sortSuitOrder();

		// 是否为不可出现的牌
		virtual bool isDisapprovedCard(const PokerCard& c) const;

		// 是否为不可出现的牌型
		virtual bool isDisapprovedGenre(const PokerGenre& pcg) const;

		// 牌数组里面是否包含不可出现的牌型，传递进来的牌数组必须是已经排序的(从小到大)
		virtual bool hasDisapprovedGenre(const CardArray& cards) const;

		// 判定牌型
		virtual int predicateCardGenre(PokerGenre& pcg) const;

		// 获取牌型的牌张数
		virtual int getGenreCardNums(int genre) const;

		// 是否为合法牌型
		virtual bool isValidGenre(int genre) const;

		// 判定两个牌值pt1是否大于pt2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int comparePoint(int pt1, int pt2) const;

		// 判定两个花色suit1是否大于suit2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareSuit(int suit1, int suit2) const;

		// 判定两个牌c1是否大于c2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareCard(const PokerCard& c1, const PokerCard& c2) const;

		// 判断两个牌型pcg1是否大于pcg2。返回0：两者相等或者无法比较，返回1：前者大于后者，返回2：后者大于前者
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const = 0;

		// 顺子牌型排除的牌(例如一般顺子都不能带王牌)
		virtual bool straightExcluded(const PokerCard& c) const;

		// 连对牌型排除的牌
		virtual bool straightPairExcluded(const PokerCard& c) const;

		// 三顺牌型排除的牌
		virtual bool straightTripleExcluded(const PokerCard& c) const;

		// 蝴蝶牌型排除的牌
		virtual bool butterflyExcluded(const PokerCard& c) const;

		/**
		 * 牌数组是否为顺子
		 * @param cards 牌数组，已按从小到大排序
		 */
		virtual bool straight(const CardArray& cards) const;

		/**
		 * 牌数组是否为连对
		 * @param cards 牌数组，已按从小到大排序
		 */
		virtual bool straightPair(const CardArray& cards) const;

		/**
		 * 牌数组是否为三顺
		 * @param cards 牌数组，已按从小到大排序
		 */
		virtual bool straightTriple(const CardArray& cards) const;

		/**
		 * 牌数组是否为蝴蝶
		 * @param cards 牌数组，已按从小到大排序
		 */
		virtual bool butterfly(const CardArray& cards) const;

	public:
		// 返回传入的牌值在牌值大小顺序表中的位置，从0开始
		int getPointOrder(int point) const;

		// 返回传入的花色在花色大小顺序表中的位置，从0开始
		int getSuitOrder(int suit) const;

		// 返回指定位置的牌值
		int getPointByOrder(int order) const;

		// 返回指定位置的花色
		int getSuitByOrder(int order) const;

	protected:
		// 牌值及花色大小顺序表
		CardOrderTable* _orderTable;
	};

	/**
	 * 牌值顺序比较器
	 * 比较两个牌值的大小顺序
	 */ 
	class PointOrderComparator : public PointComparator
	{
	public:
		PointOrderComparator(const PokerRule* rule);
		virtual ~PointOrderComparator() {}

	protected:
		// 牌值比较实现
		virtual bool compareImpl(int a, int b) const override;

	protected:
		const PokerRule* _rule;
	};

	// 牌比较器
	class CardComparator
	{
	public:
		CardComparator(const PokerRule::Ptr& rule);
		virtual ~CardComparator();

	public:
		/**
		 * 比较牌a与牌b的大小
		 * @param a 牌a
		 * @param b 牌b
		 * @return 当b > a返回true，否则返回false
		 */
		bool operator()(const PokerCard& a, const PokerCard& b) const;

	protected:
		PokerRule::Ptr _rule;
	};
}

#endif // _NIU_MA_POKER_RULE_H_