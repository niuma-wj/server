// NetMessage.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.18

#ifndef _NIU_MA_NET_MESSAGE_H_
#define _NIU_MA_NET_MESSAGE_H_

#include "MsgBase.h"

namespace NiuMa {
	// 网络消息
	class NetMessage
	{
	public:
		NetMessage(const Session::Ptr& session, const MsgBase::Ptr& msg, const std::string& type);
		virtual ~NetMessage();

		typedef std::shared_ptr<NetMessage> Ptr;

	public:
		Session::Ptr getSession() const;
		const MsgBase::Ptr& getMessage() const;
		const std::string& getType() const;

	private:
		// 连接会话
		std::weak_ptr<Session> _session;

		// 消息体
		MsgBase::Ptr _msg;

		// 消息类型
		const std::string _type;
	};
}

#endif // !_NIU_MA_NET_MESSAGE_H_
