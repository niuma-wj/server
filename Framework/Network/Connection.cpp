// Connection.cpp

#include "Connection.h"
#include "Base/BaseUtils.h"

namespace NiuMa {
	Connection::Connection()
		: _abandonTime(0LL)
	{}

	Connection::~Connection() {}

	void Connection::abandon() {
		_abandonTime = BaseUtils::getCurrentSecond();
	}

	time_t Connection::getAbandonTime() const {
		return _abandonTime;
	}
}