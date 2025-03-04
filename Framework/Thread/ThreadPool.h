// ThreadPool.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.29

#ifndef _NIU_MA_THREAD_POOL_H_
#define _NIU_MA_THREAD_POOL_H_

#include "ThreadWorker.h"
#include <vector>

namespace NiuMa {
	class ThreadPool : public std::enable_shared_from_this<ThreadPool> {
	public:
		ThreadPool();
		virtual ~ThreadPool() = default;

		typedef std::shared_ptr<ThreadPool> Ptr;

	public:
		/**
		 * 启动线程池
		 */
		void start(const ThreadStopFlag::Ptr& flag, const std::vector<ThreadWorker::Ptr>& workers);

		/**
		 * 结束线程池
		 */
		virtual void stop();

	protected:
		bool isStarted();

	private:
		// 启动标志
		std::atomic_bool _startFlag;
		
		// 结束标志
		ThreadStopFlag::Ptr _stopFlag;

		// 线程列表
		std::vector<std::shared_ptr<std::thread> > _threads;
	};
}

#endif // !_NIU_MA_THREAD_POOL_H_
