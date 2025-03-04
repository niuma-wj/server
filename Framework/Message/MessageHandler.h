// MessageHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.19

#ifndef _NIU_MA_MESSAGE_HANDLER_H_
#define _NIU_MA_MESSAGE_HANDLER_H_

#include "NetMessage.h"
#include "Thread/ThreadWorker.h"

#include <queue>
#include <mutex>
#include <unordered_set>
#include <functional>

namespace NiuMa {
	/**
	 * 消息队列
	 */
	class MessageQueue {
	public:
		MessageQueue();
		virtual ~MessageQueue();

		typedef std::shared_ptr<MessageQueue> Ptr;

	public:
		// 压入消息
		void push(const NetMessage::Ptr& msg);

		// 弹出消息
		NetMessage::Ptr pop();

	private:
		// 消息队列
		std::queue<NetMessage::Ptr> _msgQueue;

		// 信号量
		std::mutex _mtx;
	};

	/**
	 * 消息处理器
	 * 一个消息处理器可能有独占的消息队列，也可能多个消息处理器共享一个消息队列
	 * 消息处理器共享消息队列的目的是为了支持同一类网络消息可以由多个消息处理器
	 * （对应多个线程）共同处理，这类网络消息的处理通常涉及阻塞操作，例如数据库
	 * 访问，多个线程共同处理可以提高系统并发
	 * 消息处理器一般在程序启动时创建固定数量的实例，且创建之后这些实例伴随着程
	 * 序的整个生命周期
	 */
	class MessageHandler : public std::enable_shared_from_this<MessageHandler>
	{
	public:
		/**
		 * 当queue不为空时，使用queue，此时queue一般作为多个消息处理共享的消息队列
		 * 当queue为空时，则创建内部独占的消息队列
		 */
		MessageHandler(const MessageQueue::Ptr& queue);
		virtual ~MessageHandler();

		typedef std::shared_ptr<MessageHandler> Ptr;

	public:
		// 注册自身
		void registSelf();

		/**
		 * 添加接收的消息类型
		 * 该函数仅在实例初始化时(例如构造函数中)调用
		 */
		void addMessage(const std::string& msgType);

		// 初始化
		virtual void initialize();

		// 接受网络消息
		virtual bool receive(const NetMessage::Ptr& netMsg) const;

		// 压入消息
		void push(const NetMessage::Ptr& msg);

		// 设置工作线程
		void setWorker(const ThreadWorker::Ptr& worker);

		// 获取工作线程
		ThreadWorker::Ptr getWorker() const;

		/**
		 * 处理消息
		 * @return 是否处理了消息
		 */
		bool handle();

	private:
		// 弹出消息
		NetMessage::Ptr pop();

	protected:
		/**
		 * 消息预处理
		 * @param netMsg 网络消息
		 * @return 是否预处理成功，若预处理失败则消息不会真正处理
		 */
		virtual bool preproccess(const NetMessage::Ptr& netMsg);

		/**
		 * 消息处理
		 * @param netMsg 网络消息
		 * @return 消息是否被处理
		 */
		virtual bool onMessage(const NetMessage::Ptr& netMsg);

	private:
		// 消息队列
		MessageQueue::Ptr _msgQueue;

	protected:
		// 工作线程
		std::weak_ptr<ThreadWorker> _worker;

		// 接收的消息类型表
		std::unordered_set<std::string> _recieveMessages;
	};
}

#endif // !_NIU_MA_MSG_RECIEVER_H_
