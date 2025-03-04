// MessageHandler.cpp

#include "MessageHandler.h"
#include "MessageManager.h"
#include "Base/Log.h"

namespace NiuMa {
	MessageQueue::MessageQueue() {}

	MessageQueue::~MessageQueue() {}

	void MessageQueue::push(const NetMessage::Ptr& msg) {
		std::lock_guard<std::mutex> lck(_mtx);

		_msgQueue.push(msg);
	}

	NetMessage::Ptr MessageQueue::pop() {
		NetMessage::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		if (!_msgQueue.empty()) {
			ret = _msgQueue.front();
			_msgQueue.pop();
		}
		return ret;
	}

	MessageHandler::MessageHandler(const MessageQueue::Ptr& queue) {
		if (queue)
			_msgQueue = queue;
		else
			_msgQueue = std::make_shared<MessageQueue>();	// 创建的独占的消息队列
	}

	MessageHandler::~MessageHandler() {}

	void MessageHandler::registSelf() {
		MessageManager::getSingleton().registHandler(shared_from_this());
	}

	void MessageHandler::addMessage(const std::string& msgType) {
		if (msgType.empty())
			return;
		_recieveMessages.insert(msgType);
	}

	bool MessageHandler::receive(const NetMessage::Ptr& netMsg) const {
		if (!netMsg)
			return false;
		// 这里不需要做线程同步，因为addMessage仅在实例初始化时调用，初始化完成之后
		// _recieveMessages不会再被修改(只读不写)，从而可以省去线程锁提高消息处理效率
		std::unordered_set<std::string>::const_iterator it = _recieveMessages.find(netMsg->getType());
		return (it != _recieveMessages.end());
	}

	void MessageHandler::initialize() {}

	void MessageHandler::push(const NetMessage::Ptr& msg) {
		_msgQueue->push(msg);
	}

	NetMessage::Ptr MessageHandler::pop() {
		return _msgQueue->pop();
	}

	void MessageHandler::setWorker(const ThreadWorker::Ptr& worker) {
		_worker = worker;
	}

	ThreadWorker::Ptr MessageHandler::getWorker() const {
		return _worker.lock();
	}

	bool MessageHandler::handle() {
		bool ret = false;
		while (true) {
			NetMessage::Ptr msg = pop();
			if (!msg)
				break;
			ret = true;
			if (!preproccess(msg))
				continue;
			if (!onMessage(msg)) {
				WarningS << "Network message of type \"" << msg->getType() << "\" was not processed.";
			}
		}
		return ret;
	}

	bool MessageHandler::preproccess(const NetMessage::Ptr& netMsg) {
		return true;
	}

	bool MessageHandler::onMessage(const NetMessage::Ptr& netMsg) {
		return false;
	}
}