// MsgSession.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Message/MsgWrapper.h"
#include "Message/MessageManager.h"
#include "Message/MessageHandler.h"
#include "Message/MsgDisconnect.h"
#include "MsgSession.h"
#include "SecurityManager.h"

#include <mutex>


namespace NiuMa {
	class MsgSessionData
	{
	public:
		MsgSessionData(int heartbeatValid)
			: _handlerSN(0)
			, _heartbeatValid(heartbeatValid)
		{
			_heartbeat = BaseUtils::getCurrentSecond();
		}

		virtual ~MsgSessionData() {}

	public:
		bool next() {
			bool ret = false;
			try {
				ret = _unpacker.next(_object_handle);
			}
			catch (std::exception& ex) {
				// 重置解包器
				_unpacker.reset();
				_unpacker.remove_nonparsed_buffer();
				ErrorS << "Unpack data error: " << ex.what();
			}
			return ret;
		}

		void synchronizeHandlers() {
			int sn = MessageManager::getSingleton().getHandlerSN();
			if (sn != _handlerSN) {
				_handlers.clear();
				MessageManager::getSingleton().getAllHandlers(_handlers);
				_handlerSN = sn;
			}
		}

		void heartbeat() {
			std::lock_guard<std::mutex> lck(_mtx);

			_heartbeat = BaseUtils::getCurrentSecond();
		}

		// 心跳是否超时
		bool isTimeout(const time_t& nowTime) {
			if (_heartbeatValid < 1)
				return false;

			std::lock_guard<std::mutex> lck(_mtx);
			time_t elapsed = nowTime - _heartbeat;
			if (elapsed > _heartbeatValid)
				return true;

			return false;
		}

	public:
		// 解包器
		msgpack::unpacker _unpacker;

		// 接受到的对象处理
		msgpack::object_handle _object_handle;

		// 消息处理器列表
		// 每个消息会话拷贝一份处理器列表，避免接收网络消息时线程锁带来的性能消耗
		std::vector<MessageHandler::Ptr> _handlers;

		// 消息处理器序号
		int _handlerSN;

	private:
		// 心跳有效时间，即超时时间，单位秒，小于等于0表示心跳永不超时
		const int _heartbeatValid;

		// 上一次心跳时间
		time_t _heartbeat;

		// 信号量
		std::mutex _mtx;
	};


	MsgSession::MsgSession(const std::shared_ptr<Connection>& con, int heartbeatValid)
		: Session(con)
	{
		_data = new MsgSessionData(heartbeatValid);
	}

	MsgSession::~MsgSession() {
		if (_data != nullptr) {
			delete _data;
			_data = nullptr;
		}
	}

	void MsgSession::onRecieve(char* buf, std::size_t length) {
		if (buf == nullptr || length == 0)
			return;

		const std::string& remoteIp = getRemoteIp();
		if (SecurityManager::getSingleton().checkBlacklist(remoteIp))
			return;

		_data->_unpacker.reserve_buffer(length);

		memcpy(_data->_unpacker.buffer(), buf, length);
		_data->_unpacker.buffer_consumed(length);

		bool test = true;
		while (_data->next()) {
			try {
				const msgpack::object& obj = _data->_object_handle.get();
				std::stringstream ss;
				ss << obj;
				MsgWrapper::Ptr wrapper = std::make_shared<MsgWrapper>();
				obj.convert(*wrapper);
				MsgBase::Ptr msg = MessageManager::getSingleton().createMessage(wrapper);
				if (msg) {
					NetMessage::Ptr netMsg = std::make_shared<NetMessage>(shared_from_this(), msg, wrapper->getType());
					pushMsg(netMsg);
				}
				else {
					ErrorS << "Deserialize message of type: \"" << wrapper->getType() << "\" failed.";
					if (test) {
						// 记录一次异常
						test = false;
						SecurityManager::getSingleton().abnormalBehavior(remoteIp);
					}
				}
			}
			catch (std::exception& ex) {
				ErrorS << "Unpack message error: " << ex.what();
				if (test) {
					// 记录一次异常
					test = false;
					SecurityManager::getSingleton().abnormalBehavior(remoteIp);
				}
				// 重置解包器
				_data->_unpacker.reset();
				_data->_unpacker.remove_nonparsed_buffer();
				break;
			}
		}
	}

	void MsgSession::pushMsg(const NetMessage::Ptr& msg) {
		// 一个消息最多只会发送到一个消息处理器
		_data->synchronizeHandlers();
		for (const MessageHandler::Ptr& handler : _data->_handlers) {
			if (handler->receive(msg)) {
				handler->push(msg);
				break;
			}
		}
	}

	void MsgSession::onDisconnect() {
		std::string sessionId;
		getId(sessionId);
		std::shared_ptr<MsgDisconnect> msg(new MsgDisconnect());
		msg->setSessionId(sessionId);
		NetMessage::Ptr netMsg = std::make_shared<NetMessage>(nullptr, msg, MsgDisconnect::TYPE);
		pushMsg(netMsg);
	}

	bool MsgSession::isAlive(const time_t& nowTime) const {
		if (!Session::isAlive(nowTime))
			return false;
		if (_data->isTimeout(nowTime))
			return false;
		return true;
	}

	void MsgSession::heartbeat() {
		_data->heartbeat();
	}
}