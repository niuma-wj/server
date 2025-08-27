// BiJiRoom.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.25

#ifndef _NIU_MA_BI_JI_ROOM_H_
#define _NIU_MA_BI_JI_ROOM_H_

#include "Game/GameRoom.h"
#include "PokerDealer.h"

namespace NiuMa
{
	class BiJiAvatar;
	class BiJiRule;
	class BiJiRoom : public GameRoom
	{
	public:
		BiJiRoom(const std::string& venueId, const std::string& number, int mode, int diZhu, const std::shared_ptr<BiJiRule>& rule);
		virtual ~BiJiRoom();

	public:
		// 游戏状态
		enum class GameState : int
		{
			Waiting,		// 正在等待游戏开始
			Dealing,		// 正在发牌
			Combining,		// 正在组牌配墩
			Comparing,		// 正在比牌
			Resting			// 正在休息，等待下局开始
		};

	public:
		virtual void initialize() override;
		virtual void onTimer() override;
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;
		virtual void onConnect(const std::string& playerId) override;

	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg, bool robot = false) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const override;
		virtual void onAvatarJoined(int seat, const std::string& playerId) override;
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;

	private:
		/**
		 * 获取当前游戏状态已经经历了多长时间
		 * @return 毫秒
		 */
		int getStateElapsed() const;

		/**
		 * 设置游戏状态
		 */
		void setState(GameState s);

		/**
		 * 开始发牌
		 */
		void beginDeal();

		/**
		 * 扣除所有玩家的钻石
		 */
		void deductDiamond();

		/**
		 * 开始组牌
		 */
		void beginCombine();

		/**
		 * 结束组牌(组牌的时间结束)
		 */
		void endCombine();

		/**
		 * 开始比对
		 */
		void beginCompare();

		/**
		 * 开始休息
		 */
		void beginRest();

		/**
		 * 结束休息
		 */
		void endRest();

		/**
		 * 自动配墩并确认(固定)
		 */
		void autoFixDun(BiJiAvatar* avatar);

		/**
		 * 自动排序配好的墩
		 */
		void autoSortDun(BiJiAvatar* avatar);

		/**
		 * 组牌完成(所有玩家完成确认(固定)墩或者弃牌)
		 */
		void combineOver();

		/**
		 * 对比墩
		 * @param dun 墩号，0、1、2
		 */
		void compareDun(int dun);

		/**
		 * 计算合计分
		 */
		void doAggregate();

	private:
		/**
		 * 处理同步数据消息
		 * @param netMsg 网络消息
		 */
		void onSyncBiJi(const NetMessage::Ptr& netMsg);

		/**
		 * 处理指挥官开始游戏消息
		 * @param netMsg 网络消息
		 */
		void onStartGame(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家配墩消息
		 * @param netMsg 网络消息
		 */
		void onMakeDun(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家撤销墩消息
		 * @param netMsg 网络消息
		 */
		void onRevocateDun(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家重置所有墩消息
		 * @param netMsg 网络消息
		 */
		void onResetDun(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家确认(固定)墩消息
		 * @param netMsg 网络消息
		 */
		void onFixDun(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家弃牌消息
		 * @param netMsg 网络消息
		 */
		void onGiveUp(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家就绪消息
		 * @param netMsg 网络消息
		 */
		void onPlayerReady(const NetMessage::Ptr& netMsg);

	private:
		/**
		 * 通知指挥官座位号
		 */
		void notifyCommander() const;

		/**
		 * 通知当前游戏状态
		 * @param playerId 目标玩家id，为空则向全体玩家发送
		 * @param elapsed 进入当前状态已经过了多久
		 */
		void notifyState(const std::string& playerId, int elapsed) const;

		/**
		 * 通知加入牌局的所有玩家座位号
		 * @param playerId 接收消息的目标玩家id，若为空则向全体玩家发送
		 */
		void notifyJoinRound(const std::string& playerId) const;

		/**
		 * 通知发牌
		 * @param playerId 玩家id
		 * @param elapsed 已发牌多长时间
		 */
		void notifyDeal(const std::string& playerId, int elapsed) const;

		/**
		 * 通知玩家配墩消息
		 * @param avatar 玩家替身
		 * @param animate 是否播放配墩动画
		 * @param dun1 是否配头墩
		 * @param dun2 是否配中墩
		 * @param dun3 是否配尾墩
		 */
		void notifyMakeDun(BiJiAvatar* avatar, bool animate, bool dun1, bool dun2, bool dun3) const;

		/**
		 * 通知玩家弃牌或者固定墩消息
		 * @param qiPai true-弃牌，false-固定墩
		 * @param avatar 玩家替身
		 * @param targetId 接收消息的目标玩家，为空则向全体玩家
		 */
		void notifyFixDun(bool qiPai, BiJiAvatar* avatar, const std::string& targetId) const;

		/**
		 * 通知牌墩结果
		 * @param dun 墩号0, 1, 2
		 * @param playerId 接收消息的玩家id，若为空则向全体玩家发送
		 * @param animate 是否播放动画
		 */
		void notifyDunResult(int dun, const std::string& playerId, bool animate = true) const;

		/**
		 * 通知合计分
		 * @param playerId 接收消息的玩家id，若为空则向全体玩家发送
		 * @param animate 是否播放动画
		 */
		void notifyAggregate(const std::string& playerId, bool animate = true) const;

		/**
		 * 发送结算数据
		 */
		void notifySettlement();

		/**
		 *
		 */
		void notifyReady(int seat) const;

	private:
		/**
		 * 房间编号，用于手动输入进入房间
		 */
		const std::string _number;

		// 模式，0-扣钻、非0-抽水
		int _mode;

		// 底注
		int _diZhu;

		// 指挥官，控制开始游戏的玩家座位号，当指挥官离线后按座位顺序切换到下一位玩家
		int _commander;

		// 游戏状态
		GameState _gameState;

		// 进入当前状态的时间
		time_t _stateTime;

		// 玩家的弃牌顺序
		int _qiPais[6];

		// 比牌阶段
		int _compare;

		// 一局结束后被踢出房间的玩家
		bool _kicks[6];

		// 是否正在投票解散房间
		bool _disbanding;

		// 发起投票解散的玩家座位号
		int _disbander;

		// 开始投票解散的时间
		time_t _beginDisbandTick;

		// 结束投票解散的时间
		time_t _endDisbandTick;

		// 比鸡规则
		const std::shared_ptr<BiJiRule> _rule;

		// 发牌过滤器
		std::shared_ptr<DealFilter> _filter;

		// 发牌器
		PokerDealer _dealer;
	};
}

#endif