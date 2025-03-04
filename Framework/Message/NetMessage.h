// NetMessage.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.18

#ifndef _NIU_MA_NET_MESSAGE_H_
#define _NIU_MA_NET_MESSAGE_H_

#include "MsgBase.h"

namespace NiuMa {
	// ������Ϣ
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
		// ���ӻỰ
		std::weak_ptr<Session> _session;

		// ��Ϣ��
		MsgBase::Ptr _msg;

		// ��Ϣ����
		const std::string _type;
	};
}

#endif // !_NIU_MA_NET_MESSAGE_H_
