// MessageThreadPool.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.29

#ifndef _NIU_MA_MESSAGE_THREAD_POOL_H_
#define _NIU_MA_MESSAGE_THREAD_POOL_H_

#include "Thread/ThreadPool.h"
#include "MessageHandler.h"

namespace NiuMa {
	/**
	 * 网络消息处理线程队列
	 */
	class MessageThreadPool : public ThreadPool {
	public:
		MessageThreadPool() = default;
		virtual ~MessageThreadPool() = default;

	public:
		/**
		 * 启动
		 * @param threadNum 线程数量
		 * @param handlers 消息处理器列表
		 */
		void start(int threadNum, const std::vector<MessageHandler::Ptr>& handlers);
	};
}

#endif // !_NIU_MA_MESSAGE_THREAD_POOL_H_