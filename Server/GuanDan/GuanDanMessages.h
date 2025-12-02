// GuanDanMessages.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.06.05

#ifndef _NIU_MA_GUAN_DAN_MESSAGES_H_
#define _NIU_MA_GUAN_DAN_MESSAGES_H_

#include "Game/GameMessages.h"
#include "PokerCard.h"

namespace NiuMa
{
	class GuanDanMessages
	{
	private:
		GuanDanMessages() {}

	public:
		virtual ~GuanDanMessages() {}

		static void registMessages();
	};

	/**
	 * 请求同步掼蛋房间数据消息
	 * 客户端->服务器
	 */
	class MsgGuanDanSync : public MsgVenueInner {
	public:
		MsgGuanDanSync() {}
		virtual ~MsgGuanDanSync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步掼蛋房间数据消息
	 * 服务器->客户端
	 */
	class MsgGuanDanSyncResp : public MsgBase
	{
	public:
		MsgGuanDanSyncResp();
		virtual ~MsgGuanDanSyncResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 房号
		std::string number;

		// 等级
		int level;

		// 房主座位号
		int ownerSeat;

		// 座位号
		int seat;

		// 状态
		int gameState;

		MSGPACK_DEFINE_MAP(number, level, ownerSeat, seat, gameState);
	};

	/**
	 * 通知入座状态变化
	 * 服务器->客户端
	 */
	class MsgGuanDanSitting : public MsgBase
	{
	public:
		MsgGuanDanSitting();
		virtual ~MsgGuanDanSitting() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 当前游戏状态
		int gameState;

		MSGPACK_DEFINE_MAP(gameState);
	};

	/**
	 * 通知房主座位号变更
	 * 服务器->客户端
	 */
	class MsgGuanDanOwnerSeat : public MsgBase
	{
	public:
		MsgGuanDanOwnerSeat();
		virtual ~MsgGuanDanOwnerSeat() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否进入入座状态，true-是，false-离开入座状态
		int ownerSeat;

		MSGPACK_DEFINE_MAP(ownerSeat);
	};

	/**
	 * 开始游戏消息
	 * 客户端->服务器
	 */
	class MsgGuanDanStartGame : public MsgVenueInner
	{
	public:
		MsgGuanDanStartGame() {}
		virtual ~MsgGuanDanStartGame() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应房主开始游戏消息
	 * 服务器->客户端
	 */
	class MsgGuanDanStartGameResp : public MsgBase
	{
	public:
		MsgGuanDanStartGameResp();
		virtual ~MsgGuanDanStartGameResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 响应结果，0-成功，非0-其他错误
		int result;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(result, errMsg);
	};

	/**
	 * 通知客户端当前级牌消息
	 * 服务器->客户端
	 */
	class MsgGuanDanGradePoint : public MsgBase
	{
	public:
		MsgGuanDanGradePoint();
		virtual ~MsgGuanDanGradePoint() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 红方(0、2座位号玩家固定为红方)当前级牌牌值(点数)
		int gradePointRed;

		// 蓝方(1、3座位号玩家固定为蓝方)当前级牌牌值(点数)
		int gradePointBlue;

		// 本局哪方当庄，1-红方，2-蓝方，首局为0即都不当庄
		int banker;

		// 是否为实时消息，客户端接收到实时消息后需要播放提示动画
		bool realTime;

		MSGPACK_DEFINE_MAP(gradePointRed, gradePointBlue, banker, realTime);
	};

	/**
	 * 通知客户端开始发牌消息
	 * 服务器->客户端
	 */
	class MsgGuanDanDealCard : public MsgBase
	{
	public:
		MsgGuanDanDealCard() {}
		virtual ~MsgGuanDanDealCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符
		int placeholder = 0;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * 发送手牌数据消息
	 * 服务器->客户端
	 */
	class MsgGuanDanHandCard : public MsgBase
	{
	public:
		MsgGuanDanHandCard();
		virtual ~MsgGuanDanHandCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 手牌
		CardArray cards;

		// 级牌点数
		int gradePoint;

		// 收到的进贡或者收到的还贡牌id
		int contributeId;

		MSGPACK_DEFINE_MAP(cards, gradePoint, contributeId);
	};

	/**
	 * 通知客户端玩家抗贡消息
	 * 服务器->客户端
	 */
	class MsgResistTribute : public MsgBase
	{
	public:
		MsgResistTribute();
		virtual ~MsgResistTribute() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 提示文本
		std::string tip;

		// 抗贡玩家座位号1
		int seat1;

		// 抗贡玩家座位号2
		int seat2;

		MSGPACK_DEFINE_MAP(tip, seat1, seat2);
	};

	/**
	 * 通知客户端等待进贡消息
	 * 服务器->客户端
	 */
	class MsgWaitPresentTribute : public MsgBase
	{
	public:
		MsgWaitPresentTribute();
		virtual ~MsgWaitPresentTribute() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 提示文本
		std::string tip;

		// 进贡玩家座位号1
		int seat1;

		// 进贡玩家座位号2
		int seat2;

		// 已等待多久(毫秒)
		int elapsed;

		MSGPACK_DEFINE_MAP(tip, seat1, seat2, elapsed);
	};

	/**
	 * 进贡消息
	 * 客户端->服务器
	 */
	class MsgPresentTribute : public MsgVenueInner
	{
	public:
		MsgPresentTribute();
		virtual ~MsgPresentTribute() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 进贡的牌id
		int cardId;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, cardId);
	};

	/**
	 * 通知客户端进贡结果消息
	 * 服务器->客户端
	 */
	class MsgPresentTributeResult : public MsgBase
	{
	public:
		MsgPresentTributeResult();
		virtual ~MsgPresentTributeResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否进贡成功
		bool success;

		// 进贡的牌id
		int cardId;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(success, cardId, errMsg);
	};

	/**
	 * 通知客户端等待还贡消息
	 * 服务器->客户端
	 */
	class MsgWaitRefundTribute : public MsgBase
	{
	public:
		MsgWaitRefundTribute();
		virtual ~MsgWaitRefundTribute() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 提示文本
		std::string tip;

		// 进贡玩家座位号1
		int seat1;

		// 进贡玩家座位号2
		int seat2;

		// 已等待多久(毫秒)
		int elapsed;

		// 收到的进贡牌
		PokerCard cardIn;

		MSGPACK_DEFINE_MAP(tip, seat1, seat2, elapsed, cardIn);
	};

	/**
	 * 还贡消息
	 * 客户端->服务器
	 */
	class MsgRefundTribute : public MsgVenueInner
	{
	public:
		MsgRefundTribute();
		virtual ~MsgRefundTribute() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 还贡的牌id
		int cardId;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, cardId);
	};

	/**
	 * 通知客户端还贡结果消息
	 * 服务器->客户端
	 */
	class MsgRefundTributeResult : public MsgBase
	{
	public:
		MsgRefundTributeResult();
		virtual ~MsgRefundTributeResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否还贡成功
		bool success;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(success, errMsg);
	};

	/**
	 * 通知客户端进/还贡流程完成消息
	 * 服务器->客户端
	 */
	class MsgTributeComplete : public MsgBase
	{
	public:
		MsgTributeComplete() {}
		virtual ~MsgTributeComplete() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符
		int placeholder = 0;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * 通知等待出牌
	 * 服务器->客户端
	 */
	class MsgGuanDanWaitPlayCard : public MsgBase
	{
	public:
		MsgGuanDanWaitPlayCard();
		virtual ~MsgGuanDanWaitPlayCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 已等待多久(毫秒)
		int elapsed;

		// 等待出牌的玩家座位号
		int seat;

		// 是否是本轮出牌的首次出牌
		bool firstPlay;

		// 是否能出牌(即要得起最新已打出的牌)，若为false，则只能"不要"
		bool canPlay;

		MSGPACK_DEFINE_MAP(elapsed, seat, firstPlay, canPlay);
	};

	/**
	 * 玩家执行出牌操作消息
	 * 客户端->服务器
	 */
	class MsgGuanDanDoPlayCard : public MsgVenueInner
	{
	public:
		MsgGuanDanDoPlayCard();
		virtual ~MsgGuanDanDoPlayCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 是否过(即不要)
		bool pass;

		// 出牌id列表
		std::vector<int> cardIds;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, pass, cardIds);
	};

	/**
	 * 通知出牌失败消息
	 * 服务器->客户端
	 */
	class MsgGuanDanPlayCardFailed : public MsgBase
	{
	public:
		MsgGuanDanPlayCardFailed();
		virtual ~MsgGuanDanPlayCardFailed() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 原因代号
		int reason;

		MSGPACK_DEFINE_MAP(reason);
	};

	/**
	 * 通知玩家已出牌消息
	 * 服务器->客户端
	 */
	class MsgGuanDanPlayCard : public MsgBase
	{
	public:
		MsgGuanDanPlayCard();
		virtual ~MsgGuanDanPlayCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 出牌玩家座位号
		int seat;

		// 牌型
		int genre;

		// 是否过(即不要)
		bool pass;

		// 是否是本轮出牌的首次出牌
		bool firstPlay;

		// 是否为实时消息，客户端接收到实时消息后需要播放动画(例如炸弹动画)
		bool realTime;

		// 出牌数组
		CardArray cards;

		MSGPACK_DEFINE_MAP(seat, genre, pass, firstPlay, realTime, cards);
	};

	/**
	 * 通知玩家当前剩余多少张牌未出
	 * 服务器->客户端
	 */
	class MsgGuanDanCardNums : public MsgBase
	{
	public:
		MsgGuanDanCardNums();
		virtual ~MsgGuanDanCardNums() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家座位号
		int seat;

		// 当前剩余多少张牌未出
		int nums;

		MSGPACK_DEFINE_MAP(seat, nums);
	};

	/**
	 * 通知玩家即将出完牌
	 * 服务器->客户端
	 */
	class MsgGuanDanCardAlert : public MsgBase
	{
	public:
		MsgGuanDanCardAlert();
		virtual ~MsgGuanDanCardAlert() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 即将出完牌的玩家座位号
		int seat;

		MSGPACK_DEFINE_MAP(seat);
	};

	/**
	 * 玩家请求提示出牌消息
	 * 客户端->服务器
	 */
	class MsgGuanDanHintCard : public MsgVenueInner
	{
	public:
		MsgGuanDanHintCard() {}
		virtual ~MsgGuanDanHintCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 玩家请求从头开始提示出牌消息
	 * 客户端->服务器
	 */
	class MsgGuanDanResetHintCard : public MsgVenueInner
	{
	public:
		MsgGuanDanResetHintCard() {}
		virtual ~MsgGuanDanResetHintCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应玩家提示出牌消息
	 * 服务器->客户端
	 */
	class MsgGuanDanHintCardResp : public MsgBase
	{
	public:
		MsgGuanDanHintCardResp() {}
		virtual ~MsgGuanDanHintCardResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		std::vector<int> cardIds;

		MSGPACK_DEFINE_MAP(cardIds);
	};

	/**
	 * 玩家请求提示同花顺消息
	 * 客户端->服务器
	 */
	class MsgGuanDanHintStraightFlush : public MsgVenueInner
	{
	public:
		MsgGuanDanHintStraightFlush() {}
		virtual ~MsgGuanDanHintStraightFlush() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 通知清除玩家已打出的牌
	 * 服务器->客户端
	 */
	class MsgGuanDanClearPlayedOut : public MsgBase
	{
	public:
		MsgGuanDanClearPlayedOut();
		virtual ~MsgGuanDanClearPlayedOut() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家座位号
		int seat;

		MSGPACK_DEFINE_MAP(seat);
	};

	/**
	 * 通知已出完牌的玩家座位号
	 * 服务器->客户端
	 */
	class MsgGuanDanFinished : public MsgBase
	{
	public:
		MsgGuanDanFinished();
		virtual ~MsgGuanDanFinished() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 头游玩家座位号
		int touYou;

		// 二游玩家座位号
		int erYou;

		MSGPACK_DEFINE_MAP(touYou, erYou);
	};

	/**
	 * 通知玩家获得借风出牌权
	 * 服务器->客户端
	 */
	class MsgGuanDanJieFeng : public MsgBase
	{
	public:
		MsgGuanDanJieFeng();
		virtual ~MsgGuanDanJieFeng() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家座位号
		int seat;

		MSGPACK_DEFINE_MAP(seat);
	};

	/**
	 * 一局结算结果
	 * 服务器->客户端
	 */
	class MsgGuanDanResult : public MsgBase
	{
	public:
		MsgGuanDanResult();
		virtual ~MsgGuanDanResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 本局出完牌的玩家次序，finishedSeats[0]为头游玩家座位号，依次类推
		int finishedSeats[4];

		// 下一局的级牌点数
		int gradePointNext;

		// 收到消息的玩家是否被踢出房间
		bool kicks[4];

		MSGPACK_DEFINE_MAP(finishedSeats, gradePointNext, kicks);
	};

	/**
	 * 通知解散投票消息
	 * 服务器->客户端
	 */
	class MsgGuanDanDisbandVote : public MsgBase
	{
	public:
		MsgGuanDanDisbandVote();
		virtual ~MsgGuanDanDisbandVote() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 发起解散者座位号
		int disbander;

		// 等待投票已经过了多久，毫秒
		int elapsed;

		// 各玩家的选择，0-未选择、1-同意、2-反对
		int choices[4];

		MSGPACK_DEFINE_MAP(disbander, elapsed, choices);
	};
}

#endif // !_NIU_MA_GUAN_DAN_MESSAGES_H_