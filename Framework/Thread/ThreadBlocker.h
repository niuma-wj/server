// ThreadBlocker.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.06

#ifndef _NIU_MA_THREAD_BLOCKER_H_
#define _NIU_MA_THREAD_BLOCKER_H_

#include <memory>
#include <mutex>
#include <condition_variable>

namespace NiuMa {
	// 线程阻塞器
	class ThreadBlocker : public std::enable_shared_from_this<ThreadBlocker> {
	public:
		ThreadBlocker();
		virtual ~ThreadBlocker() = default;

		typedef std::shared_ptr<ThreadBlocker> Ptr;

	public:
		// 阻塞当前线程
		void block();

		// 唤醒被本阻塞器阻塞的所有线程
		void signal();

		// 重置阻塞标志
		void reset();

	private:
		// 阻塞标志，为true时调用block方法会阻塞调用线程，默认为true
		bool _flag;

		// 信号量
		std::mutex _mtx;

		// 条件变量
		std::condition_variable _cond;
	};
}

#endif // !_NIU_MA_THREAD_BLOCKER_H_
