// ThreadWorker.cpp

#include "ThreadWorker.h"
#include "Base/Log.h"
#include "Base/BaseUtils.h"

#include <list>

namespace NiuMa {
	/**
	 * 同步定时器类
	 */
	class SyncTimer : public std::enable_shared_from_this<SyncTimer> {
	public:
		SyncTimer(int interval, const ThreadWorker::TimerHandler& handler)
			: _interval(interval)
			, _handler(handler)
		{
			setTriggerTime(BaseUtils::getCurrentMillisecond());
		}

		virtual ~SyncTimer() = default;

		typedef std::shared_ptr<SyncTimer> Ptr;

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
			_triggerTime = nowTime + _interval;
		}

		/**
		 * 获取下一次触发时间
		 * @param time 返回下一次触发时间
		 */
		void getTriggerTime(time_t& time) {
			time = _triggerTime;
		}

		/**
		 * 触发定时器
		 * @param nowTime 当前时间
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
	};

	/**
	 * 同步定时器持有者
	 */
	class SyncTimerHolder {
	public:
		SyncTimerHolder() {}
		virtual ~SyncTimerHolder() {};

	public:
		void addTimer(int interval, const ThreadWorker::TimerHandler& handler) {
			SyncTimer::Ptr timer = std::make_shared<SyncTimer>(interval, handler);
			insert2SortedList(timer);
		}

		SyncTimer::Ptr captureFirstTimer(const time_t& nowTime) {
			SyncTimer::Ptr timer;
			if (!_sortedList.empty())
				timer = _sortedList.front();
			if (timer) {
				time_t nextTime = 0;
				timer->getTriggerTime(nextTime);
				if (nextTime > nowTime)
					timer.reset();
				else
					_sortedList.pop_front();
			}
			return timer;
		}

		void returnTimer(const SyncTimer::Ptr& timer) {
			insert2SortedList(timer);
		}

		/**
		 * 返回距离下一次触发还需等待多久
		 * @param nowTime 当前时间
		 * @return 等待多久(毫秒)
		 */
		int delta(const time_t& nowTime) {
			if (_sortedList.empty())
				return 100;
			time_t nextTime = 0;
			const SyncTimer::Ptr& timer = _sortedList.front();
			timer->getTriggerTime(nextTime);
			time_t delta = nextTime - nowTime;
			int ret = static_cast<int>(delta);
			if (ret < 1)
				ret = 1;	// 最低等待1毫秒
			return ret;
		}

		void clear() {
			_sortedList.clear();
		}

	private:
		void insert2SortedList(const SyncTimer::Ptr& timer) {
			bool inserted = false;
			time_t nextTime1 = 0;
			time_t nextTime2 = 0;
			timer->getTriggerTime(nextTime2);
			std::list<std::shared_ptr<SyncTimer> >::iterator it = _sortedList.begin();
			while (it != _sortedList.end()) {
				SyncTimer::Ptr& tmp = *it;
				tmp->getTriggerTime(nextTime1);
				if (nextTime1 > nextTime2) {
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
		std::list<SyncTimer::Ptr> _sortedList;
	};

	ThreadStopFlag::ThreadStopFlag()
		: _flag(false)
	{}

	void ThreadStopFlag::stop() {
		_flag = true;
	}

	bool ThreadStopFlag::isStop() const {
		return _flag;
	}

	ThreadWorker::ThreadWorker(const ThreadStopFlag::Ptr& flag)
		: _stopFlag(flag)
	{
		_holder = std::make_shared<SyncTimerHolder>();
	}

	void ThreadWorker::setThreadId(const std::thread::id& threadId) {
		_threadId = threadId;
	}

	void ThreadWorker::run() {
		before();

		ThreadDispatch::Ptr disp;
		while (true) {
			if (_stopFlag && _stopFlag->isStop())
				break;
			while (true) {
				disp = popWattingDispatch();
				if (!disp)
					break;
				try {
					disp->execute();
				} catch (std::exception& ex) {
					ErrorS << "Execute dispatch error: " << ex.what();
				}
			}
			while (true) {
				disp = popExecutedDispatch();
				if (!disp)
					break;
				try {
					disp->onExecuted();
				} catch (std::exception& ex) {
					ErrorS << "onExecuted dispatch error: " << ex.what();
				}
			}
			try {
				oneLoop();
			}
			catch (std::exception& ex) {
				ErrorS << "Thread loop error: " << ex.what();
			}
		}
		after();
	}

	bool ThreadWorker::isCuurentThread() const {
		return (_threadId == std::this_thread::get_id());
	}

	void ThreadWorker::before() {}

	void ThreadWorker::after() {}

	void ThreadWorker::oneLoop() {
		time_t nowTime = BaseUtils::getCurrentMillisecond();
		SyncTimer::Ptr timer;
		bool ret = false;
		while (true) {
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
		int delta = _holder->delta(nowTime);
		int ms = oneLoopEx();
		if (delta > ms)
			delta = ms;
		if (delta > 0)
			std::this_thread::sleep_for(std::chrono::milliseconds(delta));
	}

	void ThreadWorker::dispatch(const ThreadDispatch::Ptr& task) {
		if (!task)
			return;

		std::lock_guard<std::mutex> lck(_mtxWaitting);

		_waittingDispatches.push(task);
	}

	void ThreadWorker::dispatch(const ThreadWorker::DispatchCB& cb) {
		class CallbackDispatch : public ThreadDispatch {
		public:
			CallbackDispatch(const ThreadWorker::DispatchCB& cb)
				: _cb(cb)
			{}

			virtual ~CallbackDispatch() {}

		protected:
			virtual void executeImpl() override {
				if (_cb)
					_cb();
			}

		private:
			ThreadWorker::DispatchCB _cb;
		};
		ThreadDispatch::Ptr task = std::make_shared<CallbackDispatch>(cb);
		dispatch(task);
	}

	void ThreadWorker::executed(const ThreadDispatch::Ptr& task) {
		if (!task)
			return;

		std::lock_guard<std::mutex> lck(_mtxExecuted);

		_exectuedDispatches.push(task);
	}

	ThreadDispatch::Ptr ThreadWorker::popWattingDispatch() {
		std::lock_guard<std::mutex> lck(_mtxWaitting);

		ThreadDispatch::Ptr ret;
		if (!_waittingDispatches.empty()) {
			ret = _waittingDispatches.front();
			_waittingDispatches.pop();
		}
		return ret;
	}

	ThreadDispatch::Ptr ThreadWorker::popExecutedDispatch() {
		std::lock_guard<std::mutex> lck(_mtxExecuted);

		ThreadDispatch::Ptr ret;
		if (!_exectuedDispatches.empty()) {
			ret = _exectuedDispatches.front();
			_exectuedDispatches.pop();
		}
		return ret;
	}

	int ThreadWorker::getWattingDispatchNums() {
		std::lock_guard<std::mutex> lck(_mtxWaitting);

		return static_cast<int>(_waittingDispatches.size());;
	}

	bool ThreadWorker::addSyncTimer(int interval, const ThreadWorker::TimerHandler& handler) {
		if (interval < 5) {
			LOG_ERROR("Interval time must be equal or greater to 5 millisecond.");
			return false;
		}
		if (isCuurentThread()) {
			addSyncTimerImpl(interval, handler);
		}
		else {
			std::weak_ptr<ThreadWorker> weakSelf = shared_from_this();
			dispatch([weakSelf, interval, handler] {
				ThreadWorker::Ptr strongSelf = weakSelf.lock();
				if (strongSelf)
					strongSelf->addSyncTimerImpl(interval, handler);
				});
		}
		return true;
	}

	void ThreadWorker::addSyncTimerImpl(int interval, const TimerHandler& handler) {
		_holder->addTimer(interval, handler);
	}
}