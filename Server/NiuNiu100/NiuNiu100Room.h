// NiuNiu100Room.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.13

#ifndef _NIU_MA_NIUNIU100_ROOM_H_
#define _NIU_MA_NIUNIU100_ROOM_H_

#include "Game/GameRoom.h"
#include "PokerGenre.h"
#include "PokerDealer.h"

namespace NiuMa
{
	class NiuNiuRule;
	class NiuNiu100Room : public GameRoom
	{
	public:
		NiuNiu100Room(const std::shared_ptr<NiuNiuRule> rule,
			const std::string& venueId,
			const std::string& number,
			const std::string& bankerId,
			int64_t deposit,
			bool demo = false);

		virtual ~NiuNiu100Room();

	private:
		// 游戏状态
		enum class GameState : int
		{
			None,		// 空状态(游戏未开始)
			WaitNext,	// 等待下局
			Betting,	// 新一局下注
			DealCard,	// 发牌(等待4秒客户端做停止下注提示动画、发牌动画)
			Compare		// 比牌(等待10秒客户端做比牌动画)
		};

		// 下注区域
		enum class BetZone : int
		{
			Zone1,
			Zone2,
			Zone3,
			Zone4
		};

		// 筹码类型
		enum class ChipLevel : int
		{
			Chip1,		// 1金币筹码
			Chip10,		// 10金币筹码
			Chip50,		// 50金币筹码
			Chip100,	// 100金币筹码
			Chip300,	// 300金币筹码
			Chip500		// 500金币筹码
		};

		// 6种筹码金额
		static const int CHIP_AMOUNTS[6];

	private:
		// 庄家玩家id
		std::string _bankerId;

		// 当前游戏状态
		GameState _gameState;

		// 进入当前状态的时间
		time_t _stateTime;

		// 房间当前押金数
		int64_t _deposit;

		// 4个下注区域中各种筹码的数量
		int _chipNums[4][6];

		// 4个下注区域的下注金币总数
		int64_t _betTotals[4];

		// 庄家在4个下注区域的输赢金币数量
		int64_t _bankerScores[4];

	private:
		// 发牌器
		PokerDealer _dealer;

		// 发牌过滤器
		std::shared_ptr<DealFilter> _filter;

		// 牛牛规则
		std::shared_ptr<NiuNiuRule> _rule;

		// 庄家牌、4个下注区域的牌
		PokerGenre _genres[5];

		// 4个下注区域的赔率，小于0庄家赢，大于0庄家输
		int _multiples[4];

	private:
		// 排行榜上的6位玩家，_rankIds[0]为神算子
		std::string _rankIds[6];

		// 所有玩家的排行，_rank[0]为神算子，其余为按最近20局下注总额排列的玩家ID列表
		std::vector<std::string> _rank;

		// 庄家输赢趋势记录(保留20局，但实际客户端只显示最近7局)
		std::vector<int> _trends;

		// 庄家是否请求解散，房间在本局结束后解散
		bool _bankerDisband;

		// 是否正在踢出离线玩家
		bool _kickOffline;

	private:
		// 是否为演示房
		const bool _demoTable;

		// 下一次添加机器人的时间
		time_t _nextAddRobotTick;

		// 上一次机器人下注的时间(每300毫秒驱动机器人下注一次)
		time_t _lastBetRobotTick;

		// 全部机器人玩家id
		std::vector<std::string> _robots;		

	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg, bool robot = false) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;
		virtual void clean() override;

		// 重写
	public:
		virtual void onTimer() override;
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	private:
		void pushBets();
		void pushTrends();
		void setState(GameState s);

		/**
		 * 通知当前游戏状态
		 * @param ms 进入当前状态已经过了多少毫秒
		 * @param playerId 接收通知的玩家id，若为空则向全部玩家发送
		 */
		void notifyGameState(int ms, const std::string& playerId);
		void beginWaitNext();
		void beginBetting();
		void beginDealCard();
		void beginCompare();
		void bankerPay(int64_t gold);

		/**
		 * 通知对比结果
		 * @param ms 进入对比状态已经过了多少毫秒
		 * @param playerId 接收通知的玩家id，若为空则向全部玩家发送
		 */
		void notifyCompare(int ms, const std::string& playerId);

		void sendSettlement();

		/**
		 * 执行下注逻辑
		 * @param playerId 下注玩家id
		 * @param zone 下注区域
		 * @param chip 下注筹码类型
		 * @param session 接收下注失败消息的网络连接会话
		 */
		void doBet(const std::string& playerId, int zone, int chip, const Session::Ptr& session);

		// 刷新排行榜
		void refreshRank(bool updateSeat);

		/**
		 * 发送排行榜
		 * @param playerId 接收消息的玩家id，若为空则向全部玩家发送
		 */
		void sendRank(const std::string& playerId);

		// 获得牌型对应的倍数
		int getGenreMultiple(int genre) const;

		// 解散房间
		void disbandRoom();

		// 踢出所有离线玩家
		void kickOfflinePlayers();

		void updateDemo();
		void addRobot();
		void removeRobot();
		void robotBet();
		void robotBet(const std::string& playerId);

		// 消息处理
	private:
		void onSyncTable(const NetMessage::Ptr& netMsg);
		void onBet(const NetMessage::Ptr& netMsg);
		void onRankList(const NetMessage::Ptr& netMsg);
		void onTrend(const NetMessage::Ptr& netMsg);
		void onBankerDisband(const NetMessage::Ptr& netMsg);
	};
}

#endif // !_NIU_MA_NIUNIU100_ROOM_H_
