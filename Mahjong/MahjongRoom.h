// MahjongTable.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_TABLE_H_
#define _NIU_MA_MAHJONG_TABLE_H_

#include "Game/GameRoom.h"
#include "MahjongAvatar.h"
#include "MahjongAction.h"
#include "MahjongRule.h"
#include "MahjongDealer.h"
#include "MahjongSettlement.h"
#include "MahjongPlayback.h"

#include "Base/IdentityAllocator.h"

// 动作选项缓存池的大小，预留20个肯定够了
#define ACTION_OPTION_POOL_SIZE	20

namespace NiuMa
{
	class MahjongSettlement;
	/**
	 * 麻将游戏房间
	 */
	class MahjongRoom : public GameRoom
	{
	public:
		MahjongRoom(const MahjongRule::Ptr& rule, const std::string& venueId, int gameType, int maxPlayerNums = 4);
		virtual ~MahjongRoom();

	protected:
		/**
		 * 将麻将牌桌视为一个状态机，有三种状态：一种是摸起牌后的状态A，一种是等待选择动作
		 * 选项的状态B，另一种是等待出牌的状态C。玩家摸起一张牌之后进入A，此时可能加杠、暗
		 * 杠或自摸，这时就进入B，若此时不能进入B则立刻进入C，打出一张牌之后其他玩家可能进
		 * 入B，也可能是下一位玩家进入A，另外B也可以再次进入B，例如加杠之后等待其他玩家抢
		 * 杠。状态变化如此反复直到胡牌或者流局，画出状态图可更形象表达。正是状态的变迁推进
		 * 牌局的演化，如果状态停止变化，则牌局也将停止不前。
		 */
		enum class StateMachine : int
		{
			Null,
			Fetched,		// 取牌后状态A
			Action,			// 等待动作选项状态B
			Play,			// 等待出牌状态C
			End				// 结束状态D
		};

	public:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		// 清理
		virtual void clean() override;

	private:
		/**
		 * 处理执行动作选项消息
		 * @param netMsg 网络消息
		 */
		void onActionOption(const NetMessage::Ptr& netMsg);

		/**
		 * 处理过动作选项消息
		 * @param netMsg 网络消息
		 */
		void onPassActionOption(const NetMessage::Ptr& netMsg);

		/**
		 * 处理指定下一张牌消息（仅用于功能测试）
		 * @param netMsg 网络消息
		 */
		void onNextTile(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家选择了一个动作选项
		 * @param playerId 玩家id
		 * @param id 动作选项id
		 * @param tileId 牌id
		 */
		void doActionOption(const std::string& playerId, int actionId, int tileId);

		// 玩家不选择任何动作选项，即选择“过”
		void passActionOption(const std::string& playerId);

		// 自动执行动作选项(用于AI出牌)，参数bOnlyAuto表示本次调用是否仅针对自动出牌玩家
		void autoActionOption(bool bOnlyAuto);

		// 玩家指定下一次摸起的牌(仅用于测试麻将相关算法的正确性)
		void doNextTile(const std::string& playerId, const std::string& str);

		// 更新当前执行动作的玩家索引
		void updateCurrentActor();

		// 更新当前执行动作的玩家索引
		void updateCurrentActor(int player);

		// 状态跳转
		void changeState(StateMachine eNewState);

		// 取牌
		bool fetchTile(bool bBack = false);

		// 情况所有玩家的所有动作选项
		void clearActionOptions();

		// 清空指定玩家的所有动作选项
		void clearActionOptions(MahjongAvatar* pAvatar);

		// 执行玩家已经选择的动作选项
		bool executeActionOptions();

		// 执行胡动作
		bool executeHu();

		// 执行杠动作
		bool executeGang();

		// 执行碰动作
		bool executePeng();

		// 执行吃动作
		bool executeChi();

		// 执行出牌动作
		bool executePlay(int tileId);

		// 在摸牌、吃牌、碰牌之后通知出牌或者杠(加杠及暗杠)
		void afterFetchChiPeng(MahjongAvatar* pAvatar, int fetchedId = -1);

		// 胡牌，胡方式检测和算分
		void doHu();

		// 流局处理
		void noMoreTile();

		// 没有吃碰杠
		bool noChiPengGang() const;

	protected:
		// 发牌
		void dealTiles();

		// 获取第一个正在等待的动作选项
		bool getFirstWaitingActionOption(MahjongActionOption& ao) const;

		// 获取结算数据
		void getSettlementData(MahjongSettlement* dt) const;

		// 获取回放数据
		void getPlaybackData(MahjongPlaybackData& dt) const;

	protected:
		// 是否可以点炮
		virtual bool canDianPao() const;

		// 是否提前结束(流局)
		virtual bool earlyTermination() const;

		// 出牌之后是否需要再次摸牌(例如有些地方的规则规定打出花牌之后再次摸牌而不是轮到下家摸牌)
		virtual bool fetchAgainAfterPlay() const;

		// 通知发牌
		virtual void notifyDealTiles();

		// 通知当前动作者更新
		virtual void notifyActorUpdated(const std::string& playerId);

		// 通知状态变化
		virtual void notifyStateChanged(StateMachine oldState);

		// 通知玩家摸起一张牌(根据具体游戏项目的需求，可以重载该函数并通知其他玩家)
		virtual void notifyFetchTile(MahjongAvatar* pAvatar, bool bBack);

		// 通知指定玩家动作选项
		virtual void notifyActionOptions(MahjongAvatar* pAvatar);

		// 通知牌桌进入等待动作状态
		virtual void notifyWaitingAction(const std::string& playerId);

		// 通知指定玩家等待其他玩家选择动作选项
		virtual void notifyActionOptionsWaiting(MahjongAvatar* pAvatar);

		// 通知所有人动作选项结束(客户端收到通知之后，立即关闭选项和等待，当前没在等待的玩家忽略该通知)
		virtual void notifyActionOptionsFinish();

		// 通知所有人,玩家打出一张牌(包含听牌信息)
		virtual void notifyPlayTile(const MahjongTile& mt);

		// 通知所有人,玩家杠牌
		virtual void notifyGangTile(MahjongAvatar* pAvatar);

		// 通知所有人,玩家碰、吃牌
		virtual void notifyPengChiTile(MahjongAvatar* pAvatar, bool bPeng);

		// 通知指定玩家听牌
		virtual void notifyTingTile(MahjongAvatar* pAvatar);

		// 通知所有人，玩家胡牌
		virtual void notifyHuTile();

		// 显示所有玩家的手牌
		virtual void notifyShowTiles();

		/**
		 * 提示玩家因过碰或过胡而不能碰或胡
		 * @param avatar 玩家替身
		 * @param action 0-过碰，1-过胡
		 * @param tile 过碰或过胡的牌
		 */
		virtual void notifyPassTip(MahjongAvatar* avatar, int action, const std::string& tile);

		// 下一局的庄家
		virtual void bankerNextRound(int huNums, int huPlayer);

		// 计算胡牌分
		virtual void calcHuScore() const = 0;

		// 结算
		virtual void doJieSuan() = 0;

		// 胡牌之后的处理，例如保存得分到数据库、保存录像等
		virtual void afterHu() = 0;

	protected:
		MahjongRule::Ptr _rule;
		MahjongDealer _dealer;
		MahjongActionList _actions;
		MahjongActorList _actors;

	protected:
		/**
		 * 庄家索引(座位号)
		 */
		int _banker;

		/**
		 * 当前活动玩家(正在做连贯动作的玩家)索引
		 */
		int _actor;

		/**
		 * 当胡牌方式为明杠上花时，该变量表示被杠的玩家索引，当胡牌方式为抢杠时，
		 * 该变量表示要加杠的玩家索引
		 */
		int _gangHu;

		/**
		 * 最后要剩余多少张牌不摸(少于这个数即流局)
		 */
		int _tilesLeft;

		/**
		 * 刚打出的牌
		 */
		int _playedTileId;

		/**
		 * 本局所胡的牌
		 */
		int _huTileId;

		/**
		 * 牌桌进入等待状态的时间，单位毫秒
		 */
		time_t _waitingTick;

		/**
		 * 动作选项缓存池
		 */
		MahjongActionOption _acOpPool[ACTION_OPTION_POOL_SIZE];

		/**
		 * 动作选项ID分配器
		 */
		IdentityAllocator _acOpIdAlloc;

		/**
		 * 正在等待用户选择的动作选项，注意这里长度为4的数组不是指4个玩家，而是依次为“胡”、“杠”、“碰”、“吃”!!!
		 */
		std::vector<int> _acOps1[4];

		/**
		 * 用户已经选择的动作选项
		 */
		std::vector<int> _acOps2[4];

	protected:
		// 当前状态
		StateMachine _state;

		// 是否可以吃牌
		bool _chi;

		// 是否可以点炮
		bool _dianPao;

		// 是否可以一炮多响，若不可以由近到远有权利选择胡，例如A放炮、B为A下家，C为A下下家，只有当B放弃
		// 胡的情况下C才可以胡(有些地方的规则是不允许一炮多响的)
		bool _mutiDianPao;

		// 在一炮多响的情况下，是不是只要有一家选择胡，其他能胡的都立即胡(即便他之前点了“过”也仍能胡)。
		// 若该值为false，则需要点胡才能胡，只要有一家选择了胡，后面还没做选择的立即选择胡，但在这之前
		// 选择了“过”的玩家将不可胡
		bool _allDianPao;

		// 是否正在等待其他玩家抢杠
		bool _waitingQiangGang;

		// 本局是否已经胡了，否则牌局还没结束，或者是流局
		bool _hu;

		// 暗杠的牌其他玩家是否可见
		bool _anGangVisible;

		// 杠上炮是否算被否决(若不否决则要算杠分，因为有些地方的规则是杠上炮不能算杠的分)
		bool _gangShangPaoVetoed;

		// 是否可以延迟加杠
		bool _delayJiaGang;
	};
}

#endif