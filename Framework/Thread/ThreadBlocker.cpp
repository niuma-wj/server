// ThreadBlocker.cpp

#include "ThreadBlocker.h"

namespace NiuMa {
	ThreadBlocker::ThreadBlocker()
		: _flag(true)
	{}

	void ThreadBlocker::block() {
		std::unique_lock<std::mutex> lock(_mtx);

		while (_flag) {
			_cond.wait(lock); // 阻塞
		}
	}

	void ThreadBlocker::signal() {
		std::unique_lock<std::mutex> lock(_mtx);

		_flag = false;

		_cond.notify_all();
	}

	void ThreadBlocker::reset() {
		std::unique_lock<std::mutex> lock(_mtx);

		_flag = true;
	}
}