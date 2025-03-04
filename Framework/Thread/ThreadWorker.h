// ThreadWorker.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.29

#ifndef _NIU_MA_THREAD_WORKER_H_
#define _NIU_MA_THREAD_WORKER_H_

#include "ThreadDispatch.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <functional>
#include <thread>

namespace NiuMa {
	/**
	 * 线程结束标志
	 */
	class ThreadStopFlag {
	public:
		ThreadStopFlag();
		virtual ~ThreadStopFlag() = default;

		typedef std::shared_ptr<ThreadStopFlag> Ptr;

	public:
		void stop();
		bool isStop() const;

	private:
		std::atomic_bool _flag;
	};

	/**
	 * 线程工作者，一个线程工作者代表一个线程
	 */
	class ThreadPool;
	class SyncTimerHolder;
	class ThreadWorker : public std::enable_shared_from_this<ThreadWorker> {
	public:
		ThreadWorker(const ThreadStopFlag::Ptr& flag);
		virtual ~ThreadWorker() = default;

		typedef std::shared_ptr<ThreadWorker> Ptr;
		typedef std::function<void()> DispatchCB;
		typedef std::function<bool()> TimerHandler;

		friend class ThreadPool;

	private:
		/**
		 * 设置线程id
		 */
		void setThreadId(const std::thread::id& threadId);

		/**
		 * 线程循环 
		 */
		void run();

		/**
		 * 单次线程循环
		 */
		void oneLoop();

	protected:
		/**
		 * 判定是否为当前线程
		 */
		bool isCuurentThread() const;

		/**
		 * 工作线程循环开始前
		 * 执行线程初始化工作
		 */
		virtual void before();

		/**
		 * 工作线程循环结束后
		 * 执行线程结束操作
		 */
		virtual void after();

		/**
		 * 单次线程循环
		 * @return 返回接下来休眠多少时间(单位毫秒)
		 */
		virtual int oneLoopEx() = 0;

	public:
		/**
		 * 其他线程派遣任务给本线程(this为执行者)
		 * @param task 派遣任务
		 */
		void dispatch(const ThreadDispatch::Ptr& task);

		/**
		 * 其他线程派遣任务给本线程(this为执行者)
		 * @param cb 任务回调函数
		 */
		void dispatch(const DispatchCB& cb);

		/**
		 * 派遣任务已经被执行，执行者(其他线程)通知派遣者(本线程即this)做后续处理
		 * @param task 派遣任务
		 */
		void executed(const ThreadDispatch::Ptr& task);

		/**
		 * 获取当前等待执行的派遣任务数量
		 * @return 当前等待执行的派遣任务数量
		 */
		int getWattingDispatchNums();

		/**
		 * 添加同步定时器，即在本线程内执行的定时器任务
		 * @param interval 时间间隔(毫秒)，间隔必须大于等于5毫秒
		 * @param handler 定时任务处理器(当返回true时，结束定时任务)
		 * @return true-成功添加，false-添加失败
		 */
		bool addSyncTimer(int interval, const TimerHandler& handler);

	private:
		/**
		 * 弹出等待执行的派遣任务
		 * @return 等待执行的派遣任务
		 */
		ThreadDispatch::Ptr popWattingDispatch();

		/**
		 * 弹出已经执行的派遣任务
		 * @return 已经执行的派遣任务
		 */
		ThreadDispatch::Ptr popExecutedDispatch();

		/**
		 * 添加同步定时器
		 * @param interval 时间间隔(毫秒)，间隔必须大于等于5毫秒
		 * @param handler 定时任务处理器(当返回true时，结束定时任务)
		 * @return true-成功添加，false-添加失败
		 */
		void addSyncTimerImpl(int interval, const TimerHandler& handler);

	private:
		// 线程id
		std::thread::id _threadId;

		// 线程结束标志
		ThreadStopFlag::Ptr _stopFlag;

		// 等待执行的派遣任务（由其他线程派遣的任务列表）
		std::queue<ThreadDispatch::Ptr> _waittingDispatches;

		// 已经执行的派遣任务（由其他线程执行后返回的认为列表）
		std::queue<ThreadDispatch::Ptr> _exectuedDispatches;

		// 信号量
		std::mutex _mtxWaitting;

		// 信号量
		std::mutex _mtxExecuted;

		// 定时器持有者
		std::shared_ptr<SyncTimerHolder> _holder;
	};
}

#endif // !_NIU_MA_THREAD_WORKER_H_