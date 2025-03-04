// RabbitmqMessageHandler.cpp

#include "RabbitmqMessageHandler.h"

namespace NiuMa {
	RabbitmqMessageHandler::RabbitmqMessageHandler(const std::string& tag)
		: _tag(tag)
	{}

	RabbitmqMessageHandler::~RabbitmqMessageHandler() {}

	const std::string& RabbitmqMessageHandler::getTag() const {
		return _tag;
	}

	void RabbitmqMessageHandler::handle() {
		std::shared_ptr<std::string> ptr;
		while (true) {
			ptr = pop();
			if (!ptr)
				break;
			handleImpl(*ptr);
		}
	}

	void RabbitmqMessageHandler::push(const std::shared_ptr<std::string>& message) {
		if (!message || !receive(*message))
			return;

		std::lock_guard<std::mutex> lck(_mtx);

		_msgQueue.emplace_back(message);
	}

	std::shared_ptr<std::string> RabbitmqMessageHandler::pop() {
		std::shared_ptr<std::string> ret;

		std::lock_guard<std::mutex> lck(_mtx);

		if (_msgQueue.empty())
			return ret;
		ret = _msgQueue.front();
		_msgQueue.pop_front();
		return ret;
	}

	bool RabbitmqMessageHandler::receive(const std::string& message) {
		return false;
	}
}