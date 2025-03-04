// StandardMahjongMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.04

#ifndef _NIU_MA_STANDARD_MAHJONG_MESSAGES_H_
#define _NIU_MA_STANDARD_MAHJONG_MESSAGES_H_

#include "MahjongMessages.h"
#include "MahjongSettlement.h"

namespace NiuMa
{
	class StandardMahjongMessages
	{
	private:
		StandardMahjongMessages() {}

	public:
		virtual ~StandardMahjongMessages() {}

		static void registMessages();
	};

	/**
	 * 请求同步麻将游戏数据消息
	 * 客户端->服务器
	 */
	class MsgMahjongSync : public MsgVenueInner {
	public:
		MsgMahjongSync() {}
		virtual ~MsgMahjongSync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步麻将游戏数据消息
	 * 服务器->客户端
	 */
	class MsgMahjongSyncResp : public MsgBase
	{
	public:
		MsgMahjongSyncResp();
		virtual ~MsgMahjongSyncResp();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 房间编号，用于手动输入进入房间
		std::string number;

		// 玩家自己的金币数量
		int64_t gold;

		// 玩家自己的钻石数量
		int64_t diamond;

		// 模式
		int mode;

		// 底注
		int diZhu;

		// 是否可吃
		bool chi;

		// 是否可点炮
		bool dianPao;

		// 当前是否摸起一张牌未打出
		bool hasFetch;

		// 玩家自己的座位号
		int seat;

		// 当前局状态
		int roundState;

		// 解散状态
		int disbandState;

		// 庄家座位号
		int banker;

		// 牌池剩余牌数量
		int leftTiles;

		// 所有玩家的手牌数量
		int handTileNums[4];

		// 最新摸上的牌
		MahjongTile fetchTile;

		// 自己手牌
		MahjongTileArray handTiles;

		// 所有玩家打出的牌
		MahjongTileArray playedTiles[4];

		// 所有玩家的牌章
		MahjongChapterArray chapters[4];

		MSGPACK_DEFINE_MAP(number, gold, diamond, mode, diZhu, chi, dianPao, hasFetch,
			seat, roundState, disbandState, banker, leftTiles, handTileNums,
			fetchTile, handTiles, playedTiles, chapters);
	};

	/**
	 * 开始新一局消息
	 * 服务器->客户端
	 */
	class MsgMahjongStartRound : public MsgBase {
	public:
		MsgMahjongStartRound();
		virtual ~MsgMahjongStartRound();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 庄家座位号
		int banker;

		// 当前剩余钻石
		int diamond;

		MSGPACK_DEFINE_MAP(banker, diamond);
	};

	/**
	 * 结算数据消息
	 * 服务器->客户端
	 */
	class MsgMahjongSettlement : public MsgBase
	{
	public:
		MsgMahjongSettlement();
		virtual ~MsgMahjongSettlement();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 收到消息的玩家是否因金币不足或钻石不足而被踢出房间
		bool kick;

		// 结算后玩家的金币数
		int64_t golds[4];

		// 所有玩家的本局获利的金币数量
		int winGolds[4];

		// 结算数据
		MahjongSettlement data;

		MSGPACK_DEFINE_MAP(kick, golds, winGolds, data);
	};

	/**
	 * 通知解散投票消息
	 * 服务器->客户端
	 */
	class MsgMahjongDisbandVote : public MsgBase
	{
	public:
		MsgMahjongDisbandVote();
		virtual ~MsgMahjongDisbandVote();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 发起解散者座位号
		int disbander;

		// 等待投票已经过了多久，秒
		int elapsed;

		// 各玩家的选择，0-未选择、1-同意、2-反对
		int choices[4];

		MSGPACK_DEFINE_MAP(disbander, elapsed, choices);
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_MESSAGES_H_