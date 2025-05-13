// ThreadWorkerDumb.cpp

#include "ThreadWorkerDumb.h"

#include <thread>
#include <chrono>

namespace NiuMa {
	ThreadWorkerDumb::ThreadWorkerDumb(const ThreadStopFlag::Ptr& flag, int millisecond)
		: ThreadWorker(flag)
		, _millisecond(millisecond)
	{}

	int ThreadWorkerDumb::oneLoopEx() {
		return _millisecond;
	}
}