// ThreadDispatch.cpp

#include "ThreadWorker.h"
#include "Base/Log.h"

namespace NiuMa {
	ThreadDispatch::ThreadDispatch(const std::shared_ptr<ThreadWorker>& dispatcher)
		: _dispatcher(dispatcher)
	{}

	void ThreadDispatch::execute() {
		try {
			executeImpl();
		}
		catch (std::exception& ex) {
			ErrorS << "Exectue dispatch error: " << ex.what();
		}
		ThreadWorker::Ptr disp = _dispatcher.lock();
		if (disp)
			disp->executed(shared_from_this());
	}

	void ThreadDispatch::onExecuted() {}
}