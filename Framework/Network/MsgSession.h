// MsgSession.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.18

#ifndef _NIU_MA_MSG_SESSION_H_
#define _NIU_MA_MSG_SESSION_H_

#include "Session.h"
#include "Message/NetMessage.h"

namespace NiuMa {
	class MsgSessionData;
	class MsgSession : public Session
	{
	public:
		MsgSession(const std::shared_ptr<Connection>& con, int heartbeatValid);
		virtual ~MsgSession();

		class Creator : public SessionCreator {
		public:
			Creator(int heartbeatValid = -1)
				: _heartbeatValid(heartbeatValid)
			{}

		public:
			virtual Session::Ptr create(const std::shared_ptr<Connection>& con) const override {
				Session::Ptr sess = std::make_shared<MsgSession>(con, _heartbeatValid);
				return sess;
			}

		private:
			// 心跳有效时间，即超时时间，单位秒，小于等于0表示心跳永不超时
			int _heartbeatValid;
		};

	public:
		virtual void onRecieve(char* buf, std::size_t length) override;
		virtual void onDisconnect() override;
		virtual bool isAlive(const time_t& nowTime) const override;
		virtual void heartbeat() override;

	private:
		void pushMsg(const NetMessage::Ptr& msg);

	private:
		// 数据成员
		MsgSessionData* _data;
	};
}

#endif // !_NIU_MA_MSG_SESSION_H_
