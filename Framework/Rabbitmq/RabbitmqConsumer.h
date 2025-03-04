// RabbitmqConsumer.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.16

#ifndef _NIU_MA_RABBITMQ_CONSUMER_H_
#define _NIU_MA_RABBITMQ_CONSUMER_H_

#include "Base/Singleton.h"
#include "RabbitmqMessageHandler.h"

#include <atomic>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace NiuMa {
	/**
	 * RabbitMQ消费者
	 */
	class RabbitmqConsumer : public Singleton<RabbitmqConsumer> {
	private:
		RabbitmqConsumer();

	public:
		virtual ~RabbitmqConsumer();
		friend class Singleton<RabbitmqConsumer>;

	public:
		/**
		 * 启动
		 */
		void start();

		/**
		 * 添加消息处理器
		 * @param handler 消息处理器
		 */
		void addHandler(const RabbitmqMessageHandler::Ptr& handler);

		/**
		 * 删除消息处理器
		 * @param handler 消费处理器
		 * @return true-已删除，false-不存在
		 */
		bool removeHandler(const RabbitmqMessageHandler::Ptr& handler);

		/**
		 * 消费消息
		 * @param message 消息体
		 * @param tag 消费标签
		 */
		void consume(const std::shared_ptr<std::string>& message, const std::string& tag);

	private:
		/**
		 * 获取消息处理器
		 * @param tag 消费标签
		 * @param handlers 返回消费标签对应的处理器列表
		 */
		void getHandlers(const std::string& tag, std::vector<RabbitmqMessageHandler::Ptr>& handlers);

		/**
		 * 判定指定消费标签是否已存在对应的消息处理器
		 * @param tag 消费标签
		 */
		bool hasHandler(const RabbitmqMessageHandler::Ptr& handler);

		/**
		 * 查询处理器表是否发生了变化
		 */
		bool isChanged();

		/**
		 * 同步处理器列表
		 */
		void syncHandlerList();

		/**
		 * 定时任务
		 */
		bool onTimer();

	private:
		// 启动标志
		std::atomic_bool _startFlag;

		// 定时任务持有者，当该持有者被销毁，则说明本单例实例被销毁
		// 定时任务可直接退出
		std::shared_ptr<int> _timer;

		// 消息处理器表
		// key-消费标签，value-消息处理器
		std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> > _handlers;

		// 消息处理器列表，用于在定时任务遍历
		std::vector<RabbitmqMessageHandler::Ptr> _handlerList;

		// 处理器表是否发生了变化
		bool _changed;

		// 信号量
		std::mutex _mtx;
	};
}

#endif // !_NIU_MA_RABBITMQ_CONSUMER_H_