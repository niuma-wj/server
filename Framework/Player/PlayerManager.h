// PlayerManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_PLAYER_MANAGER_H_
#define _NIU_MA_PLAYER_MANAGER_H_

#include "Base/Singleton.h"
#include "Player.h"
#include <unordered_map>

namespace NiuMa {
	/**
	 * 玩家管理主
	 */
	class PlayerManager : public Singleton<PlayerManager> {
	private:
		PlayerManager();

	public:
		virtual ~PlayerManager();
		friend class Singleton<PlayerManager>;

	public:
		/**
		 * 初始化
		 */
		void init();

		/**
		 * 获取玩家，即引用玩家
		 * @param playerId 玩家id
		 * @return 玩家
		 */
		Player::Ptr getPlayer(const std::string& playerId) const;

		/**
		 * 从数据库加载玩家信息
		 * @param playerId 玩家id
		 * @return 玩家
		 */
		Player::Ptr loadPlayer(const std::string& playerId);

		/**
		 * 验证消息体玩家签名数据
		 * @param playerId 玩家id
		 * @param timestamp unix时间戳（单位秒）
		 * @param nonce 随机串
		 * @param signature md5签名
		 * @param outdate 返回消息是否过时
		 * @return 是否验证成功
		 */
		bool verifySignature(const std::string& playerId, const std::string& timestamp, const std::string& nonce, const std::string& signature, bool& outdate);

		/**
		 * 设置会话id对应的玩家id
		 * @param sessionId 会话id
		 * @param playerId 玩家id
		 */
		void setSessionPlayerId(const std::string& sessionId, const std::string& playerId);

		/**
		 * 将会话Id与玩家id撤销关联
		 * @param sessionId 会话id
		 */
		void removeSessionId(const std::string& sessionId);

		/**
		 * 通过会话id获取玩家id
		 * @param sessionId 会话id
		 * @param playerId 玩家id
		 * @return true-存在指定会话id，false-不存在
		 */
		bool getPlayerId(const std::string& sessionId, std::string& playerId);

		/**
		 * 通过会话id获取玩家
		 * @param sessionId 会话id
		 * @return 玩家
		 */
		Player::Ptr getPlayerBySessionId(const std::string& sessionId);

		/**
		 * 添加离线玩家
		 * @param playerId 玩家id
		 */
		void addOfflinePlayer(const std::string& playerId);

	private:
		/**
		 * 从数据库加载玩家数据
		 * @param player 玩家
		 * @return 是否加载成功
		 */
		bool loadPlayer(const Player::Ptr& player) const;

		/**
		 * 添加玩家
		 * @param player 玩家
		 */
		bool addPlayer(const Player::Ptr& player);

		/**
		 * 5秒钟定时器任务
		 */
		bool onTimer();

		/**
		 * 释放长时间不活动的玩家账户
		 */
		void freeDormantPlayers();

	private:
		// 定时任务持有者，当该持有者被销毁，则说明本单例实例被销毁
		// 定时任务可直接退出
		std::shared_ptr<int> _timer;

		// 玩家表
		std::unordered_map<std::string, Player::Ptr> _players;

		// 会话id与玩家id映射表
		// key-会话id, value-玩家id
		std::unordered_map<std::string, std::string> _sessionMap;

		// 离线玩家id列表
		std::list<std::string> _offlineIds;

		// 不精确的当时时间，每5秒更新一次，以减少频繁向系统获取当前时间的性能浪费
		// 因为每次引用玩家的时候都需要更新玩家的最新引用时间，而引用玩家是个高频动作
		time_t _inexactTime;

		// 信号量
		mutable std::mutex _mtx;
	};
}

#endif // !_NIU_MA_PLAYER_MANAGER_H_
