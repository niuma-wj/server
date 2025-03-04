// PokerCardGenre.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.23

#ifndef _NIU_MA_POKER_GENRE_H_
#define _NIU_MA_POKER_GENRE_H_

#include "PokerCard.h"

#include <memory>

namespace NiuMa
{
	/**
	 * 牌型
	 * 首先定义主牌与副牌：玩家打出的任意一首牌要么全部作为主牌，例如一对2，两张2整体作为主牌；
	 * 要么可以分成两部分，一部分作为主牌，另一部分则为副牌，例如3带2，3张作为主牌，而2张则作为
	 * 副牌，在牌型比较时往往只需要比较主牌
	 */
	class PokerRule;
	class PokerGenre
	{
	public:
		PokerGenre();
		PokerGenre(const PokerGenre& pcg);
		virtual ~PokerGenre();

	public:
		static const PokerGenre NullGenre;

	public:
		// 返回牌型
		int getGenre() const;

		// 设置牌型
		void setGenre(int g);

		// 返回牌的张数
		int getCardNums() const;

		// 返回牌数组
		const CardArray& getCards() const;

		// 返回牌数组
		CardArray& getCards();

		// 返回主牌中最大的牌
		const PokerCard& getOfficer() const;

		// 设置主牌中最大的牌
		void setOfficer(const PokerCard& c);

		// 返回副牌中最大的牌
		const PokerCard& getMate() const;

		// 设置副牌中最大的牌
		void setMate(const PokerCard& c);

		// 设置牌数组
		void setCards(const CardArray& cards_, const std::shared_ptr<PokerRule>& rule);

		// 清除牌型
		void clear();

		// 牌数组字符串
		void card2String(std::string& str) const;

		// 赋值操作
		PokerGenre& operator=(const PokerGenre& pcg);

		// 判断两个牌型是否相同
		bool operator==(const PokerGenre& pcg) const;

		// 判断两个牌型是否相同
		bool operator!=(const PokerGenre& pcg) const;

	public:
		// 返回第一张牌的牌值
		int getFirstPoint() const;

		// 返回第一张牌的花色
		int getFirstSuit() const;

		// 牌数组是否有统一的牌值
		bool samePoint() const;

		// 牌数组是否有统一的花色
		bool sameSuit() const;

		// 牌数组是否为M带N(例如常见的3带2)
		bool carryM_N(int M_, int N_) const;

	protected:
		// 牌型类别
		int genre;

		// 主牌中最大的牌
		PokerCard officer;

		// 副牌中最大的牌
		PokerCard mate;

		// 牌数组，在设置牌数组之后即进行排序，注意这里是从小到大，而不是习惯上拿牌的从大到小(从左往右)!!!
		CardArray cards;

	public:
		MSGPACK_DEFINE_MAP(genre, officer, mate, cards);
	};

	typedef std::vector<PokerGenre> GenreArray;
}

#endif