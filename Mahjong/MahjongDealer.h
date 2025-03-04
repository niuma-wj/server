// MahjongDealer.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.03

#ifndef _NIU_MA_MAHJONG_DEALER_H_
#define _NIU_MA_MAHJONG_DEALER_H_

#include "MahjongTile.h"

#include <map>

namespace NiuMa
{
	/**
	 * 麻将发牌器，每张牌桌配一个发牌器
	 */
	class MahjongTileGenerator;
	class MahjongDealer
	{
	public:
		MahjongDealer(bool bFlower = false);
		virtual ~MahjongDealer();

	protected:
		/**
		 * 是否带花牌
		 */
		const bool _flower;

		/**
		 * 一副牌中所有牌的总数量(不含花136，含花144)
		 */
		const int _totalTileNums;

		/**
		 * 牌池，所有剩余未被摸起的牌
		 */
		int _tilePool[144];

		/**
		 * 牌池中剩余牌的起始位置
		 */
		int _start;

		/**
		 * 牌池中剩余牌的终止位置(剩余的最后一张牌所在的位置+1)
		 */
		int _end;

	private:
		/**
		 * 牌生成器
		 */
		std::shared_ptr<MahjongTileGenerator> _generator;

	public:
		// 返回是否带花牌
		bool hasFlower() const;

		// 重新洗牌
		void shuffle();

		// 重新洗牌，在初始化所有玩家的手牌之后重洗牌池(本函数仅用于测试)
		void shuffle(const std::map<int, int>& initTiles);

		// 返回牌的总数量
		int getTotalTileNums() const;

		// 返回剩余牌的数量
		int getTileLeft() const;

		// 返回牌池是否已经为空
		bool isEmpty() const;

		// 由ID获得牌
		bool getTile(MahjongTile& mt) const;

		// 在牌池的起始位置处取回一张牌
		bool fetchTile(MahjongTile& mt);

		// 在牌池的终止位置处取回一张牌
		bool fetchTile1(MahjongTile& mt);

		// 在牌池中取回指定的一张牌
		bool fetchTile(MahjongTile& mt, const std::string& str);

		// 由ID获得牌
		static bool getTileById(MahjongTile& mt);

		// 由牌获得ID(第一个)
		static bool getIdByTile(MahjongTile& mt);
	};
}

#endif