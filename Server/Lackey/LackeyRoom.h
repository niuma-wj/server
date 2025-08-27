// LackeyRoom.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.31

#ifndef _NIU_MA_LACKEY_ROOM_H_
#define _NIU_MA_LACKEY_ROOM_H_

#include "Game/GameRoom.h"
#include "PokerDealer.h"

namespace NiuMa
{
	// 房间等级
	enum class LackeyRoomLevel : int
	{
		Invalid,	// 无效
		Friend,		// 好友房
		Beginner,	// 新手房
		Moderate,	// 初级房
		Advanced,	// 高级房
		Master		// 大师房
	};

	class LackeyRule;
	class LackeyAvatarEx;
	// 逮狗腿房间
	class LackeyRoom : public GameRoom
	{
	public:
		LackeyRoom(const std::shared_ptr<LackeyRule>& rule, const std::string& venueId, const std::string& number, int lvl, int mode, int diZhu);
		virtual ~LackeyRoom();

	public:
		// 游戏状态
		enum class GameState : int
		{
			Waiting,		// 正在等待游戏开始
			Dealing,		// 正在发牌
			Playing			// 游戏正在进行中...
		};

		// 等待玩家操作类型
		enum class WaitOperation : int
		{
			None,			// 无等待操作
			CallLackey,		// 等待地主叫狗腿
			ShowCard,		// 等待明牌
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
		virtual void initialize() override;
		virtual void onTimer() override;
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg, bool robot = false) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const override;
		virtual void onAvatarJoined(int seat, const std::string& playerId) override;
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;
		virtual void clean() override;
		virtual double* getDistances() override;
		virtual void getDistances(std::vector<int>& distances) const override;
		virtual int getDistanceIndex(int seat1, int seat2) const override;

	private:
		// 返回当前房间所属的区域id
		int getDistrictId() const;

		// 返回等待操作已过了多久
		int getWaitElapsed() const;

		// 开始发牌
		void beginDeal();

		// 获取局号数
		void getRoundNo();

		// 开始进入GameState::Playing状态
		void beginPlay();

		// 扣钻
		void deductDiamond();

		// 查找狗腿牌
		bool findLackeyCard(const CardArray& cards);

		/**
		 * 等待叫狗腿
		 */
		void waitCallLackey();

		/**
		 * 等待明牌
		 */
		void waitShowCard();

		/**
		 * 等待出牌
		 */
		void waitPlayCard();
		// 超时自动出牌
		void timePlayCard();
		// 托管玩家自动出牌
		void authPlayCard();
		// 返回指定玩家的阵营，true:地主阵营，false:农民阵营
		bool getCamp(int seat) const;
		// 为玩家1计算玩家2是否他的敌对阵营，0-未知、1-敌对、2-同阵营
		int getRivalry(int seat1, int seat2) const;
		// 一局结束
		void endRound();
		// 算分
		void calcScore();
		// 输赢付钱
		void doPayment();
		// 保存牌局记录
		void saveRoundRecord();
		// 踢出玩家
		void kickAvatars();

	private:
		/**
		 * 处理同步数据消息
		 * @param netMsg 网络消息
		 */
		void onSyncLackey(const NetMessage::Ptr& netMsg);

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
		 * 通知玩家托管消息
		 * @param seat 玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyPlayerAuthorize(int seat, const std::string& playerId) const;

		/**
		 * 发送输赢数据
		 * @param seat 玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void sendWinLose(int seat, const std::string& playerId) const;

		/**
		 * 发送手牌
		 * @param seat 玩家座位号
		 */
		void sendHandCard(int seat) const;

		/**
		 * 通知等待动作
		 * @param elapsed 已等待多久(毫秒)
		 * @param duration 等待多久(毫秒)
		 * @param playerId 接收消息的玩家id
		 */
		void notifyWaitOption(int elapsed, int duration, const std::string& playerId) const;

		/**
		 * 询问地主是否叫狗腿
		 * @param elapsed 已等待多久(毫秒)
		 */
		void notifyCallLackey(int elapsed) const;

		/**
		 * 处理地主叫狗腿消息
		 * @param netMsg 网络消息
		 */
		void onCallLackey(const NetMessage::Ptr& netMsg);

		/**
		 * 执行叫狗腿操作
		 * @param yes 是否叫狗腿
		 */
		void doCallLackey(bool yes);

		/**
		 * 通知狗腿牌
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyLackeyCard(const std::string& playerId) const;

		/**
		 * 通知狗腿座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 * @param playSound 是否播放狗腿露面声音
		 */
		void notifyLackeySeat(const std::string& playerId, bool playSound) const;

		/**
		 * 通知等待明牌
		 * @param elapsed 已等待多久(毫秒)
		 */
		void notifyWaitShowCard(int elapsed) const;

		/**
		 * 处理玩家执行明牌动作消息
		 * @param netMsg 网络消息
		 */
		void onShowCard(const NetMessage::Ptr& netMsg);

		/**
		 * 执行明牌操作
		 * @param yes 是否明牌
		 */ 
		void doShowCard(bool yes);

		/**
		 * 通知玩家已明牌
		 * @param seat 明牌玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyShowCard(int seat, const std::string& playerId) const;

		/**
		 * 通知当前玩家等待出牌
		 * @param elapsed 已等待多久(毫秒)
		 */
		void notifyWaitPlayCard(int elapsed) const;

		/**
		 * 处理玩家出牌消息
		 * @param netMsg 网络消息
		 */
		void onPlayCard(const NetMessage::Ptr& netMsg);

		/**
		 * 执行出牌动作
		 * @param pass 是否过(即不要)
		 * @param ids 出牌id列表
		 */
		bool doPlayCard(bool pass, const std::vector<int>& ids);

		/**
		 * 自动出牌
		 */
		void autoPlayCard();

		/**
		 * 通知出牌失败
		 * @param reason 失败原因代号
		 * @param avatar 玩家替身
		 */
		void notifyPlayCardFailed(int reason, LackeyAvatarEx* avatar) const;

		/**
		 * 通知玩家已出牌
		 * @param seat 玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 * @param xiQian 打出牌型获得的喜钱数量
		 * @param realTime 是否为实时消息，客户端接收到实时消息后需要播放动画(例如炸弹动画)
		 */
		void notifyPlayCard(int seat, const std::string& playerId, int xiQian, bool realTime) const;

		/**
		 * 通知玩家出牌获得喜钱
		 * @param seat 玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 * @param realTime 是否为实时消息，客户端接收到实时消息后需要播放动画(例如炸弹动画)
		 */
		void notifyXiQian(int seat, const std::string& playerId, bool realTime) const;

		/**
		 * 通知玩家当前剩余多少张牌未出
		 * @param seat 玩家座位号
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyCardNums(int seat, const std::string& playerId) const;

		/**
		 * 通知警告玩家即将出完牌
		 * @param seat 即将出完牌的玩家座位号
		 */
		void notifyCardAlert(int seat) const;

		/**
		 * 处理玩家请求提示出牌消息
		 * @param netMsg 网络消息
		 */
		void onHintCard(const NetMessage::Ptr& netMsg);

		/**
		 * 发送所有玩家手上剩余未打出的牌
		 */
		void sendLeftCards() const;

		/**
		 * 发送结算结果
		 */
		void sendResult() const;

		/**
		 * 处理解散房间请求消息
		 * @param netMsg 网络消息
		 */
		void onDisbandRequest(const NetMessage::Ptr& netMsg);

		/**
		 * 通知解散投票
		 * @param playerId 接收消息的玩家id，为空则向全体玩家发送
		 */
		void notifyDisbandVote(const std::string& playerId);

		/**
		 * 处理解散房间选择消息
		 * @param netMsg 网络消息
		 */
		void onDisbandChoose(const NetMessage::Ptr& netMsg);
		void doDisbandChoose(int seat, int choise);
		void disbandRoom();
		void disbandObsolete();

	private:
		// 规则
		const std::shared_ptr<LackeyRule> _rule;

		// 房间编号
		const std::string _number;
		
		// 房间等级
		const int _level;

		// 模式，0-扣钻、非0-抽水
		const int _mode;

		// 底注
		const int _diZhu;

		/**
		 * 局号数，每局递增
		 */
		int _roundNo;

		// 发牌器
		PokerDealer _dealer;

		// 游戏状态
		GameState _gameState;

		// 当前等待操作的玩家座位
		int _current;

		// 最新出牌(非不要)的玩家座位
		int _lastOut;

		// 当前等待的操作
		WaitOperation _operation;

		// 地主座位号
		int _landlord;

		// 狗腿座位号(当地主1打4时，该值为地主座位号)
		int _lackey;

		// 狗腿牌ID
		int _lackeyCard;

		// 狗腿的牌ID(当地主拿到了默认的狗腿牌时，有机会选择1打4或者以此牌叫狗腿，当没拿到默认狗腿牌时，该值为-1)
		int _callLackey;

		// 等待明牌的时长(地主若无机会1v4，则等待60秒，否则等待30秒，另30秒用于等待是否叫狗腿)
		int _showCardDuration;

		// 倍率
		int _beiLv;

		// 地主阵营是否明牌
		bool _showCard1;

		// 农民阵营是否明牌
		bool _showCard2;

		// 狗腿是否已经现身
		bool _showLackey;

		// 当前是否为第一轮出牌
		bool _firstCircle;

		// 一局结束后被踢出房间的玩家
		bool _kicks[5];

		// 是否正在解散房间
		bool _disbanding;

		// 请求解散房间的玩家座位号
		int _disbander;

		// 解散投票，0-未选择、1-同意、2-反对
		int _disbandChoises[5];

		// 上一次刷新区域内场地注册表的时间
		time_t _lastRegisterTick;

		// 开始等待操作的时间(超时执行默认操作)，单位毫秒
		time_t _waitTick;

		// 开始投票解散的时间
		time_t _beginDisbandTick;

		// 结束投票解散的时间
		time_t _endDisbandTick;

	private:
		// 尺表
		static const int RULER_TABLE[10];

		// 所有玩家之间的距离
		double _distances[10];
	};
}

#endif