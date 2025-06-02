// Session.cpp

#include "Connection.h"
#include "Session.h"

namespace NiuMa {
	Session::Session(const std::shared_ptr<Connection>& con)
		: _connection(con)
		, _valid(true)
	{
		if (con)
			con->getRemoteIp(_remoteIp);
	}

	Session::~Session() {}

	void Session::getId(std::string& id) const {
		Connection::Ptr con = _connection.lock();
		if (con)
			con->getId(id);
	}

	const std::string& Session::getRemoteIp() const {
		return _remoteIp;
	}

	bool Session::isValid() {
		std::lock_guard<std::mutex> lck(_mtx);

		return _valid;
	}

	void Session::setInvalid() {
		std::lock_guard<std::mutex> lck(_mtx);

		_valid = false;
	}

	void Session::onDisconnect() {
		setInvalid();
	}

	void Session::send(const char* buf, std::size_t length) {
		Connection::Ptr con = _connection.lock();
		if (con)
			con->send(buf, length);
	}

	void Session::send(const std::shared_ptr<std::string>& data) {
		Connection::Ptr con = _connection.lock();
		if (con)
			con->send(data);
	}

	bool Session::isAlive(const time_t& nowTime) const {
		Connection::Ptr con = _connection.lock();
		if (con)
			return !(con->isClosed());
		return false;
	}

	void Session::heartbeat() {}

	SessionCreator::SessionCreator() {}

	SessionCreator::~SessionCreator() {}
}