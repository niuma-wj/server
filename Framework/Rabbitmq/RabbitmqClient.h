// RabbitmqClient.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.12

#ifndef _NIU_MA_RABBITMQ_CLIENT_H_
#define _NIU_MA_RABBITMQ_CLIENT_H_

#include "Base/Singleton.h"
#include "RabbitmqConfig.h"

#include <atomic>
#include <thread>
#include <queue>
#include <functional>

namespace NiuMa {
	class PublishItem;

	/**
	 * RabbitMQ客户端
	 */
	class RabbitmqClient : public Singleton<RabbitmqClient> {
	private:
		RabbitmqClient();

	public:
		virtual ~RabbitmqClient() = default;

		friend class Singleton<RabbitmqClient>;
		friend class RabbitmqConfig;

	public:
		/**
		 * 启动RabbitMQ客户端
		 * @param host 主机地址
		 * @param port 端口
		 * @param vhost 虚拟主机
		 * @param username 用户名
		 * @param password 密码
		 * @param config RabbitMQ配置表，用于绑定、订阅等操作
		 * @param consumer 消息消费者
		 * @param 是否启动成功
		 */
		bool start(const std::string& host, int port, const std::string& vhost,
			const std::string& username, const std::string& password, const RabbitmqConfig::Ptr& config);

		/**
		 * 停止
		 * @param config RabbitMQ配置表，用于解绑、关闭等是否资源的操作
		 */
		void stop(const RabbitmqConfig::Ptr& config);

		// 查询连接是否就绪
		bool isConnectionOk() const;

		/**
		 * 发布消息
		 * @param exchange 交换机名称
		 * @param routingKey 路由键
		 * @param message 消息
		 * @return 是否发布成功
		 */
		bool publish(const std::string& exchange, const std::string& routingKey, const std::string& message);

		/**
		 * 发布Json消息
		 * @param exchange 交换机名称
		 * @param routingKey 路由键
		 * @param msgType 消息类型
		 * @param json 消息体
		 */
		bool publishJson(const std::string& exchange, const std::string& routingKey, const std::string& msgType, const std::string& json);

	private:
		// 检查连接
		bool checkConnection();

		//
		void onConnection(bool isOk);

		// 打开通道
		int openChannel();

		// 关闭通道
		void closeChannel(int channelId);

		// 获取发布通道id
		int getPublishChannel();

		// 设置发布通道id
		void setPublishChannel(int id);

		// 检查发布通道
		int checkPublish();

		// 发布函数
		bool publishInner();

		// 弹出发布消息
		std::shared_ptr<PublishItem> popPublishItem();

		// 线程函数
		void threadFunc();

	private:
		// RabbitMQ连接
		RabbitmqConnection::Ptr _connection;

		// 配置回调函数
		RabbitmqConfig::Ptr _config;

		// 停止标志
		std::atomic_bool _stopFlag;

		// 发布通道id，用于发布线程
		int _publishId;

		// 信号量
		std::mutex _mtx;

		// 线程
		std::shared_ptr<std::thread> _thread;

		// 发布消息队列
		std::queue<std::shared_ptr<PublishItem> > _publishItems;

		// 发布队列信号量
		std::mutex _mtxPublish;
	};
}

#endif