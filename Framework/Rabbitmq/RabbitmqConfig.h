// RabbitmqConfig.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.15

#ifndef _NIU_MA_RABBITMQ_CONFIG_H_
#define _NIU_MA_RABBITMQ_CONFIG_H_

#include "RabbitmqConnection.h"

#include <string>
#include <memory>
#include <list>

namespace NiuMa {
	class ConfigItem;
	class RabbitmqClient;
	/**
	 * RabbitMQ配置
	 * RabbitMQ Server配置表，用于初始化和断线重连后重新配置Broker
	 */
	class RabbitmqConfig : public std::enable_shared_from_this<RabbitmqConfig> {
	public:
		RabbitmqConfig() = default;
		virtual ~RabbitmqConfig() = default;

		typedef std::shared_ptr<RabbitmqConfig> Ptr;
		friend class RabbitmqClient;

	public:
		/**
		 * 添加配置项：声明交换机(exchange)
		 * 注意，如果指定交换机名称已存在，则直接返回true，而不会重复声明
		 * @param exchange 交换机名称
		 * @param type 类型(fanout, direct, topic)，暂不支持header类型
		 * @param durable 是否永久化，永久化的交换机在RabbitMQ Server重启后不会丢失
		 * @param autoDelete 是否自动删除，true-当没有队列和交换器绑定且声明的连接断开时自动删除
		 * @param internal_ 是否内置的, true-Publisher无法直接发送消息到这个交换机中, 只能通过其他交换机路由到该交换机
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void exchangeDeclare(const std::string& exchange, const std::string& type, bool durable, bool autoDelete, bool internal_, bool redo = false);

		/**
		 * 添加配置项：将交换机绑定到交换机
		 * @param destination 目标交换机
		 * @param source 源交换机
		 * @param routingKey 路由键(定向或者通配符)
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void exchangeBind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo = false);

		/**
		 * 添加配置项：将交换机与交换机解除绑定
		 * @param destination 目标交换机
		 * @param source 源交换机
		 * @param routingKey 路由键(定向或者通配符)
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void exchangeUnbind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo = false);

		/**
		 * 添加配置项：删除交换机
		 * @param exchange 交换机名称
		 * @param ifUnused 是否在交换器没有使用的情况下删除, true-只有在交换器没有被使用的情况下才会被删除, false-强制删除
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void exchangeDelete(const std::string& exchange, bool ifUnused, bool redo = false);

		/**
		 * 添加配置项：声明队列(queue)
		 * 注意，如果指定队列名称已存在，则直接返回true，而不会重复声明
		 * @param queue 队列名称
		 * @param durable 是否永久化，永久化的交换机在RabbitMQ Server重启后不会丢失
		 * @param exclusive 该属性为true的队列只对首次声明它的连接可见，其他连接不能访问该队列，并且在连接断开时自动删除，即便durable为true
		 * @param autoDelete 是否自动删除，true-当没有通道与该队列绑定且声明的连接断开时自动删除
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void queueDeclare(const std::string& queue, bool durable, bool exclusive, bool autoDelete, bool redo = false);

		/**
		 * 添加配置项：将队列绑定到交换机
		 * @param queue 队列名称
		 * @param exchange 交换机名称
		 * @param routingKey 路由键(定向或者通配符)
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void queueBind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo = false);

		/**
		 * 添加配置项：将队列与交换机解除绑定，由Consumer调用
		 * @param queue 队列名称
		 * @param exchange 交换机名称
		 * @param routingKey 路由键(定向或者通配符)
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void queueUnbind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo = false);

		/**
		 * 添加配置项：删除队列
		 * @param queue 队列名称
		 * @param ifUnused 是否在队列没有使用的情况下删除, true-只有在队列没有被使用的情况下才会被删除, false-由ifEmpty决定
		 * @param ifEmpty 是否在队列为空的情况下删除, true-只有在队列为空的情况下才会被删除, false-由ifUnused决定
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void queueDelete(const std::string& queue, bool ifUnused, bool ifEmpty, bool redo = false);

		/**
		 * 添加配置项：订阅
		 * @param queue 队列名称
		 * @param tag 消费者标签
		 * @param noLocal
		 * @param noAck 是否需要返回ack
		 * @param exclusive
		 * @param redo 断线重连后是否需要重新执行该配置
		 */
		void queueConsume(const std::string& queue, const std::string& tag, bool noLocal = false, bool noAck = false, bool exclusive = false, bool redo = true);

	private:
		/**
		 * 执行配置
		 * @param conn rabbitmq连接
		 */
		bool config(const RabbitmqConnection::Ptr& conn);

	private:
		// 配置项列表
		std::list<std::shared_ptr<ConfigItem> > _items;
	};
}

#endif // !_NIU_MA_RABBITMQ_CONFIG_H_