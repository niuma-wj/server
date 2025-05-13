// MysqlPool.cpp

#include "Base/Log.h"
#include "Timer/TimerManager.h"
#include "Thread/ThreadBlocker.h"
#include "Database/DatabasePool.h"
#include "MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa {
	class MysqlConnection : public DatabaseConnection {
	public:
		MysqlConnection(sql::Connection* connection)
			: _connection(connection)
		{}

		virtual ~MysqlConnection() {
			if (_connection != nullptr) {
				delete _connection;
				_connection = nullptr;
			}
		}

	public:
		// 获取mysql连接
		sql::Connection* getConnection() const {
			return _connection;
		}

	private:
		// mysql连接
		sql::Connection* _connection;
	};

	class MysqlPoolImpl : public DatabasePool {
	public:
		MysqlPoolImpl(const std::string& hostName,
			const std::string& userName,
			const std::string& password,
			const std::string& schemaName,
			int keepConnections,
			int maxConnections)
			: DatabasePool(keepConnections, maxConnections)
			, _hostName(hostName)
			, _userName(userName)
			, _password(password)
			, _schemaName(schemaName)
			, _driver(nullptr)
		{}

		virtual ~MysqlPoolImpl() {}

		typedef std::shared_ptr<MysqlPoolImpl> Ptr;

	public:
		// 执行mysql查询任务
		void execute(const MysqlQueryTask::Ptr& task) {
			if (!task)
				return;
			DatabaseConnection::Ptr con = getConnection();
			if (con)
				execute(task, con);
		}

		void pushAsyncTask(const ThreadDispatch::Ptr& task) {
			std::lock_guard<std::mutex> lck(_mtxTask);

			_asyncTasks.push(task);
		}

		ThreadDispatch::Ptr popAsyncTask() {
			ThreadDispatch::Ptr task;

			std::lock_guard<std::mutex> lck(_mtxTask);

			if (!_asyncTasks.empty()) {
				task = _asyncTasks.front();
				_asyncTasks.pop();
			}
			return task;
		}

		void clear() {
			clearAsyncTasks();
			clearConnections();
		}

	protected:
		// 创建mysql连接
		virtual DatabaseConnection::Ptr createConnection() override {
			DatabaseConnection::Ptr con;
			sql::Connection* sqlCon = nullptr;
			try {
				if (_driver == nullptr)
					_driver = sql::mysql::get_mysql_driver_instance();
				if (_driver != nullptr)
					sqlCon = _driver->connect(_hostName, _userName, _password);
				if (sqlCon != nullptr) {
					sqlCon->setSchema(_schemaName);
					con = std::make_shared<MysqlConnection>(sqlCon);
				}
			}
			catch (sql::SQLException& e) {
				ErrorS << "SQLException: " << e.what()
					<< ", error code: " << e.getErrorCode()
					<< ", SQLState: " << e.getSQLState();
			}
			catch (std::exception& ex) {
				ErrorS << "Mysql error: " << ex.what();
			}
			if (!con && (sqlCon != nullptr))
				delete sqlCon;
			return con;
		}

		virtual void onTimerImpl(const DatabaseConnection::Ptr& con) override {
			// 发起测试查询
			std::string sql = "select 1";
			MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Select);
			execute(task, con);
		}

	private:
		// 执行mysql查询任务
		void execute(const MysqlQueryTask::Ptr& task, const DatabaseConnection::Ptr& conIn) {
			std::shared_ptr<MysqlConnection> con = std::dynamic_pointer_cast<MysqlConnection>(conIn);
			if (!con)
				return;
			bool disconnected = false;
			sql::Statement* stmt = nullptr;
			sql::ResultSet* res = nullptr;
			try {
				con->setQueryTime();

				std::string sql;
				MysqlQueryTask::QueryType type = task->buildQuery(sql);
				sql::Connection* sqlCon = con->getConnection();
				stmt = sqlCon->createStatement();
				if (type == MysqlQueryTask::QueryType::Select) {
					res = stmt->executeQuery(sql);
					int rows = task->fetchResult(res);
					task->setRows(rows);
				} else if (
					(type == MysqlQueryTask::QueryType::Insert) ||
					(type == MysqlQueryTask::QueryType::Update) ||
					(type == MysqlQueryTask::QueryType::Delete)) {
					int rows = stmt->executeUpdate(sql);
					task->setAffectedRecords(rows);
				} else if (type == MysqlQueryTask::QueryType::InsertAI) {
					int rows = stmt->executeUpdate(sql);
					task->setAffectedRecords(rows);
					res = stmt->executeQuery("SELECT LAST_INSERT_ID()");
					if (res->next()) {
						task->setAutoInc(res->getInt64(1));
					}
				} else {
					throw std::runtime_error("Unknown query type.");
				}
				task->setSucceed();
			}
			catch (sql::SQLException& e) {
				int errNo = e.getErrorCode();
				std::string sqlState = e.getSQLState();
				if ((errNo == 2006) || (errNo == 2013) || (sqlState == "08S01"))
					disconnected = true;	// 与数据库服务器连接断开
				ErrorS << "SQLException: " << e.what()
					<< ", error code: " << errNo
					<< ", SQLState: " << sqlState;
			}
			catch (std::exception& ex) {
				ErrorS << "Execute mysql task error: " << ex.what();
			}
			if (res != nullptr)
				delete res;
			if (stmt != nullptr)
				delete stmt;
			if (disconnected)
				removeConnection(con);
			else {
				con->recycle();
				notifyOne();
			}
		}

		void clearAsyncTasks() {
			std::lock_guard<std::mutex> lck(_mtxTask);

			while (!_asyncTasks.empty()) {
				_asyncTasks.pop();
			}
		}

	private:
		// 数据库服务器地址(如：tcp://127.0.0.1:3306)
		const std::string _hostName;

		// 用户名
		const std::string _userName;

		// 密码
		const std::string _password;

		// 数据库名
		const std::string _schemaName;

		//
		sql::Driver* _driver;

		// 异步查询任务队列
		std::queue<ThreadDispatch::Ptr> _asyncTasks;

		// 任务队列信号量
		std::mutex _mtxTask;
	};

	class MysqlDispatch : public ThreadDispatch {
	public:
		MysqlDispatch(const ThreadWorker::Ptr& dispatcher, const MysqlQueryTask::Ptr& task, const MysqlPoolImpl::Ptr& impl)
			: ThreadDispatch(dispatcher)
			, _task(task)
			, _impl(impl)
		{}

		virtual ~MysqlDispatch() = default;

	public:
		virtual void onExecuted() override {
			// 在派遣者线程中调用onQueried方法
			_task->onQueried();
		}

	protected:
		virtual void executeImpl() override {
			MysqlConnection::Ptr con;
			MysqlPoolImpl::Ptr impl = _impl.lock();
			if (impl)
				impl->execute(_task);
			ThreadWorker::Ptr disp = _dispatcher.lock();
			if (!disp) {
				// 该任务没有设定派遣者，直接在执行者线程(即当前线程)中调用onQueried方法
				_task->onQueried();
			}
		}

	private:
		// 任务
		MysqlQueryTask::Ptr _task;

		//
		std::weak_ptr<MysqlPoolImpl> _impl;
	};
	
	class MysqlThreadWorker : public ThreadWorker {
	public:
		MysqlThreadWorker(const ThreadStopFlag::Ptr& flag, const MysqlPoolImpl::Ptr& impl)
			: ThreadWorker(flag)
			, _impl(impl)
		{}

		virtual ~MysqlThreadWorker() = default;

	protected:
		virtual int oneLoopEx() override {
			ThreadDispatch::Ptr task;
			while (true) {
				task = _impl->popAsyncTask();
				if (task)
					dispatch(task);
				else
					break;
			}
			// 休眠10毫秒
			return 10;
		}

	private:
		MysqlPoolImpl::Ptr _impl;
	};

	template<> MysqlPool* Singleton<MysqlPool>::_inst = nullptr;

	void MysqlPool::start(const std::string& hostName,
		const std::string& userName,
		const std::string& password,
		const std::string& schemaName,
		int keepConnections,
		int maxConnections,
		int threadNum) {
		if (_impl)
			return;
		if (threadNum < 1)
			threadNum = 1;
		else if (threadNum > 8)
			threadNum = 8;
		if (isStarted())
			return;
		if (keepConnections < 1)
			keepConnections = 1;
		if (maxConnections < 1)
			maxConnections = 1;
		_impl = std::make_shared<MysqlPoolImpl>(hostName, userName, password, schemaName, keepConnections, maxConnections);

		ThreadStopFlag::Ptr flag = std::make_shared<ThreadStopFlag>();
		std::vector<ThreadWorker::Ptr> workers;
		for (int i = 0; i < threadNum; i++) {
			ThreadWorker::Ptr worker = std::make_shared<MysqlThreadWorker>(flag, _impl);
			workers.push_back(worker);
		}
		ThreadPool::start(flag, workers);

		std::weak_ptr<MysqlPoolImpl> weakImpl = _impl;
		TimerManager::getSingleton().addAsyncTimer(2000, [weakImpl]() {
			MysqlPoolImpl::Ptr impl = weakImpl.lock();
			if (impl)
				return impl->onTimer();
			return true;
		});
		InfoS << "Mysql pool started, host: " << hostName << ", username: " << userName << ", schema: " << schemaName
			<< ", keep connections: " << keepConnections << ", max connections: " << maxConnections << ", threadNum: " << threadNum;
	}

	void MysqlPool::stop() {
		ThreadPool::stop();

		if (_impl) {
			_impl->stop();
			_impl->clear();
			_impl.reset();
		}
	}

	void MysqlPool::syncQuery(const MysqlQueryTask::Ptr& task) {
		_impl->execute(task);
		task->onQueried();
	}

	void MysqlPool::syncQuery(const std::string& sql, MysqlQueryTask::QueryType type) {
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, type);
		syncQuery(task);
	}

	void MysqlPool::asyncQuery(const MysqlQueryTask::Ptr& task, const ThreadWorker::Ptr& dispatcher) {
		ThreadDispatch::Ptr disp = std::make_shared<MysqlDispatch>(dispatcher, task, _impl);
		_impl->pushAsyncTask(disp);
	}

	void MysqlPool::asyncQuery(const std::string& sql, MysqlQueryTask::QueryType type, const ThreadWorker::Ptr& dispatcher) {
		MysqlQueryTask::Ptr task = std::make_shared<MysqlCommonTask>(sql, type);
		asyncQuery(task, dispatcher);
	}
}