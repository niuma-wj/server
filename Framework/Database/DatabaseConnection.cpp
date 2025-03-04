// DatabaseConnection.cpp

#include "Base/BaseUtils.h"
#include "DatabaseConnection.h"

namespace NiuMa {
	DatabaseConnection::DatabaseConnection()
		: _occupied(false)
		, _lastOccupiedTime(0)
		, _lastQueryTime(0)
	{}

	void DatabaseConnection::occupy(bool flag) {
		std::lock_guard<std::mutex> lck(_mtx);

		_occupied = true;
		if (flag)
			_lastOccupiedTime = BaseUtils::getCurrentSecond();
	}

	void DatabaseConnection::recycle() {
		std::lock_guard<std::mutex> lck(_mtx);

		_occupied = false;
	}

	bool DatabaseConnection::isOccupied() {
		std::lock_guard<std::mutex> lck(_mtx);

		return _occupied;
	}

	void DatabaseConnection::setQueryTime() {
		std::lock_guard<std::mutex> lck(_mtx);

		_lastQueryTime = BaseUtils::getCurrentSecond();
	}

	void DatabaseConnection::getLastTime(time_t& occupyTime, time_t& queryTime) {
		std::lock_guard<std::mutex> lck(_mtx);

		occupyTime = _lastOccupiedTime;
		queryTime = _lastQueryTime;
	}
}