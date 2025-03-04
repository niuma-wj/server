// NiuNiu100Messages.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.13

#ifndef _NIU_MA_NIUNIU100_MESSAGES_H_
#define _NIU_MA_NIUNIU100_MESSAGES_H_

#include "Game/GameMessages.h"
#include "PokerGenre.h"

namespace NiuMa
{
	class NiuNiu100Messages
	{
	private:
		NiuNiu100Messages() {}

	public:
		virtual ~NiuNiu100Messages() {}

		static void registMessages();
	};

	/**
	 * 请求同步百人牛牛游戏数据消息
	 * 客户端->服务器
	 */
	class MsgNiu100Sync : public MsgVenueInner {
	public:
		MsgNiu100Sync() {}
		virtual ~MsgNiu100Sync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步百人牛牛游戏数据消息
	 * 服务器->客户端
	 */
	class MsgNiu100SyncResp : public MsgBase {
	public:
		MsgNiu100SyncResp();
		virtual ~MsgNiu100SyncResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否为展示房
		bool demo;

		// 牌桌状态
		int gameState;

		// 当前押金
		int64_t deposit;

		// 玩家金币数量
		int64_t gold;

		// 4个下注区域当前的下注总额
		int64_t betTotals[4];

		// 玩家的下注金额
		int myBetAmounts[4];

		// 4个下注区域当前各种筹码的数量
		std::vector<int> chipNums;

		// 庄家ID
		std::string bankerId;

		// 庄家昵称
		std::string bankerName;

		// 庄家头像
		std::string bankerHeadImgUrl;

		MSGPACK_DEFINE_MAP(demo, gameState, deposit, gold, betTotals, myBetAmounts, chipNums, bankerId, bankerName, bankerHeadImgUrl);
	};

	/**
	 * 通知百人牛牛游戏状态
	 * 服务器->客户端
	 */
	class MsgNiu100GameState : public MsgBase {
	public:
		MsgNiu100GameState();
		virtual ~MsgNiu100GameState() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 游戏状态
		int gameState;

		// 进入该状态已经过了多少毫秒
		int elapsed;

		MSGPACK_DEFINE_MAP(gameState, elapsed);
	};

	/**
	 * 通知百人牛牛比牌比牌结果
	 * 服务器->客户端
	 */
	class MsgNiu100Compare : public MsgBase
	{
	public:
		MsgNiu100Compare();
		virtual ~MsgNiu100Compare() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 进入比牌状态已过了多久(毫秒)
		int elapsed;

		// 玩家在本局结算后的金币数
		int64_t gold;

		// 房间当前剩余押金
		int64_t deposit;

		// 4个下注区域的输赢赔率
		int multiples[4];

		// 玩家在四个下注区域的输赢金币数
		int scores[4];

		// 庄家在四个下注区域的输赢金币数
		int64_t bankerScores[4];

		// 排行榜座位上第1位玩家在四个下注区域的输赢金币数
		int64_t rankScores1[4];

		// 排行榜座位上第2位玩家在四个下注区域的输赢金币数
		int64_t rankScores2[4];

		// 排行榜座位上第3位玩家在四个下注区域的输赢金币数
		int64_t rankScores3[4];

		// 排行榜座位上第4位玩家在四个下注区域的输赢金币数
		int64_t rankScores4[4];

		// 排行榜座位上第5位玩家在四个下注区域的输赢金币数
		int64_t rankScores5[4];

		// 排行榜座位上第6位玩家在四个下注区域的输赢金币数
		int64_t rankScores6[4];

		// 排行榜座位上6位玩家在本局结算后的金币数
		int64_t rankGolds[6];

		// 庄家牌、4个下注区域的牌型
		PokerGenre genres[5];

		MSGPACK_DEFINE_MAP(elapsed, gold, deposit, multiples, scores, bankerScores, rankScores1,
			rankScores2, rankScores3, rankScores4, rankScores5, rankScores6, rankGolds, genres);
	};

	/**
	 * 玩家下注消息
	 * 客户端->服务器
	 */
	class MsgNiu100Bet : public MsgVenueInner {
	public:
		MsgNiu100Bet();
		virtual ~MsgNiu100Bet() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 下注区域
		int zone;

		// 筹码类型
		int chip;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, zone, chip);
	};

	/**
	 * 通知玩家下注失败消息
	 * 服务器->客户端
	 */
	class MsgNiu100BetFailed : public MsgBase
	{
	public:
		MsgNiu100BetFailed() {}
		virtual ~MsgNiu100BetFailed() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(errMsg);
	};

	/**
	 * 通知玩家下注成功消息
	 * 服务器->客户端
	 */
	class MsgNiu100BetSucess : public MsgBase
	{
	public:
		MsgNiu100BetSucess();
		virtual ~MsgNiu100BetSucess() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家id
		std::string playerId;

		// 下注区域
		int zone;

		// 下注筹码
		int chip;

		MSGPACK_DEFINE_MAP(playerId, zone, chip);
	};

	/**
	 * 结算消息
	 * 服务器->客户端
	 */
	class MsgNiu100Settlement : public MsgBase
	{
	public:
		MsgNiu100Settlement();
		virtual ~MsgNiu100Settlement() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家输赢金币数
		int64_t score;

		// 庄家家输赢金币数
		int64_t bankerScore;

		// 除庄家外赢金最多的4位玩家(包含玩家自己)的输赢金币数
		int winnerScores[4];

		// 赢金最多的4位玩家ID
		std::string winnerIds[4];

		// 赢金最多的4位玩家昵称
		std::string winnerNames[4];

		// 赢金最多的4位玩家头像
		std::string winnerHeadImgUrls[4];

		MSGPACK_DEFINE_MAP(score, bankerScore, winnerScores, winnerIds, winnerNames, winnerHeadImgUrls);
	};

	/**
	 * 排行榜消息
	 * 服务器->客户端
	 */
	class MsgNiu100Rank : public MsgBase
	{
	public:
		MsgNiu100Rank();
		virtual ~MsgNiu100Rank() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 排行榜上的6位玩家Id，_rankIds[0]为神算子
		std::string rankIds[6];

		// 排行榜上的玩家金币量
		int64_t golds[6];

		// 排行榜座位上的玩家昵称
		std::string names[6];

		// 排行榜座位上的玩家头像
		std::string headImgUrls[6];

		MSGPACK_DEFINE_MAP(rankIds, golds, names, headImgUrls);
	};

	class Niu100RankItem
	{
	public:
		Niu100RankItem();
		virtual ~Niu100RankItem() {}

	public:
		// 玩家id
		std::string playerId;

		// 玩家昵称
		std::string nickname;

		// 玩家头像图片链接地址
		std::string headImgUrl;

		// 玩家金币数量
		int64_t gold;

		// 近20局的累计赢局数
		int accWins20;

		// 近20局累计下注金额
		int64_t accBets20;

		MSGPACK_DEFINE_MAP(playerId, nickname, headImgUrl, gold, accWins20, accBets20);
	};

	/**
	 * 请求获取排行榜消息
	 * 客户端->服务器
	 */
	class MsgNiu100GetRankList : public MsgVenueInner {
	public:
		MsgNiu100GetRankList() {}
		virtual ~MsgNiu100GetRankList() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 排行榜消息
	 * 服务器->客户端
	 */
	class MsgNiu100RankList : public MsgBase {
	public:
		MsgNiu100RankList() {}
		virtual ~MsgNiu100RankList() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<Niu100RankItem> items;

		MSGPACK_DEFINE_MAP(items);
	};

	/**
	 * 请求获取趋势数据消息
	 * 客户端->服务器
	 */
	class MsgNiu100GetTrend : public MsgVenueInner {
	public:
		MsgNiu100GetTrend() {}
		virtual ~MsgNiu100GetTrend() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 趋势数据消息
	 * 服务器->客户端
	 */
	class MsgNiu100Trend : public MsgBase {
	public:
		MsgNiu100Trend() {}
		virtual ~MsgNiu100Trend() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<int> trends;

		MSGPACK_DEFINE_MAP(trends);
	};

	/**
	 * 通知房主解散房间 
	 * 服务器->客户端
	 */
	class MsgNiu100BankerDisband : public MsgBase {
	public:
		MsgNiu100BankerDisband() {}
		virtual ~MsgNiu100BankerDisband() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符
		int placeHolder = 0;

		MSGPACK_DEFINE_MAP(placeHolder);
	};
}

#endif