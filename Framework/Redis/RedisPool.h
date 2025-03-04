// RedisPool.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.06

#ifndef _NIU_MA_REDIS_POOL_H_
#define _NIU_MA_REDIS_POOL_H_

#include "Base/Singleton.h"
#include <string>
#include <memory>
#include <vector>

namespace NiuMa {
	/**
	 * Redis连接池
	 */
	class RedisPoolImpl;
	class RedisPool : public Singleton<RedisPool> {
	private:
		RedisPool() = default;

	public:
		virtual ~RedisPool() = default;

		friend class Singleton<RedisPool>;

	public:
		/**
		 * 启动Redis连接池
		 * @param host Redis服务器主机地址
		 * @param port 端口号
		 * @param password 密码
		 * @param database 数据库号
		 * @param keepConnections 保持的活跃的最大连接数
		 * @param maxConnections 创建的最大连接数
		 */
		void start(const std::string& host, int port, const std::string& password, int database, int keepConnections, int maxConnections);

		/**
		 * 停止
		 */
		void stop();

		/**
		 * 获取指定键的值
		 * @param key 键名
		 * @param value 返回值
		 * @return 是否获取成功
		 */
		bool get(const std::string& key, std::string& value);

		/**
		 * 获取指定键的值
		 * @param key 键名
		 * @param value 返回值
		 * @return 是否获取成功
		 */
		bool get(const std::string& key, int64_t& value);

		/**
		 * 设置指定键的值
		 * @param key 键名
		 * @param value 键值
		 * @return 是否设置成功
		 */
		bool set(const std::string& key, const std::string& value);

		/**
		 * 设置指定键的值
		 * @param key 键名
		 * @param value 键值
		 * @return 是否设置成功
		 */
		bool set(const std::string& key, int64_t value);

		/**
		 * 获取哈希表指定字段的值
		 * @param key 键名
		 * @param field 字段名
		 * @param value 返回值
		 * @return 是否获取成功
		 */
		bool hget(const std::string& key, const std::string& field, std::string& value);

		/**
		 * 获取哈希表指定字段的值
		 * @param key 键名
		 * @param field 字段名
		 * @param value 返回值
		 * @return 是否获取成功
		 */
		bool hget(const std::string& key, const std::string& field, int64_t& value);

		/**
		 * 设置哈希表指定字段的值
		 * @param key 键名
		 * @param field 字段名
		 * @param value 字段值
		 * @return 是否设置成功
		 */
		bool hset(const std::string& key, const std::string& field, const std::string& value);

		/**
		 * 设置哈希表指定字段的值
		 * @param key 键名
		 * @param field 字段名
		 * @param value 字段值
		 * @return 是否设置成功
		 */
		bool hset(const std::string& key, const std::string& field, int64_t value);

		/**
		 * 判定哈希表是否存在指定字段
		 * @param key 键名
		 * @param field 字段名
		 * @param result 返回判定结果
		 * @return 是否判定成功
		 */
		bool hexists(const std::string& key, const std::string& field, bool& result);

		/**
		 * 获取哈希表中的所有字段名
		 * @param key 键名
		 * @param fields 返回字段名列表
		 * @return 是否获取成功
		 */
		bool hkeys(const std::string& key, std::vector<std::string>& fields);

		/**
		 * 删除指定键
		 * @param key 键名
		 * @return 是否删除成功
		 */
		bool del(const std::string& key);

		/**
		 * 删除哈希表指定字段
		 * @param key 键名
		 * @param field 字段名
		 * @return 是否删除成功
		 */
		bool hdel(const std::string& key, const std::string& field);

		/**
		 * 向集合中添加元素
		 * @param key 键名
		 * @param values 元素数组
		 * @return 是否添加成功
		 */
		bool sadd(const std::string& key, const std::vector<std::string>& values);

		/**
		 * 向集合中添加元素
		 * @param key 键名
		 * @param value 元素值
		 * @return 是否添加成功
		 */
		bool sadd(const std::string& key, const std::string& value);

		/**
		 * 删除集合中的元素
		 * @param key 键名
		 * @param values 元素数组
		 * @return 是否删除成功
		 */
		bool sremove(const std::string& key, const std::vector<std::string>& values);

		/**
		 * 删除集合中的元素
		 * @param key 键名
		 * @param value 元素
		 * @return 是否删除成功
		 */
		bool sremove(const std::string& key, const std::string& value);

		/**
		 * 判定元素是否为集合中成员
		 * @param key 键名
		 * @param value 元素
		 * @param result 返回判定结果
		 * @return 是否判定成功
		 */
		bool sismember(const std::string& key, const std::string& value, bool& result);

		/**
		 * 获取集合中的全部元素
		 * @param key 键名
		 * @param values 返回元素数组
		 * @return 是否获取成功
		 */
		bool smembers(const std::string& key, std::vector<std::string>& values);

		/**
		 * 设置指定键过期时间
		 * @param key 键名
		 * @param sec 过期时间(秒)
		 * @return 是否设置成功
		 */
		bool expire(const std::string& key, int sec);

		/**
		 * 设置指定键过期时间
		 * @param key 键名
		 * @param timeStamp 过期时间(Unix时间戳，精确到毫秒)
		 * @return 是否设置成功
		 */
		bool expireAt(const std::string& key, int64_t timeStamp);

		/**
		 * 增量设置指定键
		 * @param key 键名
		 * @param delta 增量
		 * @return 是否设置成功
		 */
		bool incrBy(const std::string& key, int delta);

		/**
		 * 增量设置哈希表指定字段
		 * @param key 键名
		 * @param field 字段名
		 * @param delta 增量
		 * @return 是否设置成功
		 */
		bool hincrBy(const std::string& key, const std::string& field, int delta);

		/**
		 * 向指定数组末尾添加成员
		 * @param key 键名
		 * @param value 成员值
		 * @return 是否添加成功
		 */
		bool rpush(const std::string& key, const std::string& value);

		/**
		 * 向指定数组末尾添加成员
		 * @param key 键名
		 * @param value 成员值
		 * @return 是否添加成功
		 */
		bool rpush(const std::string& key, int64_t value);

		/**
		 * 向指定数组末尾批量添加成员
		 * @param key 键名
		 * @param arr 成员列表
		 * @return 是否添加成功
		 */
		bool rpushv(const std::string& key, const std::vector<std::string>& arr);

		/**
		 * 向指定数组末尾批量添加成员
		 * @param key 键名
		 * @param arr 成员列表
		 * @return 是否添加成功
		 */
		bool rpushv(const std::string& key, const std::vector<int64_t>& arr);

		/**
		 * 向指定数组开头插入成员
		 * @param key 键名
		 * @param value 成员值
		 * @return 是否插入成功
		 */
		bool lpush(const std::string& key, const std::string& value);

		/**
		 * 向指定数组开头插入成员
		 * @param key 键名
		 * @param value 成员值
		 * @return 是否插入成功
		 */
		bool lpush(const std::string& key, int64_t value);

		/**
		 * 向指定数组开头批量插入成员
		 * @param key 键名
		 * @param arr 成员列表
		 * @return 是否插入成功
		 */
		bool lpushv(const std::string& key, const std::vector<std::string>& arr);

		/**
		 * 向指定数组开头批量插入成员
		 * @param key 键名
		 * @param arr 成员列表
		 * @return 是否插入成功
		 */
		bool lpushv(const std::string& key, const std::vector<int64_t>& arr);

		/**
		 * 获取指定数组的全部成员
		 * @param key 键名
		 * @param arr 返回数组成员
		 * @return 是否获取成功
		 */
		bool lrange(const std::string& key, std::vector<std::string>& arr);

		/**
		 * 获取指定数组的全部成员
		 * @param key 键名
		 * @param arr 返回数组成员
		 * @return 是否获取成功
		 */
		bool lrange(const std::string& key, std::vector<int64_t>& arr);

		/**
		 * 持久化指定键
		 * @param key 键名
		 * @return 是否设置成功
		 */
		bool persist(const std::string& key);

	private:
		std::shared_ptr<RedisPoolImpl> _impl;
	};
}

#endif // !_NIU_MA_REDIS_POOL_H_
