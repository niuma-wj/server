// PokerCard.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.23

#ifndef _NIU_MA_POKER_CARD_H_
#define _NIU_MA_POKER_CARD_H_

#include <vector>
#include <string>
#include <memory>

#include "msgpack/msgpack.hpp"

namespace NiuMa
{
	/**
	 * 扑克牌牌值(点数)
	 */
	enum class PokerPoint : int
	{
		Invalid,	// 无效
		Ace,		// 
		Two,
		Three,
		Four,
		Five,
		Six,
		Seven,
		Eight,
		Nine,
		Ten,
		Jack,
		Queen,
		King,
		Joker,		//
		Total
	};

	/**
	 * 扑克牌花色
	 */
	enum class PokerSuit : int
	{
		Invalid,	// 无效
		Diamond,	// 方块
		Club,		// 梅花
		Heart,		// 红桃
		Spade,		// 黑桃
		Little,		// 小王
		Big,		// 大王
		Total
	};

	/**
	 * 扑克牌
	 */
	class PokerCard
	{
	public:
		PokerCard(PokerPoint point_ = PokerPoint::Invalid, PokerSuit suit_ = PokerSuit::Invalid, int id_ = -1);
		PokerCard(const PokerCard& c);
		virtual ~PokerCard();

	protected:
		/**
		 * 点数
		 */
		int point;

		/**
		 * 花色
		 */
		int suit;

		/**
		 * ID，每张牌有独立的ID，即便是两张具有相同的牌值和花色的牌，
		 * 它们的id也不相同，id其实就是每张牌在牌池里面的索引
		 */
		int id;

	public:
		// 返回ID
		int getId() const;

		// 返回牌值
		int getPoint() const;

		// 设置牌值
		void setPoint(int p);

		// 返回花色
		int getSuit() const;

		// 设置花色
		void setSuit(int s);

		// 判断扑克牌是否有效
		bool isValid() const;

		// 返回扑克牌的名称，例如“方块4”
		void toString(std::string& name) const;

		// 牌存储到int32中
		int toInt32() const;

		// 从int32中解析
		void fromInt32(int val);

		// 赋值操作
		PokerCard& operator=(const PokerCard& c);

		// 判断两张牌是否为相同牌值及花色
		bool operator==(const PokerCard& c) const;

		// 判断两张牌是否为相同牌值及花色
		bool operator!=(const PokerCard& c) const;

	public:
		MSGPACK_DEFINE_MAP(point, suit, id);
	};

	typedef std::vector<PokerCard> CardArray;

	void getCardIds(const CardArray& cards, std::vector<int>& ids);

	// 牌值较器基类
	class PointComparator
	{
	public:
		PointComparator() {}
		virtual ~PointComparator() {}

		typedef std::shared_ptr<PointComparator> Ptr;

	public:
		/**
		 * 比较牌值a与牌值b的大小
		 * @param a 牌值a
		 * @param b 牌值b
		 * @return 当b > a返回true，否则返回false
		 */
		bool operator()(int a, int b) const;

	protected:
		// 牌值比较实现
		virtual bool compareImpl(int a, int b) const = 0;
	};
}

#endif