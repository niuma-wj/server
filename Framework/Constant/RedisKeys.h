// RedisKeys.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_REDIS_KEYS_H_
#define _NIU_MA_REDIS_KEYS_H_

#include <string>

namespace NiuMa {
	class RedisKeys {
	private:
		RedisKeys();

	public:
		virtual ~RedisKeys();

		// 玩家消息密钥。后根玩家id
		static const std::string PLAYER_MESSAGE_SECRET;

		// 玩家当前所在的场地id，后根玩家id
		static const std::string PLAYER_CURRENT_VENUE;

		// 玩家当前北授权进入的场地id。后根玩家id
		static const std::string PLAYER_AUTHORIZED_VENUE;

		// IP黑名单
		static const std::string IP_BLACKLIST;

		// 场地服务器当前玩家数量，后加服务器id
		static const std::string SERVER_PLAYER_COUNT;

		// 场地当前所在的服务器id，后跟场地id
		// 每个服务器实例都有唯一id，该id同时作为RabbitMQ玩家消息路由键
		static const std::string VENUE_SERVER_MAP;

		// 场地服务器(游戏服务器)集合
		// 该集合实际上是分布式系统的服务器注册表，该集合中包含了当前分布式系统中全部游戏服务器id
		static const std::string VENUE_SERVER_SET;

		// 场地内当前玩家数量，后加场地id
		static const std::string VENUE_PLAYER_COUNT;

		// 场地服务器访问地址，后加服务器id
		// 数据格式为ip:port，例如192.168.1.100:10086
		static const std::string SERVER_ACCESS_ADDRESS;

		// 场地服务器保活时间，后加服务器id
		// 值为unix时间戳，定时更新该数值以进行服务器保活
		static const std::string SERVER_KEEP_ALIVE;

		// 区域内的场地登记表，后加区域id
		// key-场地id，value-刷新时间，超过30秒不刷新认为该场地无效（例如后台停机维护之后留存在Redis中的数据）
		static const std::string DISTRICT_VENUE_REGISTER;

		// 区域内玩家人数未满的场地哈希表，后加区域id
		// key-场地id，value-玩家人数
		static const std::string DISTRICT_NOT_FULL_VENUES;

		// 区域内玩家进入过的场地哈希表，{0}-区域id，{1}-玩家id
		// 用于记录玩家进入的场地轨迹，以便为玩家切换区域内场地提供依据
		// key-场地id，value-离开时刻的unix时间戳
		static const std::string DISTRICT_PLAYER_TRACK;
	};
}

#endif // !_NIU_MA_REDIS_KEYS_H_