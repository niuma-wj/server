// MahjongMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.02

#ifndef _NIU_MA_MAHJONG_MESSAGES_H_
#define _NIU_MA_MAHJONG_MESSAGES_H_

#include "MahjongAction.h"
#include "MahjongChapter.h"

#include "Game/GameMessages.h"

namespace NiuMa
{
	class MahjongMessages
	{
	private:
		MahjongMessages() {}

	public:
		virtual ~MahjongMessages() {}

		static void registMessages();
	};

	/**
	 * 发牌消息
	 * 服务器->客户端
	 */
	class MsgMahjongTiles : public MsgBase
	{
	public:
		MsgMahjongTiles();
		virtual ~MsgMahjongTiles();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 手牌
		MahjongTileArray tiles;

		MSGPACK_DEFINE_MAP(tiles);
	};

	/**
	 * 通知当前玩家更新消息
	 * 服务器->客户端
	 */
	class MsgActorUpdated : public MsgBase
	{
	public:
		MsgActorUpdated();
		virtual ~MsgActorUpdated();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;

		MSGPACK_DEFINE_MAP(actor);
	};

	/**
	 * 通知等待玩家执行动作
	 * 服务器->客户端
	 */
	class MsgWaitAction : public MsgBase {
	public:
		MsgWaitAction();
		virtual ~MsgWaitAction();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 等待(true)或取消等待(false)
		bool waitting;

		// 被通知的玩家是否正在被等待
		bool beingHeld;

		// 已等待时间(秒)
		int second;

		MSGPACK_DEFINE_MAP(waitting, beingHeld, second);
	};

	/**
	 * 通知摸牌消息
	 * 服务器->客户端
	 */
	class MsgFetchTile : public MsgBase
	{
	public:
		MsgFetchTile();
		virtual ~MsgFetchTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 摸起牌的玩家
		int player;
		// 剩余的牌数量
		int nums;
		// 是否从后面摸起(杠后补牌)
		bool back;
		//
		MahjongTile tile;

		MSGPACK_DEFINE_MAP(player, nums, back, tile);
	};

	/**
	 * 通知动作选项消息
	 * 服务器->客户端
	 */
	class MsgActionOption : public MsgBase
	{
	public:
		MsgActionOption();
		virtual ~MsgActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<MahjongActionOption> actionOptions;

		MSGPACK_DEFINE_MAP(actionOptions);
	};

	/**
	 * 执行动作选项
	 * 客户端->服务器
	 */
	class MsgDoActionOption : public MsgVenueInner {
	public:
		MsgDoActionOption();
		virtual ~MsgDoActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 动作id
		int actionId;

		// 牌id
		int tileId;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, actionId, tileId);
	};

	/**
	 * 执行动作选项，点“过”，即放弃胡、杠、碰、吃
	 * 客户端->服务器
	 */
	class MsgPassActionOption : public MsgVenueInner {
	public:
		MsgPassActionOption();
		virtual ~MsgPassActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 通知动作选项已经完成
	 * 服务器->客户端
	 */
	class MsgActionOptionFinish : public MsgBase {
	public:
		MsgActionOptionFinish();
		virtual ~MsgActionOptionFinish();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符，没啥用
		int placeholder;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * 通知玩家出牌消息
	 * 服务器->客户端
	 */
	class MsgPlayTile : public MsgBase
	{
	public:
		MsgPlayTile();
		virtual ~MsgPlayTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;							// 出牌的玩家
		MahjongTile tile;					// 打出的牌
		MahjongTileArray handTiles;			// 出牌之后的手牌

		MSGPACK_DEFINE_MAP(actor, tile, handTiles);
	};

	/**
	 * 通知玩家杠牌消息
	 * 服务器->客户端
	 */
	class MsgGangTile : public MsgBase
	{
	public:
		MsgGangTile();
		virtual ~MsgGangTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;						// 杠牌玩家
		int player;						// 被杠牌的玩家
		int tileNums;					// 剩余手牌数量
		int chapter;					// 杠章类型(直杠、加杠、暗)
		MahjongTileArray handTiles;		// 杠、碰、吃牌之后的手牌
		MahjongChapterArray chapters;	// 牌章

		MSGPACK_DEFINE_MAP(actor, player, tileNums, chapter, handTiles, chapters);
	};

	/**
	 * 通知玩家碰、吃牌消息
	 * 服务器->客户端
	 */
	class MsgPengChiTile : public MsgBase
	{
	public:
		MsgPengChiTile();
		virtual ~MsgPengChiTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 碰、吃牌玩家
		int actor;

		// 被碰、吃牌的玩家
		int player;

		// 碰-true, 吃-false
		bool pengOrChi;

		//
		MahjongTileArray tiles;

		// 碰、吃牌之后的手牌
		MahjongTileArray handTiles;		

		MSGPACK_DEFINE_MAP(actor, player, pengOrChi, tiles, handTiles);
	};

	/**
	 * 通知玩家听牌消息
	 * 服务器->客户端
	 */
	class MsgTingTile : public MsgBase {
	public:
		MsgTingTile();
		virtual ~MsgTingTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 听牌列表
		MahjongTile::TileArray tiles;

		MSGPACK_DEFINE_MAP(tiles);
	};

	/**
	 * 通知玩家胡消息
	 * 服务器->客户端
	 */
	class MsgHuTile : public MsgBase
	{
	public:
		MsgHuTile();
		virtual ~MsgHuTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int players[3];					// 胡牌玩家位置
		int actor;						// 出牌玩家(即放炮玩家)
		bool ziMo;						// 是否自摸
		MahjongTile tile;				// 所胡的牌

		MSGPACK_DEFINE_MAP(players, actor, ziMo, tile);
	};

	/**
	 * 显示所有玩家的手牌消息(用于在一局结束时，所有玩家的牌倒下)
	 * 服务器->客户端
	 */
	class MsgShowTiles : public MsgBase
	{
	public:
		MsgShowTiles();
		virtual ~MsgShowTiles();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		MahjongTileArray handTiles[4];	// 所有玩家的手牌

		MSGPACK_DEFINE_MAP(handTiles);
	};

	/**
	 * 提示玩家因过碰或过胡而不能碰或胡
	 * 服务器->客户端
	 */
	class MsgPassTip : public MsgBase {
	public:
		MsgPassTip();
		virtual ~MsgPassTip();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 0-过碰，1-过胡
		int action;

		// 过碰或过胡的牌
		std::string tile;

		MSGPACK_DEFINE_MAP(action, tile);
	};

	/**
	 * 指定下一次摸起的牌(仅用于测试游戏相关算法的正确性)
	 * 客户端->服务器
	 */
	class MsgNextTile : public MsgVenueInner {
	public:
		MsgNextTile();
		virtual ~MsgNextTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 牌id
		int tileId;

		// 牌名称
		std::string tileName;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, tileId, tileName);
	};
}

#endif