// GameRoom.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.22

#ifndef _NIU_MA_GAME_ROOM_H_
#define _NIU_MA_GAME_ROOM_H_

#include "Venue/Venue.h"
#include "GameAvatar.h"
#include "Message/MsgBase.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

// 最大座位数量，预留20个座位无论如何都是够了的
#define MAX_SEAT_NUMS 20

namespace NiuMa {
	/**
	 * 游戏房间代表了一个完整独立的棋牌类游戏环境，游戏房内存在两种角色：玩家和观众，前者加入游
	 * 戏，而后者仅是旁观。
	 * 游戏房有两类：一类是严格限定玩家人数，并且玩家加入游戏后，就拥有了一个固定的座位号，典型
	 * 的例子如麻将游戏，玩家的座位号0~3依次代表东南西北各家；另一类则不限定玩家数量，玩家不拥
	 * 有座位号，游戏规则与座位无关，典型的例子如自由博彩，玩家可以自由选择当局下注或不下注。平
	 * 台将第一类游戏房定义为A类游戏房，第二类游戏房定义为B类游戏房。
	 * 对于A类游戏房，游戏房上可能会存在玩家和观众两种角色：用户进入游戏房后如果加入游戏即为玩
	 * 家，如果用户进入A类房后不加入游戏则是观众。有些A类房允许观众存在，而有些A类房则不允许，
	 * 对于不允许观众存在的A类房，用户进入房间必须立即加入游戏成为玩家。
	 * 对于B类游戏房，房内上仅有一种角色：即玩家，用户只要进入B类房就会成为玩家，B类房没有观众
	 * 这个角色，是因为玩家实际上同时已经是观众了(例如只要他不参与下注就是一名观众)。
	 * 另外，一般清况下不要锁定B类游戏房，因为B类房一般是来往没有限制，在锁定之后其他玩家就无法
	 * 加入B类房了（锁定房间将Venue::_status字段置1），对于允许观众存在的A类房一般也不进行锁定。
	 */
	enum class RoomCategory : int
	{
		RoomCategoryA = 0,		// A类房
		RoomCategoryB			// B类房
	};

	/**
	 * 棋牌类游房间基类
	 */
	class GameRoom : public Venue {
	public:
		GameRoom(const std::string& id, int gameType, int maxPlayerNums, RoomCategory category = RoomCategory::RoomCategoryA);
		virtual ~GameRoom();

	public:
		RoomCategory getCategory() const;
		int getMaxPlayerNums() const;
		int64_t getCashPledge() const;
		int64_t getDiamondNeed() const;
		GameAvatar::Ptr getAvatar(int seat) const;
		GameAvatar::Ptr getAvatar(const std::string& playerId) const;
		int getAvatarCount() const;
		bool hasSpectator(const std::string& playerId) const;

	protected:
		/**
		 * 获取全部玩家替身
		 */
		const std::unordered_map<std::string, GameAvatar::Ptr>& getAllAvatars() const;

		/**
		 * 设置押金
		 * @param cashPledge 押金数量
		 */
		void setCashPledge(int64_t cashPledge);

		/**
		 * 设置每局钻石扣数
		 * @param diamondNeed 钻石数量
		 */
		void setDiamondNeed(int64_t diamondNeed);

		/**
		 * 查找空位
		 * @return 空位索引，返回-1表示当前无空位
		 */
		int getEmptySeat() const;

		/**
		 * 人数是否已满
		 */
		bool isFull() const;

		/**
		 * 是否全部玩家已就绪
		 */
		bool isAllReady() const;

		/**
		 * 归还玩家押金
		 * @param playerId 玩家id
		 * @return 是否归还成功
		 */
		bool returnCashPledge(const std::string& playerId) const;

		/**
		 * 从玩家金币中补充扣除押金，若不足够扣除则将全部押金转为玩家金币并将玩家踢出游戏房
		 * @param avatar 玩家替身
		 * @return 是否补充扣除成功，不成功可能是金币不足或者数据库错误
		 */
		bool deductCashPledge(const GameAvatar::Ptr& avatar) const;

		/**
		 * 从玩家金币中补充扣除押金
		 * @param avatar 玩家替身
		 * @param goldNeed 需要扣除的目标押金总数，注意不是补充扣除增量
		 * @param kick true：若不足够扣除则将全部押金转为玩家金币并将玩家踢出游戏房，false：扣除失败
		 * @return 是否补充扣除成功，不成功可能是金币不足或者数据库错误
		 */
		bool deductCashPledge(const GameAvatar::Ptr& avatar, int64_t goldNeed, bool kick) const;

		/**
		 * 更新数据库中的押金数量
		 * @param playerId 玩家id
		 * @param cashPledge 押金数量，当该值为0时删除数据库中的押金记录
		 * @return 是否更新成功
		 */
		bool updateCashPledge(const std::string& playerId, int64_t cashPledge) const;

		/**
		 * 扣除玩家钻石
		 * @param playerId 玩家id
		 * @param diamond 返回扣除后玩家的钻石数量
		 * @return 是否扣除成功
		 */
		bool deductDiamond(const std::string& playerId, int64_t& diamond);

		/**
		 * 奖励玩家代理
		 * @param playerId 玩家id
		 * @param reward 奖励金额
		 * @param winGold 玩家赢得的金币数量
		 */
		void rewardAgency(const std::string& playerId, double reward, int64_t winGold);

		/**
		 * 踢出玩家
		 * @param avatar 玩家替身
		 */
		void kickAvatar(const GameAvatar::Ptr& avatar);

		/**
		 * 踢出全部玩家，一般用于解散游戏房间
		 */
		void kickAllAvatars();

		/**
		 * 计算指定玩家与所有其他玩家之间的地理距离
		 * @param seat 玩家座位号
		 * @param distances 距离数值数组
		 */
		void calcDistances(int seat, double* distances);

		/**
		 * 清除指定玩家与所有其他玩家之间的距离数值
		 * @param seat 座位号
		 * @param distances 距离数值数组
		 */
		void clearDistances(int seat, double* distances);

	public:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;
		virtual void onConnect(const std::string& playerId) override;
		virtual void onDisconnect(const std::string& playerId) override;

	protected:
		// 包括观众，即当playerId为观众时也返回true
		virtual bool hasPlayer(const std::string& playerId) override;

		// 包括观众
		virtual void getPlayerIds(std::vector<std::string>& playerIds) override;

		// 包括观众
		virtual int getPlayerCount() override;

		//
		virtual bool enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) override;

		//
		virtual int leaveImpl(const std::string& playerId, std::string& errMsg) override;

	protected:
		/**
		 * 检查指定玩家是否满足进入房间的条件
		 * @param playerId 玩家id
		 * @param errMsg 错误消息
		 * @return 是否满足进入房间的条件
		 */
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg) const = 0;

		/**
		 * 检查指定玩家当前是否可以离开房间
		 * @param playerId 玩家id
		 * @param errMsg 错误消息
		 * @return 0-离开成功，其他-离开失败
		 */
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const = 0;

		/**
		 * 创建玩家替身
		 * @param playerId 玩家id
		 * @param seat 座位号
		 * @param robot 是否为机器人
		 * @param 玩家替身
		 */
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const = 0;

		/**
		 * 是否允许观众存在，默认不允许
		 * 当允许观众存在时，进入房间首先成为观众，否则进入房间必须立即加入游戏成为玩家
		 * 对于B类游戏房固定返回false，不要重写(override)该方法
		 * @return true-允许，false-不允许
		 */
		virtual bool enableSpectator() const;

		/**
		 * 检查指定玩家是否满足加入游戏的条件，默认返回true
		 * 该方法与checkEnter不同之处在于，该方法用于允许观众存在的游戏房，在观众后续想要
		 * 加入游戏时作条件检测，对于不允许观众存在的游戏房，通常不需要重写(override)该方
		 * 法，只需要在checkEnter方法中作条件检测即可
		 * @param seat 座位号
		 * @param playerId 玩家id
		 * @param errMsg 错误消息
		 * @return 是否满足加入游戏的条件
		 */
		virtual bool checkJoin(int seat, const std::string& playerId, std::string& errMsg);

		/**
		 * 通知添加玩家
		 * @param seat 座位号
		 * @param playerId 玩家id
		 */
		virtual void notifyAddAvatar(int seat, const std::string& playerId);

		/**
		 * 通知删除玩家
		 * @param seat 玩家座位号
		 * @param playerId 玩家id
		 */
		virtual void notifyRemoveAvatar(int seat, const std::string& playerId);

		/**
		 * 通知添加观众
		 * @param playerId 玩家id
		 */
		virtual void notifyAddSpectator(const std::string& playerId);

		/**
		 * 通知删除观众
		 * @param playerId 玩家id
		 */
		virtual void notifyRemoveSpectator(const std::string& playerId);

		/**
		 * 获取玩家额外信息
		 * @param avatar 玩家替身
		 * @param base64 额外信息json打包成base64
		 */
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const;

		/**
		 * 获取观众额外信息
		 * @param playerId 玩家id
		 * @param base64 额外信息json打包成base64
		 */
		virtual void getSpectatorExtraInfo(const std::string& playerId, std::string& base64) const;

		/**
		 * 新玩家加入后的上层逻辑处理
		 * @param seat 座位号
		 * @param playerId 玩家id
		 */
		virtual void onAvatarJoined(int seat, const std::string& playerId);

		/**
		 * 玩家离开后(包括主动离开和被踢开)的上层逻辑处理
		 * @param seat 座位号
		 * @param playerId 玩家id
		 */
		virtual void onAvatarLeaved(int seat, const std::string& playerId);

		/**
		 * 清理游戏房间
		 */
		virtual void clean();

		/**
		 * 获取玩家之间距离数值数组
		 * @return 玩家之间距离数值数组
		 */
		virtual double* getDistances();

		/**
		 * 获取玩家间的距离，单位米
		 */
		virtual void getDistances(std::vector<int>& distances) const;

		/**
		 * 获取两个玩家之间的距离值在_distances数组中的索引
		 * 默认返回-1
		 * @param seat1 座位号1
		 * @param seat2 座位号2
		 */
		virtual int getDistanceIndex(int seat1, int seat2) const;

	protected:
		/**
		 * 向指定连接发送全部玩家数据
		 * @param session 连接会话
		 */
		void sendAvatars(const Session::Ptr& session) const;

		/**
		 * 向指定玩家发送消息
		 * @param msg 消息
		 * @param playerId 玩家id
		 */
		void sendMessage(const MsgBase& msg, const std::string& playerId) const;

		/**
		 * 向全部玩家发送消息
		 * @param userExcept 被排除的玩家
		 * @param spectator 是否发送到观众
		 */
		void sendMessageToAll(const MsgBase& msg, const std::string& playerExcepted = std::string(), bool spectator = false) const;

	private:
		/**
		 * 加入游戏(创建GameAvatar)
		 * @param seat 座位号
		 * @param playerId 玩家id
		 * @param errMsg 返回错误消息
		 * @return 是否加入成功
		 */
		bool joinGame(int seat, const std::string& playerId, std::string& errMsg);

		/**
		 * 添加玩家替身
		 */
		void addAvatar(const GameAvatar::Ptr& avatar);

		/**
		 * 删除玩家替身
		 * @param playerId 玩家id
		 */
		void removeAvatar(const std::string& playerId);

		/**
		 * 删除观众
		 * @param playerId 玩家id
		 */
		void removeSpectator(const std::string& playerId);

		/**
		 * 响应获取全部玩家数据消息
		 * @param netMsg 网络消息
		 */ 
		void onGetAvatars(const NetMessage::Ptr& netMsg);

		/**
		 * 响应获取全部观众数据消息
		 * @param netMsg 网络消息
		 */
		void onGetSpectators(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家定位消息
		 */
		void onPlayerGeolocation(const NetMessage::Ptr& netMsg);

		/**
		 * 查询所有玩家之间的距离
		 */
		void onDistancesRequest(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家发送聊天消息
		 * @param netMsg 网络消息
		 */
		void onChatClient(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家投掷特效消息
		 * @param netMsg 网络消息
		 */
		void onEffectClient(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家发送语音消息
		 * @param netMsg 网络消息
		 */
		void onVoiceClient(const NetMessage::Ptr& netMsg);

	private:
		// 游戏房类别(A类房、B类房)
		const RoomCategory _category;

		// A类房上的最大玩家数量(例如麻将游戏一般最多4人玩)，0表示无限制
		const int _maxPlayerNums;

		// 押金数额
		int64_t _cashPledge;

		// 加入游戏需要钻石数量
		int64_t _diamondNeed;

		// 座位上的玩家
		GameAvatar::Ptr _avatarSeats[MAX_SEAT_NUMS];

		// 全部玩家替身
		// key-玩家id, value-玩家替身
		std::unordered_map<std::string, GameAvatar::Ptr> _allAvatars;

		// A类房上的全部观众玩家id
		// 注意，游戏房不为旁观者创建玩家替身，只记录旁观者的玩家id
		std::unordered_set<std::string> _spectators;
	};
}

#endif // !_NIU_MA_GAME_ROOM_H_