// SecurityManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_SECURITY_MANAGER_H_
#define _NIU_MA_SECURITY_MANAGER_H_

#include "Base/Singleton.h"

#include <string>
#include <ctime>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <atomic>

namespace NiuMa {
	/**
	 * 网络安全管理者
	 * 记录那些网络行为异常的ip地址，当在3秒内网络行为异常的次数达到20，则将该ip地址加入临时黑名单，
	 * 在临时黑名单内的ip地址在5分钟内不允许再次连接到服务器
	 */
	class AbnormalRecord;
	class SecurityManager : public Singleton<SecurityManager> {
	private:
		SecurityManager();

	public:
		virtual ~SecurityManager();
		friend class Singleton<SecurityManager>;

	public:
		/**
		 * 初始化
		 * @param fanoutExchange RabbitMQ广播交换机名称
		 * @param consumerTag RabbitMQ广播消费者标签
		 */
		void init(const std::string& fanoutExchange, const std::string& consumerTag);

		/**
		 * 记录一次网路异常行为
		 * @param remoteIp 产生异常行为的网络连接的远端ip
		 */
		void abnormalBehavior(const std::string& remoteIp);

		/**
		 * 检查远端ip是否在黑名单内
		 * @param remoteIp 产生异常行为的网络连接的远端ip
		 * @return true-在黑名单内，false-不在黑名单内
		 */
		bool checkBlacklist(const std::string& remoteIp);

	private:
		/**
		 * 获取指定远端ip的异常记录
		 * @param remoteIp 远端ip
		 * @param createIfNotExist 若不存在则新建
		 * @return 异常记录
		 */
		std::shared_ptr<AbnormalRecord> getRecord(const std::string& remoteIp, bool createIfNotExist);

		/**
		 * 将远端ip加入黑名单
		 * @param remoteIp 远端ip
		 * @param timestamp unix时间戳
		 */
		void add2Blacklist(const std::string& remoteIp, const time_t& timestamp);

		/**
		 * 从黑名单中删除ip
		 * @param remoteIp 远端ip
		 */
		void removeFromBlacklist(const std::string& remoteIp);

		/**
		 * 查询ip加入黑名单的时间戳
		 * @param remoteIp 远端ip
		 * @param timestamp 返回unix时间戳
		 * @return true-获取成功，false-获取失败(指定ip不在黑名单中)
		 */
		bool getTimestamp(const std::string& remoteIp, time_t& timestamp);

		/**
		 * 处理RabbitMQ消息
		 * @param msgType 消息类型
		 * @param json 消息体
		 */
		void handleMessage(const std::string& msgType, const std::string& json);

	private:
		// 异常记录表
		// key-远端ip，value-异常记录
		std::unordered_map<std::string, std::shared_ptr<AbnormalRecord> > _abnormalRecords;

		// 黑名单列表
		// key-远端ip，value-被加入黑名单的时间
		std::unordered_map<std::string, time_t> _blacklist;

		// 信号量
		std::mutex _mtx;

		// 初始化标志
		std::atomic_bool _initFlag;

		// RabbitMQ扇出交换机(广播消息)名称
		std::string _fanoutExchange;

		// RabbitMQ广播消费者标签
		std::string _consumerTag;
	};
}

#endif // !_NIU_MA_SECURITY_MANAGER_H_