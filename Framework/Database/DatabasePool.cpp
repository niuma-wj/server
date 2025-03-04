// DatabasePool.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "DatabasePool.h"

namespace NiuMa {
	DatabasePool::DatabasePool(int keepConnections, int maxConnections)
		: _keepConnections(keepConnections)
		, _maxConnections(maxConnections)
		, _stopFlag(false)
	{}

	DatabaseConnection::Ptr DatabasePool::getConnection() {
		ThreadBlocker::Ptr blocker;
		DatabaseConnection::Ptr con;
		while (true) {
			if (_stopFlag)
				return con;
			con = getConnection(blocker);
			if (con) {
				//LOG_DEBUG("Get connection.");
				break;
			}
			if (blocker) {
				// 请求连接失败，阻塞当前线程并等待空闲连接
				//LOG_DEBUG("Block thread.");
				blocker->block();
				//LOG_DEBUG("Thread wake up.");
				blocker = nullptr;
			}
			else
				break;
		}
		return con;
	}

	DatabaseConnection::Ptr DatabasePool::getConnection(ThreadBlocker::Ptr& blocker) {
		DatabaseConnection::Ptr con;
		if (_stopFlag)
			return con;

		std::lock_guard<std::mutex> lck(_mtx);

		for (const DatabaseConnection::Ptr& tmp : _connections) {
			if (tmp->isOccupied())
				continue;
			tmp->occupy();
			con = tmp;
			return con;
		}
		int nums = static_cast<int>(_connections.size());
		if (nums >= _maxConnections) {
			// ErrorS << "Request connection failed, the number of current connections exceeds the upper limit.";
			// 当前连接数量已经超出最大数量，需要阻塞当前线程，等待其他连接释放
			blocker = std::make_shared<ThreadBlocker>();
			_blockQueue.push(blocker);
			return con;
		}
		con = createConnection();
		if (con) {
			con->occupy();
			_connections.push_back(con);
		}
		return con;
	}

	void DatabasePool::removeConnection(const DatabaseConnection::Ptr& con) {
		if (!con)
			return;

		std::lock_guard<std::mutex> lck(_mtx);

		std::list<DatabaseConnection::Ptr>::iterator it = _connections.begin();
		for (; it != _connections.end(); it++) {
			if (*it == con) {
				_connections.erase(it);
				break;
			}
		}
		if (!_blockQueue.empty()) {
			ThreadBlocker::Ptr blocker = _blockQueue.front();
			_blockQueue.pop();
			blocker->signal();
		}
	}

	void DatabasePool::notifyOne() {
		std::lock_guard<std::mutex> lck(_mtx);

		if (!_blockQueue.empty()) {
			ThreadBlocker::Ptr blocker = _blockQueue.front();
			_blockQueue.pop();
			blocker->signal();
		}
	}

	bool DatabasePool::onTimer() {
		if (_stopFlag)
			return true;

		DatabaseConnection::Ptr con = onTimerImpl();
		if (con)
			onTimerImpl(con);
		return false;
	}

	DatabaseConnection::Ptr DatabasePool::onTimerImpl() {
		// 当前时间，秒
		time_t nowTime = BaseUtils::getCurrentSecond();

		std::lock_guard<std::mutex> lck(_mtx);

		bool test = false;
		bool removeFlag = true;
		int nums = static_cast<int>(_connections.size());
		if (nums <= _keepConnections)
			removeFlag = false;
		const time_t MIN5 = 300LL;	// 5分钟
		const time_t SEC30 = 30LL;	// 30秒
		time_t delta = 0LL;
		time_t occupyTime = 0LL;
		time_t queryTime = 0LL;
		std::list<DatabaseConnection::Ptr>::iterator it = _connections.begin();
		while (it != _connections.end()) {
			DatabaseConnection::Ptr& con = *it;
			if (con->isOccupied()) {
				++it;
				continue;
			}
			test = false;
			con->getLastTime(occupyTime, queryTime);
			if (removeFlag) {
				delta = nowTime - occupyTime;
				if (delta > MIN5) {
					// 连接超过5分钟未被占用，删除
					test = true;
				}
			}
			if (test) {
				it = _connections.erase(it);
				nums--;
				if (!_blockQueue.empty()) {
					ThreadBlocker::Ptr blocker = _blockQueue.front();
					_blockQueue.pop();
					blocker->signal();
				}
				if (nums <= _keepConnections)
					removeFlag = false;
			}
			else {
				delta = nowTime - queryTime;
				if (delta > SEC30) {
					// 超过30秒没有查询数据库，发起测试查询，以免连接长时间不活动被数据库服务器关闭
					con->occupy(false);
					return con;
				}
				++it;
			}
		}
		return nullptr;
	}

	void DatabasePool::stop() {
		_stopFlag = true;
	}

	void DatabasePool::clearConnections() {
		std::lock_guard<std::mutex> lck(_mtx);

		_connections.clear();
	}
}