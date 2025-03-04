// MahjongAction.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_ACTION_H_
#define _NIU_MA_MAHJONG_ACTION_H_

#include <vector>

#include "msgpack/msgpack.hpp"

namespace NiuMa
{
	/**
	 * 麻将动作
	 * 麻将游戏的逻辑流程及状态变化由动作来驱动
	 */
	class MahjongAction
	{
	public:
		// 动作类型
		enum class Type : int
		{
			Invalid = 0,	// 非法动作
			Fetch,			// 从剩余牌中取回一张牌
			Play,			// 打出一张牌
			Chi,			// 吃
			Peng,			// 碰
			ZhiGang,		// 直杠
			JiaGang,		// 加杠
			AnGang,			// 暗杠
			DianPao,		// 点炮(有的地方称接炮，都是胡其他玩家打出的牌的意思)
			ZiMo			// 自摸
		};

	public:
		MahjongAction();
		MahjongAction(const MahjongAction& ma);
		MahjongAction(Type type_, int slot_, int tile_);
		virtual ~MahjongAction();

	protected:
		/**
		 * 动作类型
		 */
		int type;

		/**
		 * 动作者列表中的位置索引
		 */
		int slot;

		/**
		 * 此动作的牌ID
		 */
		int tile;

	public:
		MahjongAction& operator=(const MahjongAction& ma);

	public:
		Type getType() const;
		void setType(Type t);
		int getSlot() const;
		void setSlot(int s);
		int getTile() const;
		void setTile(int id);

		MSGPACK_DEFINE_MAP(type, slot, tile);
	};

	// 动作列表
	typedef std::vector<MahjongAction> MahjongActionList;

	/**
	 * 动作者
	 * 一个动作者会有连贯多个动作，例如：最简单的摸牌是一个动作，接着出牌也是一个动作，
	 * 这两个连贯的动作都由一个动作者来完成；更复杂的情况，直杠是一个动作(1)，接着杠后补牌
	 * 又是一个动作(2)，如果摸上来的牌又能杠(暗杠)，暗杠也是一个动作(3)，接着杠后补牌又是一个
	 * 动作(4)，完后出牌还是一个动作(5)，总共5个连贯的动作都由一个动作者完成
	 */
	class MahjongActor
	{
	public:
		MahjongActor();
		MahjongActor(const MahjongActor& ma);
		MahjongActor(int p, int s);
		virtual ~MahjongActor();

	public:
		MahjongActor& operator=(const MahjongActor& ma);

	public:
		int getPlayer() const;
		int getStart() const;

	protected:
		/**
		 * 玩家在牌桌上的位置索引
		 */
		int player;

		/**
		 * 该动作者的第一个动作在动作列表中的索引
		 */
		int start;

	public:
		MSGPACK_DEFINE_MAP(player, start);
	};

	// 动作者列表
	typedef std::vector<MahjongActor> MahjongActorList;


	/**
	 * 动作选项
	 * 当玩家摸起来一张牌后，能自摸、能加杠、能暗杠，注意加杠和暗杠不一定是杠刚摸起来的牌
	 * (可能手头上能加杠或者暗杠多幅牌，每个杠都是一个选项)，这样就有多种动作选项。另外，
	 * 当玩家打出一张牌后，其他玩家能胡(可能同时有多个玩家胡)、能直杠、能碰，下家能吃(最多
	 * 能有三种吃法，每种吃法都是一个选项)，这样也有很多种动作选项
	 */
	class MahjongActionOption
	{
	public:
		MahjongActionOption();
		MahjongActionOption(const MahjongActionOption& ao);
		virtual ~MahjongActionOption();

	public:
		MahjongActionOption& operator=(const MahjongActionOption& ma);

	public:
		int getId() const;
		void setId(int id_);
		MahjongAction::Type getType() const;
		void setType(MahjongAction::Type t);
		int getPlayer() const;
		void setPlayer(int p);
		int getTileId1() const;
		void setTileId1(int id);
		int getTileId2() const;
		void setTileId2(int id);

	protected:
		/**
		 * 选项ID
		 */
		int id;

		/**
		 * 动作类型(MahjongAction::Type)
		 */
		int type;

		/**
		 * 玩家位置索引
		 */
		int player;

		/**
		 * 牌ID1
		 */
		int tile1;

		/**
		 * 牌ID2(吃选项会有两个牌)
		 */
		int tile2;

	public:
		MSGPACK_DEFINE_MAP(id, type, player, tile1, tile2);
	};
}

#endif