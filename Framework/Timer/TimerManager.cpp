// TimerManager.cpp

#include "TimerManager.h"
#include "Base/Log.h"
#include "Base/BaseUtils.h"

#include <list>
#include <unordered_map>

namespace NiuMa {
	/**
	 * 异步定时器类
	 */
	class AsyncTimer : public std::enable_shared_from_this<AsyncTimer> {
	public:
		AsyncTimer(int interval, const ThreadWorker::TimerHandler& handler)
			: _interval(interval)
			, _handler(handler)
		{
			setTriggerTime(BaseUtils::getCurrentMillisecond());
		}

		virtual ~AsyncTimer() = default;

		typedef std::shared_ptr<AsyncTimer> Ptr;

	public:
		// 返回定时器时间间隔
		int getInterval() const {
			return _interval;
		}

		/**
		 * 设置下一次触发时间
		 * @param nowTime 当前时间
		 */
		void setTriggerTime(const time_t& nowTime) {
			std::lock_guard<std::mutex> lck(_mtx);

			_triggerTime = nowTime + _interval;
		}

		/**
		 * 获取下一次触发时间
		 * @param ret 下一次触发时间
		 */
		void getTriggerTime(time_t& ret) {
			std::lock_guard<std::mutex> lck(_mtx);

			ret = _triggerTime;
		}

		/**
		 * 触发定时器
		 * @return 是否结束定时任务，true-结束，false-不结束
		 */
		bool trigger(const time_t& nowTime) {
			setTriggerTime(nowTime);
			return _handler();
		}

	private:
		// 触发间隔(毫秒)
		const int _interval;

		// 处理器
		ThreadWorker::TimerHandler _handler;

		// 下一次触发时间
		time_t _triggerTime;

		// 
		std::mutex _mtx;
	};

	/**
	 * 异步定时器持有者
	 */
	class AsyncTimerHolder {
	public:
		AsyncTimerHolder() {}
		virtual ~AsyncTimerHolder() {};

	public:
		bool addTimer(int interval, const ThreadWorker::TimerHandler& handler) {
			if (interval < 5) {
				LOG_ERROR("Interval time must be equal or greater to 5 millisecond.");
				return false;
			}
			std::lock_guard<std::mutex> lck(_mtx);

			AsyncTimer::Ptr timer = std::make_shared<AsyncTimer>(interval, handler);
			insert2SortedList(timer);
			return true;
		}

		AsyncTimer::Ptr captureFirstTimer(const time_t& nowTime) {
			AsyncTimer::Ptr timer;

			std::lock_guard<std::mutex> lck(_mtx);

			if (!_sortedList.empty())
				timer = _sortedList.front();
			if (timer) {
				time_t nextTime;
				timer->getTriggerTime(nextTime);
				if (nextTime > nowTime)
					timer.reset();
				else
					_sortedList.pop_front();
			}
			return timer;
		}

		void returnTimer(const AsyncTimer::Ptr& timer) {
			std::lock_guard<std::mutex> lck(_mtx);

			insert2SortedList(timer);
		}

		void clear() {
			std::lock_guard<std::mutex> lck(_mtx);

			_sortedList.clear();
		}

	private:
		void insert2SortedList(const AsyncTimer::Ptr& timer) {
			bool inserted = false;
			time_t time1;
			time_t time2;
			timer->getTriggerTime(time2);
			std::list<std::shared_ptr<AsyncTimer> >::iterator it = _sortedList.begin();
			while (it != _sortedList.end()) {
				AsyncTimer::Ptr& tmp = *it;
				tmp->getTriggerTime(time1);
				if (time1 > time2) {
					_sortedList.insert(it, timer);
					inserted = true;
					break;
				}
				++it;
			}
			if (!inserted)
				_sortedList.push_back(timer);
		}

	private:
		// 按下一次触发时间的先后排序的列表
		std::list<AsyncTimer::Ptr> _sortedList;

		// 
		std::mutex _mtx;
	};

	class TimerThreadWorker : public ThreadWorker {
	public:
		TimerThreadWorker(const ThreadStopFlag::Ptr& flag, const std::shared_ptr<AsyncTimerHolder>& holder)
			: ThreadWorker(flag)
			, _holder(holder)
		{}

		virtual ~TimerThreadWorker() = default;

	protected:
		virtual int oneLoopEx() override {
			time_t nowTime = BaseUtils::getCurrentMillisecond();
			AsyncTimer::Ptr timer;
			bool ret = false;
			while (true) {
				if (isStop())
					break;
				timer = _holder->captureFirstTimer(nowTime);
				if (!timer)
					break;
				try {
					ret = timer->trigger(nowTime);
				}
				catch (std::exception& ex) {
					ErrorS << "Trigger timer error: " << ex.what();
					ret = false;
				}
				if (ret)
					timer.reset();
				else
					_holder->returnTimer(timer);
			}
			// 等待1毫秒
			return 1;
		}

	private:
		std::shared_ptr<AsyncTimerHolder> _holder;
	};

	template<> TimerManager* Singleton<TimerManager>::_inst = nullptr;

	TimerManager::TimerManager() {
		_holder = std::make_shared<AsyncTimerHolder>();
	}

	void TimerManager::start(int threadNum) {
		if (threadNum < 1)
			threadNum = 1;
		else if (threadNum > 8)
			threadNum = 8;
		if (isStarted())
			return;
		ThreadStopFlag::Ptr flag = std::make_shared<ThreadStopFlag>();
		std::vector<ThreadWorker::Ptr> workers;
		for (int i = 0; i < threadNum; i++) {
			ThreadWorker::Ptr worker = std::make_shared<TimerThreadWorker>(flag, _holder);
			workers.push_back(worker);
		}
		ThreadPool::start(flag, workers);

		InfoS << "TimerManager started, threadNum: " << threadNum;
	}

	void TimerManager::stop() {
		ThreadPool::stop();

		_holder->clear();
	}

	bool TimerManager::addAsyncTimer(int interval, const ThreadWorker::TimerHandler& handler) {
		return _holder->addTimer(interval, handler);
	}
}