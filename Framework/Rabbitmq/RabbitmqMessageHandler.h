// RabbitmqMessageHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.30

#ifndef _NIU_MA_RABBITMQ_MESSAGE_HANDLER_H_
#define _NIU_MA_RABBITMQ_MESSAGE_HANDLER_H_

#include <memory>
#include <string>
#include <list>
#include <mutex>

namespace NiuMa {
	/**
	 * RabbitMQ消息处理器接口
	 */
	class RabbitmqMessageHandler {
	public:
		RabbitmqMessageHandler(const std::string& tag);
		virtual ~RabbitmqMessageHandler();

		typedef std::shared_ptr<RabbitmqMessageHandler> Ptr;

	public:
		/**
		 * 获取消费标签
		 */
		const std::string& getTag() const;

		/**
		 * 处理消息
		 */
		void handle();

		/**
		 * 压入消息
		 */
		void push(const std::shared_ptr<std::string>& message);

	protected:
		/**
		 * 判定是否接收压入的消息，默认全不接收
		 * @param message 压入的消息体
		 * @return true-接收，false-不接收
		 */
		virtual bool receive(const std::string& message);

		/**
		 * 处理消息
		 * @param message 消息体
		 */
		virtual void handleImpl(const std::string& message) = 0;

	private:
		/**
		 * 弹出消息
		 * @return 消息
		 */
		std::shared_ptr<std::string> pop();

	private:
		// 消费标签
		const std::string _tag;

		// 消息队列
		std::list<std::shared_ptr<std::string> > _msgQueue;

		// 信号量
		std::mutex _mtx;
	};
}

#endif // !_NIU_MA_RABBITMQ_MESSAGE_HANDLER_H_