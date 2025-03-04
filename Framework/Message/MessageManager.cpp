// MessageManager.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "MessageManager.h"

namespace NiuMa {
	template<> MessageManager* Singleton<MessageManager>::_inst = nullptr;

	MessageManager::MessageManager()
		: _handlerSN(0)
	{}

	void MessageManager::registCreator(const std::string& type, const IMsgCreator::Ptr& creator) {
		if (type.empty() || !creator)
			return;

		std::unordered_map<std::string, IMsgCreator::Ptr>::const_iterator it = _creatorMap.find(type);
		if (it == _creatorMap.end())
			_creatorMap.insert(std::make_pair(type, creator));
		else
			ErrorS << "Regist message creator error, message creator with name \"" << type << "\" already existed.";
	}

	MsgBase::Ptr MessageManager::createMessage(const MsgWrapper::Ptr& wrapper) {
		if (!wrapper)
			return MsgBase::Ptr();
		std::unordered_map<std::string, IMsgCreator::Ptr>::const_iterator it = _creatorMap.find(wrapper->getType());
		if (it == _creatorMap.end())
			return MsgBase::Ptr();
		const IMsgCreator::Ptr& creator = it->second;
		return creator->deserialize(wrapper);
	}

	void MessageManager::registHandler(const MessageHandler::Ptr& handler) {
		if (!handler)
			return;

		std::lock_guard<std::mutex> lck(_mtxHandler);

		if (BaseUtils::contain(_handlers, handler))
			return;
	
		_handlers.emplace_back(handler);

		_handlerSN++;
	}

	void MessageManager::unregistHandler(const MessageHandler::Ptr& handler) {
		std::lock_guard<std::mutex> lck(_mtxHandler);

		bool flag = false;
		std::vector<MessageHandler::Ptr>::const_iterator it = _handlers.begin();
		while (it != _handlers.end()) {
			if (*it == handler) {
				_handlers.erase(it);
				flag = true;
				break;
			}
			++it;
		}
		if (flag)
			_handlerSN++;
	}

	int MessageManager::getHandlerSN() {
		std::lock_guard<std::mutex> lck(_mtxHandler);

		return _handlerSN;
	}

	void MessageManager::getAllHandlers(std::vector<MessageHandler::Ptr>& vec) {
		vec.clear();

		std::lock_guard<std::mutex> lck(_mtxHandler);

		for (const MessageHandler::Ptr& handler : _handlers)
			vec.emplace_back(handler);
	}
}