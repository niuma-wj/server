// TimerManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.30

#ifndef _NIU_MA_TIMER_MANAGER_H_
#define _NIU_MA_TIMER_MANAGER_H_

#include "Base/Singleton.h"
#include "Thread/ThreadPool.h"

#include <functional>

namespace NiuMa {
	class AsyncTimerHolder;
	class TimerManager : public ThreadPool, public Singleton<TimerManager> {
	private:
		TimerManager();

	public:
		virtual ~TimerManager() = default;

		friend class Singleton<TimerManager>;

	public:
		/**
		 * 启动
		 * @param threadNum 线程数量
		 */ 
		void start(int threadNum);

		/**
		 * 停止
		 */
		virtual void stop() override;

		/**
		 * 添加异步定时器
		 * @param interval 时间间隔(毫秒)，间隔必须大于等于5毫秒
		 * @param handler 定时任务处理器(当返回true时，结束定时任务)
		 * @return true-成功添加，false-添加失败
		 */
		bool addAsyncTimer(int interval, const ThreadWorker::TimerHandler& handler);

	private:
		std::shared_ptr<AsyncTimerHolder> _holder;
	};
}

#endif // !_NIU_MA_TIMER_MANAGER_H_