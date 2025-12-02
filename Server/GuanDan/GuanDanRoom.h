// GuanDanRoom.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.06.05

#ifndef _NIU_MA_GUAN_DAN_ROOM_H_
#define _NIU_MA_GUAN_DAN_ROOM_H_

#include "Game/GameRoom.h"
#include "PokerDealer.h"
#include "PokerCombination.h"

namespace NiuMa
{
	// 房间等级
	enum class GuanDanRoomLevel : int
	{
		Invalid,	// 无效
		Friend,		// 好友房
		Practice,	// 练习房(与机器人进行游戏，相当于单机模式)
		Beginner,	// 初级房
		Moderate,	// 中级房
		Advanced,	// 高级房
		Master		// 大师房
	};

	class GuanDanRule;
	class GuanDanAvatar;
	/**
	 * 入座状态<---->游戏状态(包含等待、发牌和进行中三个子状态)
	 * 1、练习场房间(单机模式)没有入座状态，玩家一进入就是游戏状态
	 * 2、非练习场房间初始为入座状态
	 * 3、朋友场房间有一位控制开始游戏的房主，在游戏状态时房主具备踢人权限，房主玩家离开后房主角色按位置顺序转移到下一个座位的玩家
	 * 4、朋友场房间在所有玩家都准备就绪后，房主可点击开始游戏，开始后进入游戏状态
	 * 5、初级场到大师场房间没有房主，当所有玩家准备后立即进入游戏状态
	 * 6、房间处于游戏状态时，当任意一位玩家离开房间，即便进入入座状态
	 * 7、房间从入座状态转为游戏状态，或者从游戏状态转为入座状态时，向客户端发出通知
	 */
	class GuanDanRoom : public GameRoom
	{
	public:
		GuanDanRoom(const std::string& venueId, const std::string& number, int lvl);
		virtual ~GuanDanRoom();

	public:
		// 游戏状态
		enum class GameState : int
		{
			Sitting,		// 等待玩家入座
			Waiting,		// 等待游戏开始
			Dealing,		// 正在发牌
			Playing			// 游戏正在进行中...
		};

		// 等待玩家操作类型
		enum class WaitOperation : int
		{
			None,			// 无等待操作
			PresentTribute,	// 等待进贡
			RefundTribute,	// 等待还贡
			PlayCard		// 等待玩家出牌
		};

		// 出牌失败原因
		enum class PlayCardFailed : int
		{
			Unknown,		// 未知错误
			CanNotPass,		// 新一轮出牌不能“不要”
			NotFound,		// 找不到指定的牌
			Invalid,		// 无效牌型
			CanNotPlay		// 要不起
		};

		// 重写
	public:
		virtual void onTimer() override;
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		// 检查指定玩家是否满足进入房间的条件
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg, bool robot = false) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		// 允许观众，玩家在坐到座位之前以观众的身份存在，观众的数量没有限制，
		// 在座位坐满之后所有观众都必须立即离开房间
		virtual bool enableSpectator() const override;
		virtual void onSpectatorLeaved(const std::string& playerId) override;
		// 玩家加入后处理
		virtual void onAvatarJoined(int seat, const std::string& playerId) override;
		// 玩家离开后处理
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;
		// 获取玩家额外信息
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const override;
		// 清理
		virtual void clean() override;

	private:
		// 返回当前房间所属的区域id
		int getDistrictId() const;

		// 返回等待操作已过了多久
		int getWaitElapsed(time_t nowTick = 0LL) const;

		// 练习房添加机器人
		void addRobot(time_t nowTick);

		// 开始发牌
		void beginDeal();

		// 获取局号数
		void getRoundNo();

		// 开始进入GameState::Playing状态
		void beginPlay();

		// 扣钻
		void deductDiamond();

		/**
		 * 根据上局出完牌的次序获取座位号
		 * @param order 上局出完牌的次序，0为头游，1为二游，依次类推
		 * @return 对应次序的座位号
		 */
		int getLastFinishedSeat(int order) const;

		/**
		 * 获取指定玩家座位号上局的出完牌次序
		 * @param seat 玩家座位号
		 * @return 出完牌次序
		 */
		int getLastFinishedOrder(int seat) const;

		/**
		 * 获取指定座位号的友家座位号
		 * @param seat 指定座位号
		 * @return 友家座位号
		 */
		int getFriendSeat(int seat) const;

		/**
		 * 判定指定的两个座位号是否互为友家
		 * @param seat1 座位号1
		 * @param seat2 座位号2
		 * @return true-互为友家，false-互为敌家
		 */
		bool isFriend(int seat1, int seat2) const;

		/**
		 * 判定座位号是红方还是蓝方
		 * @param seat 指定座位号
		 * @return true-红方，false-蓝方
		 */
		bool isRedSeat(int seat) const;

		/**
		 * 通知房主座位号变更
		 */
		void notifyOwnerSeat() const;

		/**
		 * 通知入座状态变化
		 */
		void notifySitting() const;

		/**
		 * 通知玩家托管消息
		 * @param avatar 玩家替身
		 */
		void notifyPlayerAuthorize(GuanDanAvatar* avatar) const;

		/**
		 * 判断手牌中是否包含可用于还贡的牌，即除级牌外2~10的牌
		 * @param cards 手牌数组，已从小到大排序
		 * @return 包含-true，不包含-false
		 */
		bool canRefundTribute(const CardArray& cards);

		/**
		 * 通知当前级牌
		 * @param avatarIn 接收消息的玩家替身，为空则向全体玩家发送
		 * @param realTime 是否为实时消息，客户端接收到实时消息后需要播放提示动画
		 */
		void notifyGradePoint(GuanDanAvatar* avatarIn, bool realTime);

		/**
		 * 发送玩家手牌
		 * @param avatar 玩家替身
		 */
		void sendHandCard(GuanDanAvatar* avatar);

		// 生成进贡提示文本
		void makePresentTributeTip();

		// 生成还贡提示文本
		void makeRefundTributeTip();

		// 开始等待进贡
		void waitPresentTribute();

		/**
		 * 通知等待进贡
		 * @param elapsed 已等待了多久(毫秒)
		 * @param avatar 接收消息的玩家替身，为空则向全体玩家发送
		 */
		void notifyWaitPresentTribute(int elapsed, GuanDanAvatar* avatar);

		/**
		 * 执行进贡操作
		 * @param avatar 玩家替身
		 * @param card 进贡牌
		 */
		void doPresentTribute(GuanDanAvatar* avatar, const PokerCard& card);

		// 开始等待还贡
		void waitRefundTribute();

		/**
		 * 通知等待还贡
		 * @param elapsed 已等待了多久(毫秒)
		 * @param avatar 接收消息的玩家替身，为空则向全体玩家发送
		 */
		void notifyWaitRefundTribute(int elapsed, GuanDanAvatar* avatar);

		/**
		 * 执行还贡操作
		 * @param avatar 玩家替身
		 * @param card 还贡牌
		 */
		void doRefundTribute(GuanDanAvatar* avatar, const PokerCard& card);

		// 开始等待出牌
		void waitPlayCard();

		/**
		 * 执行出牌动作
		 * @param avatar 玩家替身
		 * @param ids 出牌id列表
		 * @param comb 出牌组合
		 * @return 是否成功出牌
		 */
		bool doPlayCard(GuanDanAvatar* avatar, const std::vector<int>& ids, const PokerCombination::Ptr& comb = PokerCombination::Ptr());

		/**
		 * 执行出牌操作
		 * @param avatar 玩家替身
		 * @param comb 出牌组合
		 * @return 是否成功出牌
		 */
		bool doPlayCard(GuanDanAvatar* avatar, const PokerCombination::Ptr& comb);

		/**
		 * 玩家过，即不要
		 * @param avatar 玩家替身
		 * @return 是否成功不要
		 */
		bool doPass(GuanDanAvatar* avatar);

		/**
		 * 通知等待出牌
		 * @param elapsed 已等待了多久(毫秒)
		 * @param avatar 接收消息的玩家替身，为空则向全体玩家发送
		 */
		void notifyWaitPlayCard(int elapsed, GuanDanAvatar* avatar) const;

		/**
		 * 通知出牌失败
		 * @param reason 失败原因代号
		 * @param avatar 玩家替身
		 */
		void notifyPlayCardFailed(int reason, GuanDanAvatar* avatar) const;

		/**
		 * 通知玩家剩余牌张数
		 * @param seat 目标玩家座位号
		 * @param avatarIn 接收消息的玩家替身，为空则向全体玩家发送
		 */
		void notifyCardNums(int seat, GuanDanAvatar* avatarIn) const;

		/**
		 * 通知警告玩家即将出完牌
		 * @param seat 即将出完牌的玩家座位号
		 */
		void notifyCardAlert(int seat) const;

		/**
		 * 通知玩家已出牌
		 * @param seat 出牌玩家座位号
		 * @param avatarIn 接收消息的玩家替身，为空则向全体玩家发送
		 * @param realTime 是否为实时消息，客户端接收到实时消息后需要播放动画(例如炸弹动画)
		 */
		void notifyPlayCard(int seat, GuanDanAvatar* avatarIn, bool realTime) const;

		/**
		 * 通知清除玩家已打出的牌
		 * 玩家出完牌之后再轮一圈发此通知
		 * @param seat 玩家座位号
		 */
		void notifyClearPlayedOut(int seat);

		/**
		 * 通知玩家出完牌
		 * @param avatarIn 接收消息的玩家替身，为空则向全体玩家发送
		 */
		void notifyFinished(GuanDanAvatar* avatarIn);

		/**
		 * 通知玩家获得借风出牌权
		 * @param seat 玩家座位号
		 */
		void notifyJieFeng(int seat);

		// 自动操作
		void autoExecute(time_t nowTick);

		// 自动进贡
		void autoPresentTribute(GuanDanAvatar* avatar);

		// 自动还贡
		void autoRefundTribute(GuanDanAvatar* avatar);

		// 自动出牌
		void autoPlayCard(GuanDanAvatar* avatar);

		/**
		 * 自动出牌时做过牌决策
		 * @param avatar 当前出牌玩家
		 * @param comb 出牌组合
		 * @return true-不要，false-压牌
		 */
		bool makePassDecision(GuanDanAvatar* avatar, const PokerCombination::Ptr& comb);

		/**
		 * 通知解散投票
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyDisbandVote(const std::string& playerId);

		// 一局结束
		void endRound();

		// 玩家选择是否解散房间
		void doDisbandChoose(int seat, int choise);

		// 解散房间
		void disbandRoom();

		// 取消解散
		void disbandObsolete();

		// 从数据库中删除游戏房间
		void removeRoomFromDb();

	private:
		/**
		 * 处理同步数据消息
		 * @param netMsg 网络消息
		 */
		void onSyncGuanDan(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家准备消息
		 * @param netMsg 网络消息
		 */
		void onPlayerReady(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家托管(委托后台自动出牌)消息
		 * @param netMsg 网络消息
		 */
		void onPlayerAuthorize(const NetMessage::Ptr& netMsg);

		/**
		 * 处理房主开始游戏消息
		 * @param netMsg 网络消息
		 */
		void onStartGame(const NetMessage::Ptr& netMsg);

		/**
		 * 处理进贡消息
		 * @param netMsg 网络消息
		 */
		void onPresentTribute(const NetMessage::Ptr& netMsg);

		/**
		 * 处理还贡消息
		 * @param netMsg 网络消息
		 */
		void onRefundTribute(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家出牌消息
		 * @param netMsg 网络消息
		 */
		void onPlayCard(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家请求提示出牌消息
		 * @param netMsg 网络消息
		 */
		void onHintCard(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家请求从头开始提示出牌消息
		 * @param netMsg 网络消息
		 */
		void onResetHintCard(const NetMessage::Ptr& netMsg);

		/**
		 * 处理玩家请求提示同花顺消息
		 * @param netMsg 网络消息
		 */
		void onHintStraightFlush(const NetMessage::Ptr& netMsg);

		/**
		 * 处理解散房间请求消息
		 * @param netMsg 网络消息
		 */
		void onDisbandRequest(const NetMessage::Ptr& netMsg);

		/**
		 * 处理解散房间选择消息
		 * @param netMsg 网络消息
		 */
		void onDisbandChoose(const NetMessage::Ptr& netMsg);

	private:
		// 规则
		std::shared_ptr<GuanDanRule> _rule;

		// 房间编号
		const std::string _number;

		// 房间等级
		const int _level;

		/**
		 * 房主座位号，无房主时为-1
		 */
		int _ownerSeat;

		/**
		 * 局号数，每局递增
		 */
		int _roundNo;

		// 发牌器
		std::shared_ptr<PokerDealer> _dealer;

		// 游戏状态
		GameState _gameState;

		// 进贡类型，0-不需要进贡（首局或者抗贡），1-单贡，2-双贡
		int _tribute;

		// 当前等待出牌的玩家座位
		int _current;

		// 最新出牌(非不要)的玩家座位
		int _lastOut;

		// 当前等待的操作
		WaitOperation _operation;

		// 红方(0、2座位号玩家固定为红方)当前级牌牌值(点数)
		int _gradePointRed;

		// 蓝方(1、3座位号玩家固定为蓝方)当前级牌牌值(点数)
		int _gradePointBlue;

		// 本局哪方当庄，1-红方，2-蓝方，首局为0即都不当庄
		int _banker;

		// 下一局哪方当庄，1-红方，2-蓝方，首局为0即都不当庄
		int _bankerNext;

		// 下一局级牌点数
		int _gradePointNext;

		// 等待进贡提示文本
		std::string _presentTributeTip;

		// 等待还贡提示文本
		std::string _refundTributeTip;

		// 进贡牌列表，first-进贡玩家座位号，second-进贡牌
		std::vector<std::pair<int, PokerCard> > _presentTributeCards;

		// 已还贡玩家数量
		int _refundPlayers;

		// 上一局出完牌的玩家次序，_lastFinishedSeats[0]为头游玩家座位号，依次类推
		int _lastFinishedSeats[4];

		// 本局出完牌的玩家次序，_finishedSeats[0]为头游玩家座位号，依次类推
		int _finishedSeats[4];

		// 一局结束后被踢出房间的玩家
		bool _kicks[4];

		// 是否正在解散房间
		bool _disbanding;

		// 请求解散房间的玩家座位号
		int _disbander;

		// 解散投票，0-未选择、1-同意、2-反对
		int _disbandChoises[4];

		// 上一次刷新区域内场地注册表的时间
		time_t _lastRegisterTick;

		// 开始等待操作的时间(超时执行默认操作)，单位毫秒
		time_t _waitTick;

		// 执行自动操作时间
		time_t _autoTick;

		// 开始投票解散的时间
		time_t _beginDisbandTick;

		// 结束投票解散的时间
		time_t _endDisbandTick;
	};
}

#endif // !_NIU_MA_GUAN_DAN_ROOM_H_