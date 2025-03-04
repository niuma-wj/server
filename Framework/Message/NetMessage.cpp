// NetMessage.cpp

#include "NetMessage.h"

namespace NiuMa {
	NetMessage::NetMessage(const Session::Ptr& session, const MsgBase::Ptr& msg, const std::string& type)
		: _session(session)
		, _msg(msg)
		, _type(type)
	{}

	NetMessage::~NetMessage() {}

	Session::Ptr NetMessage::getSession() const {
		Session::Ptr session = _session.lock();
		return session;
	}

	const MsgBase::Ptr& NetMessage::getMessage() const {
		return _msg;
	}

	const std::string& NetMessage::getType() const {
		return _type;
	}
}