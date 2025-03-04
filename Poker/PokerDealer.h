// PokerDealer.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_POKER_DEALER_H_
#define _NIU_MA_POKER_DEALER_H_

#include "PokerCard.h"

#include <memory>
#include <list>

namespace NiuMa
{
	/**
	 * 发牌过滤器
	 */
	class DealFilter
	{
	public:
		DealFilter();
		virtual ~DealFilter();

		typedef std::shared_ptr<DealFilter> Ptr;

	public:
		// 指定的牌是否OK
		virtual bool isOk(const PokerCard& c) const = 0;
	};

	/**
	 * 发牌器，不同的玩法规则需要重载该类
	 */
	class PokerRule;
	class PokerCardPool;
	class PokerDealer
	{
	public:
		// 初始化并洗牌
		PokerDealer(const std::shared_ptr<PokerRule>& rule);
		virtual ~PokerDealer();

	private:
		// 多少副牌
		const int _packNums;

		// 玩法规则
		const std::shared_ptr<PokerRule> _rule;

		// 初始化时按顺序排好的牌池，后面任何时候不可改动
		std::shared_ptr<PokerCardPool> _cardPool;

		// 剩余未发出去的牌的索引列表
		std::list<int> _indicesLeft;

	protected:
		// 初始化牌池，完后牌池不可再被改动，仅由构造函数调用
		virtual void initializeCardPool();

	public:
		// 返回总共有多少副牌
		int getPackNums() const;

		// 返回牌池中总共有多少张牌
		int getCardPoolSize() const;

		// 返回牌池中剩余多少张牌未发
		int getCardLeft() const;
		
		// 返回牌池里面指定ID的牌
		void getCard(PokerCard& c, int id) const;

		// 返回牌池里面第一张与指定牌有相同牌值及花色的牌，目的是为了获得该牌的id
		bool getFirstCard(PokerCard& c) const;

		// 重新洗牌
		void shuffle();

		/**
		 * 将剩余没发出去的牌全部发出去，并等量划分成指定数量的堆
		 * @param cardHeaps 分发的牌堆
		 * @param heapNums 分发的堆数量
		 * @return 是否成功发牌
		 */
		virtual bool handOutCards(std::vector<CardArray>& cardHeaps, int heapNums);

		/**
		 * 发牌，发出nums张牌
		 * @param cards 发出去的牌数组
		 * @param nums 张数
		 * @param filter 发牌过滤器
		 * @return 是否成功发牌
		 */
		virtual bool handOutCards(CardArray& cards, int nums, const DealFilter::Ptr& filter = nullptr);

		// 发牌，发出1张牌
		virtual bool handOutCard(PokerCard& c, const DealFilter::Ptr& filter = nullptr);
	};
}

#endif