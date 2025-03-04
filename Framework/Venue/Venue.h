// Venue.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.25

#ifndef _NIU_MA_VENUE_H_
#define _NIU_MA_VENUE_H_

#include "Message/NetMessage.h"

#include <vector>

namespace NiuMa {
	/**
	 * 场地类
	 * 场地可以代表一个游戏中的一个场景(例如MMOPRG游戏中的一个城镇或者副本)，或者是游戏中的一个关卡，
	 * 或者是游戏的一个进行地点（例如棋牌类游戏中的一张棋牌桌，在这张棋牌桌上可以进行多局游戏）
	 */
	class VenueInnerHandler;
	class Venue : public std::enable_shared_from_this<Venue> {
	public:
		Venue(const std::string& id, int gameType);
		virtual ~Venue();

		typedef std::shared_ptr<Venue> Ptr;

	public:
		/////////////////////////////////
		// 以下方法可以在任意线程中调用，其余方法请务必在场地内消息处理线程中调用
		/**
		 * 获取场地id
		 */
		const std::string& getId() const;

		/**
		 * 获取游戏类型
		 */
		int getGameType() const;

		/**
		 * 设置场地内消息处理器
		 * @param handler 场地内消息处理器
		 */
		void setHandler(const std::shared_ptr<VenueInnerHandler>& handler);

		/**
		 * 获取场地内消息处理器
		 * @return 场地内消息处理器
		 */
		std::shared_ptr<VenueInnerHandler> getHandler();

		/////////////////////////////////

	public:
		/**
		 * 初始化
		 */
		virtual void initialize();

		/**
		 * 处理网络消息
		 * @param netMsg 网络消息
		 * @return 消息是否被处理
		 */
		virtual bool onMessage(const NetMessage::Ptr& netMsg);

		/**
		 * 30毫秒定时任务
		 */
		virtual void onTimer();

		/**
		 * 获取状态
		 * @return 状态
		 */
		int getStatus();

		/**
		 * 玩家进入场地
		 * @param playerId 玩家id
		 * @param base64 额外信息，打包成base64
		 * @param errMsg 错误消息
		 * @return true-进入成功，false-进入失败
		 */
		bool onEnter(const std::string& playerId, const std::string& base64, std::string& errMsg);

		/**
		 * 玩家离开场地
		 * @param playerId 玩家id
		 * @param errMsg 错误消息
		 * @return 0-离开成功，其他-离开失败
		 */
		int onLeave(const std::string& playerId, std::string& errMsg);

		/**
		 * 玩家重新连接到服务器
		 * @param playerId 玩家id
		 */
		virtual void onConnect(const std::string& playerId);

		/**
		 * 玩家连接断开
		 * @param playerId 玩家id
		 */
		virtual void onDisconnect(const std::string& playerId);

		/**
		 * 响应离开场地消息
		 * @param netMsg 网络消息
		 */
		void onLeaveVenue(const NetMessage::Ptr& netMsg);

		/**
		 * 场地是否已经废弃
		 * @return 是否已经废弃
		 */
		bool isObsolete() const;

		/**
		 * 获取废弃时间
		 * @return 废弃时间
		 */
		time_t getObsoleteTime() const;

	private:
		/**
		 * 响应玩家心跳消息
		 * @param netMsg 网络消息
		 */
		void onHeartbeat(const NetMessage::Ptr& netMsg);

		/**
		 * 向数据库更新场地状态
		 */
		void updateStatus2Db();

		/**
		 * 废弃场地，废弃30秒后将被销毁
		 */
		void setObsolete();

	protected:
		/**
		 * 判定是否存在指定玩家
		 * @param playerId 玩家id
		 */
		virtual bool hasPlayer(const std::string& playerId) = 0;

		/**
		 * 获取全部玩家id
		 * @param playerIds 玩家id数组
		 */
		virtual void getPlayerIds(std::vector<std::string>& playerIds) = 0;

		/**
		 * 获取场地内当前玩家数量
		 * @return 玩家数量
		 */
		virtual int getPlayerCount() = 0;

		/**
		 * 玩家进入场地
		 * @param playerId 玩家id
		 * @param base64 额外信息，打包成base64
		 * @param errMsg 错误消息
		 * @return true-进入成功，false-进入失败
		 */
		virtual bool enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) = 0;

		/**
		 * 玩家离开场地
		 * @param playerId 玩家id
		 * @param errMsg 错误消息
		 * @return 0-离开成功，其他-离开失败
		 */
		virtual int leaveImpl(const std::string& playerId, std::string& errMsg) = 0;

		/**
		 * 玩家心跳，默认什么都不做
		 * @param playerId 玩家id
		 */
		virtual void heartbeatImpl(const std::string& playerId);

		/**
		 * 玩家成功进入之后
		 * @param playerId 玩家id
		 */
		void afterEnter(const std::string& playerId);

		/**
		 * 玩家成功离开之后
		 * @param playerId 玩家id
		 */
		void afterLeave(const std::string& playerId);

		/**
		 * 更新玩家数量
		 * @return 当前玩家数量
		 */
		int updatePlayerCount();

		/**
		 * 游戏结束
		 */
		void gameOver();

	private:
		// 场地id
		const std::string _id;

		// 游戏类型
		const int _gameType;

		// 游戏状态，0-正常，1-场地已锁定，2-游戏已结束，3-游戏异常中止，只有正常状态的场地玩家才能进入
		int _status;

		// 场地内消息处理器
		std::weak_ptr<VenueInnerHandler> _handler;

		// 场地是否已经被废弃，废弃30秒后将被销毁
		bool _obsolete;

		// 废弃时间
		time_t _obsoleteTime;
	};
}

#endif // !_NIU_MA_VENUE_H_