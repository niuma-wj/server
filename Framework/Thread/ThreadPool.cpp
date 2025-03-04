// ThreadPool.cpp

#include "ThreadPool.h"

namespace NiuMa {
	ThreadPool::ThreadPool()
		: _startFlag(false)
	{}

	void ThreadPool::start(const ThreadStopFlag::Ptr& flag, const std::vector<ThreadWorker::Ptr>& workers) {
		if (_startFlag)
			return;

		_startFlag = true;
		_stopFlag = flag;
		for (const ThreadWorker::Ptr& worker : workers) {
			std::shared_ptr<std::thread> thread = std::make_shared<std::thread>([worker]() {
				worker->run();
			});
			worker->setThreadId(thread->get_id());
			_threads.push_back(thread);
		}
	}

	void ThreadPool::stop() {
		if (!_startFlag)
			return;

		if (_stopFlag)
			_stopFlag->stop();

		for (const std::shared_ptr<std::thread>& thread : _threads) {
			thread->join();
		}
		_threads.clear();
		_startFlag = false;
	}

	bool ThreadPool::isStarted() {
		return _startFlag;
	}
}