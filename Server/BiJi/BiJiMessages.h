// BiJiMessages.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.25

#ifndef _NIU_MA_BI_JI_MESSAGES_H_
#define _NIU_MA_BI_JI_MESSAGES_H_

#include "Game/GameMessages.h"
#include "PokerCard.h"

#include <vector>

namespace NiuMa
{
	class BiJiMessages
	{
	private:
		BiJiMessages() {}

	public:
		virtual ~BiJiMessages() {}

		static void registMessages();
	};

	/**
	 * 请求同步比鸡游戏数据消息
	 * 客户端->服务器
	 */
	class MsgBiJiSync : public MsgVenueInner {
	public:
		MsgBiJiSync() {}
		virtual ~MsgBiJiSync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步比鸡游戏数据消息
	 * 服务器->客户端
	 */
	class MsgBiJiSyncResp : public MsgBase
	{
	public:
		MsgBiJiSyncResp();
		virtual ~MsgBiJiSyncResp();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::string number;

		//
		int mode;

		// 
		int diZhu;

		//
		int seat;

		//
		int commander;

		//
		int gameState;

		//
		bool disbanding;

		MSGPACK_DEFINE_MAP(number, mode, diZhu, seat, commander, gameState, disbanding);
	};

	/**
	 * 通知指挥官座位号
	 * 服务器->客户端
	 */
	class MsgBiJiCommander : public MsgBase
	{
	public:
		MsgBiJiCommander();
		virtual ~MsgBiJiCommander();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		int commander;

		MSGPACK_DEFINE_MAP(commander);
	};

	/**
	 * 通知游戏状态
	 * 服务器->客户端
	 */
	class MsgBiJiGameState : public MsgBase
	{
	public:
		MsgBiJiGameState();
		virtual ~MsgBiJiGameState();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		int gameState;

		// 进入该状态已经多久，单位毫秒
		int elapsed;

		MSGPACK_DEFINE_MAP(gameState, elapsed);
	};

	/**
	 * 指挥官开始游戏
	 * 客户端->服务器
	 */
	class MsgBiJiStartGame : public MsgVenueInner
	{
	public:
		MsgBiJiStartGame() {}
		virtual ~MsgBiJiStartGame() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 指挥官开始游戏错误响应
	 * 服务器->客户端
	 */
	class MsgBiJiStartGameResp : public MsgBase
	{
	public:
		MsgBiJiStartGameResp() {}
		virtual ~MsgBiJiStartGameResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::string errMsg;

		MSGPACK_DEFINE_MAP(errMsg);
	};

	/**
	 * 通知所有参加牌局的玩家座位号
	 * 服务器->客户端
	 */
	class MsgBiJiJoinRound : public MsgBase
	{
	public:
		MsgBiJiJoinRound() {}
		virtual ~MsgBiJiJoinRound() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<int> seats;

		MSGPACK_DEFINE_MAP(seats);
	};

	/**
	 * 发牌消息
	 * 服务器->客户端
	 */
	class MsgBiJiDealCard : public MsgBase
	{
	public:
		MsgBiJiDealCard();
		virtual ~MsgBiJiDealCard();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 手牌
		CardArray cards;

		// 牌ID按原始顺序排列
		int orderRaw[9];

		// 牌ID按花色排列
		int orderSuit[9];

		int elapsed;

		MSGPACK_DEFINE_MAP(cards, orderRaw, orderSuit, elapsed);
	};

	/**
	 * 玩家配墩消息
	 * 客户端->服务器
	 */
	class MsgBiJiMakeDun : public MsgVenueInner
	{
	public:
		MsgBiJiMakeDun();
		virtual ~MsgBiJiMakeDun() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 墩号，0、1、2
		int dun;

		// 三张牌ID
		int cardIds[3];

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, dun, cardIds);
	};

	/**
	 * 响应玩家配墩
	 * 服务器->客户端
	 */
	class MsgBiJiMakeDunResp : public MsgBase
	{
	public:
		MsgBiJiMakeDunResp();
		virtual ~MsgBiJiMakeDunResp();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否播放配墩动画
		bool animate;

		// 是否配头、中、尾墩
		bool duns[3];

		// 头墩牌ID
		int dunIds1[3];

		// 中墩牌ID
		int dunIds2[3];

		// 尾墩牌ID
		int dunIds3[3];

		MSGPACK_DEFINE_MAP(animate, duns, dunIds1, dunIds2, dunIds3);
	};

	/**
	 * 对墩进行排序
	 * 服务器->客户端
	 */
	class MsgBiJiSortDun : public MsgBase
	{
	public:
		MsgBiJiSortDun();
		virtual ~MsgBiJiSortDun();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 头、中、尾墩位置是否调整
		bool duns[3];

		// 头墩牌ID
		int dunIds1[3];

		// 中墩牌ID
		int dunIds2[3];

		// 尾墩牌ID
		int dunIds3[3];

		MSGPACK_DEFINE_MAP(duns, dunIds1, dunIds2, dunIds3);
	};

	/**
	 * 玩家撤销墩消息
	 * 客户端->服务器
	 */
	class MsgBiJiRevocateDun : public MsgVenueInner
	{
	public:
		MsgBiJiRevocateDun();
		virtual ~MsgBiJiRevocateDun() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 墩号，0、1、2
		int dun;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, dun);
	};

	/**
	 * 响应玩家撤销墩
	 * 服务器->客户端
	 */
	class MsgBiJiRevocateDunResp : public MsgBase
	{
	public:
		MsgBiJiRevocateDunResp();
		virtual ~MsgBiJiRevocateDunResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 墩号，0、1、2
		int dun;

		// 被撤销的墩的三张牌ID
		int cardIds[3];

		MSGPACK_DEFINE_MAP(dun, cardIds);
	};

	/**
	 * 玩家重置所有墩消息
	 * 客户端->服务器
	 */
	class MsgBiJiResetDun : public MsgVenueInner
	{
	public:
		MsgBiJiResetDun() {}
		virtual ~MsgBiJiResetDun() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应玩家重置所有墩
	 * 服务器->客户端
	 */
	class MsgBiJiResetDunResp : public MsgBase
	{
	public:
		MsgBiJiResetDunResp();
		virtual ~MsgBiJiResetDunResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 所有牌id
		int cardIds[9];

		MSGPACK_DEFINE_MAP(cardIds);
	};

	/**
	 * 玩家确认(固定)墩消息
	 * 客户端->服务器
	 */
	class MsgBiJiFixDun : public MsgVenueInner
	{
	public:
		MsgBiJiFixDun() {}
		virtual ~MsgBiJiFixDun() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 玩家弃牌消息
	 * 客户端->服务器
	 */
	class MsgBiJiGiveUp : public MsgVenueInner
	{
	public:
		MsgBiJiGiveUp() {}
		virtual ~MsgBiJiGiveUp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应玩家确认(固定)墩或者弃牌消息
	 * 服务器->客户端
	 */
	class MsgBiJiFixDunResp : public MsgBase
	{
	public:
		MsgBiJiFixDunResp();
		virtual ~MsgBiJiFixDunResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 座位号
		int seat;
	
		// 是否弃牌
		bool qiPai;

		// 所有牌id
		int cardIds[9];

		MSGPACK_DEFINE_MAP(seat, qiPai, cardIds);
	};

	// 牌墩结果
	class DunResult
	{
	public:
		DunResult();
		DunResult(const DunResult& dr);
		virtual ~DunResult() {}

		DunResult& operator=(const DunResult& dr);

	public:
		// 座位号
		int seat;

		// 三张牌
		int cards[3];

		// 牌型
		int genre;

		// 得分
		int score;

		MSGPACK_DEFINE_MAP(seat, cards, genre, score);
	};

	/**
	 * 牌墩结果消息
	 * 服务器->客户端
	 */
	class MsgBiJiDunResult : public MsgBase
	{
	public:
		MsgBiJiDunResult();
		virtual ~MsgBiJiDunResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 墩号
		int dun;

		// 是否播放动画
		bool animate;

		// 所有玩家的墩结果
		std::vector<DunResult> results;

		MSGPACK_DEFINE_MAP(dun, animate, results);
	};

	// 合计分
	class BiJiScore
	{
	public:
		BiJiScore();
		virtual ~BiJiScore() {}

	public:
		// 座位号
		int seat;

		// 奖励分
		int reward;

		// 总得分
		int total;

		// 奖励类型
		int rewardType;

		MSGPACK_DEFINE_MAP(seat, reward, total, rewardType);
	};

	/**
	 * 合计分消息
	 * 服务器->客户端
	 */
	class MsgBiJiAggregate : public MsgBase
	{
	public:
		MsgBiJiAggregate();
		virtual ~MsgBiJiAggregate() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否播放动画
		bool animate;

		// 所有玩家的合计分
		std::vector<BiJiScore> scores;

		MSGPACK_DEFINE_MAP(animate, scores);
	};

	// 单个玩家结算数据
	class BiJiSettlement
	{
	public:
		BiJiSettlement();
		BiJiSettlement(const BiJiSettlement& s);
		virtual ~BiJiSettlement() {}

		BiJiSettlement& operator=(const BiJiSettlement& s);

	public:
		// 座位号
		int seat;

		// 各墩得分
		int dunScores[3];

		// 总得分
		int totalScore;

		// 输赢金币数量
		int64_t winGold;

		// 结算后玩家携带的金币数
		int64_t gold;

		// 奖励类型
		int rewardType;

		// 所有牌
		int cards[9];

		// 各墩牌型
		int genres[3];

		// 是否弃牌
		bool qiPai;

		MSGPACK_DEFINE_MAP(seat, dunScores, totalScore, winGold, gold, rewardType, cards, genres, qiPai);
	};

	/**
	 * 结算消息
	 * 服务器->客户端
	 */
	class MsgBiJiSettlement : public MsgBase
	{
	public:
		MsgBiJiSettlement();
		virtual ~MsgBiJiSettlement() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否被踢出游戏
		bool kick;

		// 
		std::vector<BiJiSettlement> settlements;

		MSGPACK_DEFINE_MAP(kick, settlements);
	};
}

#endif // !_NIU_MA_BI_JI_MESSAGES_H_