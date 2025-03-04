// EchoSession.cpp

#include "EchoSession.h"

namespace NiuMa {
	EchoSession::EchoSession(const std::shared_ptr<Connection>& con)
		: Session(con)
	{}

	EchoSession::~EchoSession() {}

	void EchoSession::onRecieve(char* buf, std::size_t length) {
		// 直接将接收到的数据发回客户端
		send(buf, length);
	}
}