// MahjongTile.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_TILE_H_
#define _NIU_MA_MAHJONG_TILE_H_

#include <vector>

#include "msgpack/msgpack.hpp"

namespace NiuMa
{
	/**
	 * 麻将牌
	 */
	class MahjongTile
	{
	public:
		// 牌类型:筒、条、万、字、花，所有牌的ID都按照牌类型和牌数值来排序
		enum class Pattern : int
		{
			Invalid = 0,// 无效
			Tong,		// 筒子
			Tiao,		// 条子
			Wan,		// 万子
			Dong,		// 东风
			Nan,		// 南风
			Xi,			// 西风
			Bei,		// 北风
			Zhong,		// 红中
			Fa,			// 发财
			Bai,		// 白板
			Chun,		// 春
			Xia,		// 夏
			Qiu,		// 秋
			Winter,		// 冬
			Mei,		// 梅
			Lan,		// 兰
			Ju,			// 菊
			Zhu			// 竹
		};

		// 牌点数
		enum class Number : int
		{
			Invalid = 0,	// 无效
			Yi,				// 1
			Er,				// 2
			San,			// 3
			Si,				// 4
			Wu,				// 5
			Liu,			// 6
			Qi,				// 7
			Ba,				// 8
			Jiu				// 9
		};

		class Tile
		{
		public:
			Tile();
			Tile(const Tile& t);
			Tile(Pattern p, Number n = Number::Invalid);

		public:
			Tile& operator=(const Tile& t);
			bool operator==(const Tile& t) const;
			bool operator!=(const Tile& t) const;
			bool operator>(const Tile& t) const;
			bool operator>=(const Tile& t) const;
			bool operator<(const Tile& t) const;
			bool operator<=(const Tile& t) const;

			// 返回牌类型
			Pattern getPattern() const;

			// 返回牌数值
			Number getNumber() const;

			// 判断牌是否为有效牌
			bool isValid() const;

			// 设置为无效牌
			void setInvalid();

			// 是否为带数值的牌、字牌和花牌不带数值
			bool isNumbered() const;

			// 判断两张牌是否相邻，如1万和2万、4筒和5筒，参数astride为true时判断是否跨一张牌相邻，如1万和3万，4筒和6筒
			bool isAdjacent(const Tile& t, bool astride) const;

			// 转换为字符串
			void toString(std::string& text) const;

			// 从字符串解析而来
			static Tile fromString(const std::string& text);

		private:
			int pattern;		// 牌类型
			int number;			// 牌点数
		
		public:
			MSGPACK_DEFINE_MAP(pattern, number);
		};

		typedef std::vector<Tile> TileArray;

		static const int INVALID_ID;
		static const char* PATTERN_NAMES[18];
		static const char* NUMBER_NAMES[9];

	public:
		MahjongTile();
		MahjongTile(const MahjongTile& mt);
		MahjongTile(int id, Pattern p, Number n);
		virtual ~MahjongTile();

	public:
		MahjongTile& operator=(const MahjongTile& mt);
		// 用于摆牌排列的比较函数，单纯的比较两张牌的ID
		bool operator==(const MahjongTile& mt) const;
		bool operator!=(const MahjongTile& mt) const;
		bool operator>(const MahjongTile& mt) const;
		bool operator>=(const MahjongTile& mt) const;
		bool operator<(const MahjongTile& mt) const;
		bool operator<=(const MahjongTile& mt) const;

	public:
		// 是否为有效牌
		bool isValid() const;

		// 设置无效牌
		void setInvalid();

		// 返回牌
		const Tile& getTile() const;

		//
		void setTile(const Tile& t);

		// 返回牌类型
		Pattern getPattern() const;

		// 返回牌数值
		Number getNumber() const;

		// 返回牌ID
		int getId() const;

		// 设置牌ID
		void setId(int id_);

		// 判断两张牌是不是同类型同数值
		bool isSame(const MahjongTile& mt) const;

		// 判断两张牌是不是同类型同数值
		bool isSame(const MahjongTile::Tile& t) const;

		// 判断两张牌是否相邻，如1万和2万、4筒和5筒，参数astride为true时判断是否跨一张牌相邻，如1万和3万，4筒和6筒
		bool isAdjacent(const MahjongTile::Tile& t, bool astride) const;

	private:
		/**
		 * 牌ID，每张牌都有唯一ID，即便是同类型同点数
		 */
		int id;

		/**
		 * 牌
		 */
		Tile tile;

	public:
		MSGPACK_DEFINE_MAP(id, tile);
	};

	typedef std::vector<MahjongTile> MahjongTileArray;
}

#endif