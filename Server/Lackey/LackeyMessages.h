// LackeyMessages.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.01.03

#ifndef _NIU_MA_LACKEY_MESSAGES_H_
#define _NIU_MA_LACKEY_MESSAGES_H_

#include "Game/GameMessages.h"
#include "PokerCard.h"

namespace NiuMa
{
	class LackeyMessages
	{
	private:
		LackeyMessages() {}

	public:
		virtual ~LackeyMessages() {}

		static void registMessages();
	};

	/**
	 * 请求同步狗腿游戏数据消息
	 * 客户端->服务器
	 */
	class MsgLackeySync : public MsgVenueInner {
	public:
		MsgLackeySync() {}
		virtual ~MsgLackeySync() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应同步狗腿游戏数据消息
	 * 服务器->客户端
	 */
	class MsgLackeySyncResp : public MsgBase
	{
	public:
		MsgLackeySyncResp();
		virtual ~MsgLackeySyncResp() {}

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

		// 模式
		int mode;

		// 底注
		int diZhu;

		// 座位号
		int seat;

		// 地主座位号
		int landlord;

		// 状态
		int gameState;

		MSGPACK_DEFINE_MAP(number, level, mode, diZhu, seat, landlord, gameState);
	};

	/**
	 * 发送玩家输赢数据消息
	 * 服务器->客户端
	 */
	class MsgLackeyWinLose : public MsgBase
	{
	public:
		MsgLackeyWinLose();
		virtual ~MsgLackeyWinLose() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 座位号
		int seat;

		// 赢局总数
		int win;

		// 输局总数
		int lose;

		// 平局总数
		int draw;

		MSGPACK_DEFINE_MAP(seat, win, lose, draw);
	};

	/**
	 * 通知开始发牌消息，客户端接收到消息后做发牌动画
	 * 服务器->客户端
	 */
	class MsgLackeyDealCard : public MsgBase
	{
	public:
		MsgLackeyDealCard();
		virtual ~MsgLackeyDealCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 地主座位号
		int landlord;

		MSGPACK_DEFINE_MAP(landlord);
	};

	/**
	 * 发送手牌数据消息
	 * 服务器->客户端
	 */
	class MsgLackeyHandCard : public MsgBase
	{
	public:
		MsgLackeyHandCard() {}
		virtual ~MsgLackeyHandCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 手牌
		CardArray cards;

		MSGPACK_DEFINE_MAP(cards);
	};

	/**
	 * 通知等待玩家操作消息
	 * 服务器->客户端
	 */
	class MsgLackeyWaitOption : public MsgBase
	{
	public:
		MsgLackeyWaitOption();
		virtual ~MsgLackeyWaitOption() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 座位号
		int seat;

		// 已等待多久(毫秒)
		int elapsed;

		// 总共要等待多久(毫秒)
		int duration;

		MSGPACK_DEFINE_MAP(seat, elapsed, duration);
	};

	/**
	 * 询问地主是否叫狗腿消息
	 * 服务器->客户端
	 */
	class MsgWaitCallLackey : public MsgBase
	{
	public:
		MsgWaitCallLackey();
		virtual ~MsgWaitCallLackey() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 已等待多久(毫秒)
		int elapsed;

		MSGPACK_DEFINE_MAP(elapsed);
	};

	/**
	 * 地主执行叫狗腿操作消息
	 * 客户端->服务器
	 */
	class MsgDoCallLackey : public MsgVenueInner
	{
	public:
		MsgDoCallLackey();
		virtual ~MsgDoCallLackey() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 是否叫狗腿
		bool yes;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, yes);
	};

	/**
	 * 通知地主叫狗腿完成消息
	 * 服务器->客户端
	 */
	class MsgCallLackeyDone : public MsgBase
	{
	public:
		MsgCallLackeyDone();
		virtual ~MsgCallLackeyDone() {}

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
	 * 通知狗腿牌
	 * 服务器->客户端
	 */
	class MsgLackeyCard : public MsgBase
	{
	public:
		MsgLackeyCard() {}
		virtual ~MsgLackeyCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 狗腿牌
		PokerCard card;

		MSGPACK_DEFINE_MAP(card);
	};

	/**
	 * 通知狗腿玩家座位号
	 * 服务器->客户端
	 */
	class MsgLackeySeat : public MsgBase
	{
	public:
		MsgLackeySeat();
		virtual ~MsgLackeySeat() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		int seat;

		// 
		bool playSound;

		MSGPACK_DEFINE_MAP(seat, playSound);
	};

	/**
	 * 通知等待明牌消息
	 * 服务器->客户端
	 */
	class MsgLackeyWaitShowCard : public MsgBase
	{
	public:
		MsgLackeyWaitShowCard();
		virtual ~MsgLackeyWaitShowCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 已等待多久(毫秒)
		int elapsed;

		// 总共要等待多久(毫秒)
		int duration;

		MSGPACK_DEFINE_MAP(elapsed, duration);
	};

	/**
	 * 玩家执行明牌操作消息
	 * 客户端->服务器
	 */
	class MsgLackeyDoShowCard : public MsgVenueInner
	{
	public:
		MsgLackeyDoShowCard();
		virtual ~MsgLackeyDoShowCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 是否明牌
		bool yes;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, yes);
	};

	/**
	 * 通知玩家明牌消息
	 * 服务器->客户端
	 */
	class MsgLackeyShowCard : public MsgBase
	{
	public:
		MsgLackeyShowCard();
		virtual ~MsgLackeyShowCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		int seat;

		// 角色，0-地主，1-狗腿，2-平民
		int role;

		// 手牌
		CardArray cards;

		MSGPACK_DEFINE_MAP(seat, role, cards);
	};

	/**
	 * 通知玩家自己明牌完成消息
	 * 服务器->客户端
	 */
	class MsgLackeyShowCardDone: public MsgBase
	{
	public:
		MsgLackeyShowCardDone();
		virtual ~MsgLackeyShowCardDone() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否明牌
		bool show;

		MSGPACK_DEFINE_MAP(show);
	};

	/**
	 * 通知玩家当前正在等待他(她)出牌
	 * 服务器->客户端
	 */
	class MsgLackeyWaitPlayCard : public MsgBase
	{
	public:
		MsgLackeyWaitPlayCard();
		virtual ~MsgLackeyWaitPlayCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 是否是本轮出牌的首次出牌
		bool firstPlay;

		// 是否能出牌(即要得起最新已打出的牌)，若为false，则只能"不要"
		bool canPlay;

		// 已等待多久(毫秒)
		int elapsed;

		MSGPACK_DEFINE_MAP(firstPlay, canPlay, elapsed);
	};

	/**
	 * 玩家执行出牌操作消息
	 * 客户端->服务器
	 */
	class MsgLackeyDoPlayCard : public MsgVenueInner
	{
	public:
		MsgLackeyDoPlayCard();
		virtual ~MsgLackeyDoPlayCard() {}

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
	class MsgLackeyPlayCardFailed : public MsgBase
	{
	public:
		MsgLackeyPlayCardFailed();
		virtual ~MsgLackeyPlayCardFailed() {}

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
	class MsgLackeyPlayCard : public MsgBase
	{
	public:
		MsgLackeyPlayCard();
		virtual ~MsgLackeyPlayCard() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		int seat;

		// 打出牌型获得喜钱
		int xiQian;

		// 牌型
		int genre;

		// 是否过(即不要)
		bool pass;

		// 是否为实时消息，客户端接收到实时消息后需要播放动画(例如炸弹动画)
		bool realTime;

		// 出牌数组
		CardArray cards;

		MSGPACK_DEFINE_MAP(seat, xiQian, genre, pass, realTime, cards);
	};

	/**
	 * 通知玩家本局累计获得喜钱数量消息
	 * 服务器->客户端
	 */
	class MsgLackeyXiQian : public MsgBase
	{
	public:
		MsgLackeyXiQian();
		virtual ~MsgLackeyXiQian() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		int seat;

		// 本局累计获得喜钱数量
		int xiQian;

		// 是否为实时消息，客户端接收到实时消息后需要播放动画
		bool realTime;

		MSGPACK_DEFINE_MAP(seat, xiQian, realTime);
	};

	/**
	 * 通知玩家当前剩余多少张牌未出
	 * 服务器->客户端
	 */
	class MsgLackeyCardNums : public MsgBase
	{
	public:
		MsgLackeyCardNums();
		virtual ~MsgLackeyCardNums() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 
		int seat;

		// 当前剩余多少张牌未出
		int cardNums;

		MSGPACK_DEFINE_MAP(seat, cardNums);
	};

	/**
	 * 通知玩家即将出完牌
	 * 服务器->客户端
	 */
	class MsgLackeyCardAlert : public MsgBase
	{
	public:
		MsgLackeyCardAlert();
		virtual ~MsgLackeyCardAlert() {}

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
	class MsgLackeyHintCard : public MsgVenueInner
	{
	public:
		MsgLackeyHintCard() {}
		virtual ~MsgLackeyHintCard() {}

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
	class MsgLackeyHintCardResp : public MsgBase
	{
	public:
		MsgLackeyHintCardResp() {}
		virtual ~MsgLackeyHintCardResp() {}

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
	 * 发送所有玩家手上剩余未出的牌消息
	 * 服务器->客户端
	 */
	class MsgLackeyLeftCards : public MsgBase
	{
	public:
		MsgLackeyLeftCards() {}
		virtual ~MsgLackeyLeftCards() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家1剩余手牌
		CardArray cards1;

		// 玩家2剩余手牌
		CardArray cards2;

		// 玩家3剩余手牌
		CardArray cards3;

		// 玩家4剩余手牌
		CardArray cards4;

		// 玩家5剩余手牌
		CardArray cards5;

		MSGPACK_DEFINE_MAP(cards1, cards2, cards3, cards4, cards5);
	};

	class LackeyResult
	{
	public:
		LackeyResult();
		virtual ~LackeyResult() {}

	public:
		// 得分
		float score;

		// 喜钱分
		int xiQian;

		// 输赢金币数量
		int winGold;

		// 剩余金币数量
		int64_t gold;

		// 是否明牌
		bool showCard;

		MSGPACK_DEFINE_MAP(score, xiQian, winGold, gold, showCard);
	};

	/**
	 * 结算结果消息
	 * 服务器->客户端
	 */
	class MsgLackeyResult : public MsgBase
	{
	public:
		MsgLackeyResult();
		virtual ~MsgLackeyResult() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 倍率
		int beiLv;

		// 最先出完牌的玩家座位号
		int first;

		// 是否被提出
		bool kick;

		// 结果分
		std::vector<LackeyResult> results;

		MSGPACK_DEFINE_MAP(beiLv, first, kick, results);
	};

	/**
	 * 通知解散投票消息
	 * 服务器->客户端
	 */
	class MsgLackeyDisbandVote : public MsgBase
	{
	public:
		MsgLackeyDisbandVote();
		virtual ~MsgLackeyDisbandVote() {}

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
		int choices[5];

		MSGPACK_DEFINE_MAP(disbander, elapsed, choices);
	};
}

#endif // !_NIU_MA_LACKEY_MESSAGES_H_