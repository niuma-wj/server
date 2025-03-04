// MahjongChapter.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_CHAPTER_H_
#define _NIU_MA_MAHJONG_CHAPTER_H_

#include "MahjongTile.h"

namespace NiuMa
{
	/**
	 * 麻将章，玩家吃、碰、杠之后摆在桌面上的3(4)张牌称为一章
	 */
	class MahjongChapter
	{
	public:
		MahjongChapter();
		MahjongChapter(const MahjongChapter& mc);
		virtual ~MahjongChapter();

	public:
		enum class Type : int {
			Invalid,	// 非法
			Chi,		// 吃
			Peng,		// 碰
			ZhiGang,	// 直杠
			JiaGang,	// 加杠
			AnGang		// 暗杠
		};

	public:
		MahjongChapter& operator=(const MahjongChapter& mc);

	public:
		// 返回是否被否决
		bool isVetoed() const;
		// 将牌章否决
		void setVetoed();
		// 最终的牌章类型
		Type getType() const;
		// 在指定动作执行之后，牌章的类型
		Type getType(int actionId) const;
		//
		void getTypes(int types_[], int actionIds_[]) const;
		void addType(Type t, int actionId);
		bool hasActionId(int actionId) const;
		bool isGang() const;
		int getTargetPlayer() const;
		void setTargetPlayer(int p);
		int getTargetTile() const;
		void setTargetTile(int t);
		MahjongTileArray& getAllTiles();
		const MahjongTileArray& getAllTiles() const;
		void setAllTiles(const MahjongTileArray& tiles_);
		void hideAnGangTiles();

	private:
		/**
		 * 牌章是否被否决，被否决之后不能计入结果分数，例如有的地方规定杠上炮不能算杠分，
		 * 这时可以将最后的一个杠否决
		 */
		bool _vetoed;

		/**
		 * 类型数组，牌章在任意时刻只能是特定的一种类型，这里为什么要用类型数组呢？因为碰章有状
		 * 态变迁:碰->加杠->碰(被抢杠之后)，这里需要记录状态变迁发生时机(对应动作列表中的索引)，
		 * 以便后面可以进行游戏过程录像回放
		 */
		int types[3];

		/**
		 * 动作索引
		 */
		int actionIds[3];

		/**
		 * 目标玩家位置，玩家A打出牌被碰，那么该成员的值就是玩家A的位置索引，
		 * 如果是暗杠则该成员没有意义
		 */
		int targetPlayer;

		/**
		 * 目标牌ID，如果是暗杠则该成员没有意义
		 */
		int targetTile;

		/**
		 * 章里面的全部牌
		 */
		MahjongTileArray tiles;

	public:
		MSGPACK_DEFINE_MAP(types, actionIds, targetPlayer, targetTile, tiles);
	};

	typedef std::vector<MahjongChapter> MahjongChapterArray;
}

#endif // !_NIU_MA_MAHJONG_CHAPTER_H_
