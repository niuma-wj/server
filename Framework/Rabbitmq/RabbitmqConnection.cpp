// RabbitmqConnection.cpp

#include "RabbitmqConnection.h"

namespace NiuMa {
	RabbitmqConnection::RabbitmqConnection()
		: _ok(false)
	{}

	RabbitmqConnection::~RabbitmqConnection()
	{}

	void RabbitmqConnection::setOk(bool setting) {
		std::lock_guard<std::mutex> lck(_mtx);
		_ok = setting;
	}

	bool RabbitmqConnection::isOk() const {
		std::lock_guard<std::mutex> lck(_mtx);
		return _ok;
	}
}