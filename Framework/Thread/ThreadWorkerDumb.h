// ThreadWorkerDumb.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.31

#ifndef _NIU_MA_THREAD_WORKER_DUMB_H_
#define _NIU_MA_THREAD_WORKER_DUMB_H_

#include "ThreadWorker.h"

namespace NiuMa {
	/**
	 * 线程循环不做任何事情的工作者
	 * 每次线程循环只是简单休眠指定时间长度(毫秒)
	 * 可用于仅执行线程间派遣任务的用例场景
	 */
	class ThreadWorkerDumb : public ThreadWorker {
	public:
		ThreadWorkerDumb(const ThreadStopFlag::Ptr& flag, int millisecond);
		virtual ~ThreadWorkerDumb() = default;

	protected:
		virtual int oneLoopEx() override;

	private:
		// 每次循环休眠时间长度(毫秒)，小于等于0则不休眠
		int _millisecond;
	};
}

#endif