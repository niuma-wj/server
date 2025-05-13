// Example.cpp

#include "Example.h"
#include "Base/Log.h"
#include "MySql/MysqlPool.h"
#include "Rabbitmq/RabbitmqClient.h"

#include <mysql/jdbc.h>
#include <sstream>
#include <chrono>

namespace NiuMa {
	MysqlQueryTask::QueryType TestSelect::buildQuery(std::string& sql) {
		sql = "select * from `test`";
		return QueryType::Select;
	}

	int TestSelect::fetchResult(sql::ResultSet* res) {
		Record rec;
		int rows = 0;
		while (res->next()) {
			rec._id = res->getInt("id");
			rec._field1 = res->getString("field1");
			rec._field2 = res->getString("field2");
			_records.push_back(rec);
			rows++;
		}
		return rows;
	}

	void TestSelect::onQueried() {
		if (!getSucceed()) {
			ErrorS << "Sql query execute failed.";
			return;
		}
		LOG_INFO("Sql result:");
		LOG_INFO("id\t\tfield1\t\tfield2");
		std::stringstream ss;
		for (const Record& rec : _records) {
			ss << rec._id << "\t\t" << rec._field1 << "\t\t" << rec._field2;
			LOG_INFO(ss.str());
			ss.str("");
		}
	}

	MysqlQueryTask::QueryType TestAutoIncrement::buildQuery(std::string& sql) {
		sql = "insert into `test`(`field1`, `field2`) values(\'sdas\', \'basdabb\')";
		return QueryType::InsertAI;
	}

	void TestAutoIncrement::onQueried() {
		if (!getSucceed()) {
			ErrorS << "Sql query execute failed.";
			return;
		}
		InfoS << "The auto increment key is :" << getAutoInc();
	}

	MysqlQueryTask::QueryType TestUpdate::buildQuery(std::string& sql) {
		sql = "update `test` set `field1` = \'wujian1\', `field2` = \'powerful\' where `id` = 10";
		return QueryType::Update;
	}

	void TestUpdate::onQueried() {
		if (!getSucceed())
			ErrorS << "Sql query execute failed.";
		else
			InfoS << "Update succeed, " << getAffectedRecords() << " affected records.";
	}

	TestSelectWorker::TestSelectWorker(const ThreadStopFlag::Ptr& flag)
		: ThreadWorker(flag)
		, _first(true)
	{}

	int TestSelectWorker::oneLoopEx() {
		if (_first) {
			_first = false;
			for (int i = 0; i < 10; i++) {
				MysqlQueryTask::Ptr task = std::make_shared<NiuMa::TestSelect>();
				MysqlPool::getSingleton().asyncQuery(task, shared_from_this());
			}
		}
		return 10;
	}

	TestPrintHandler::TestPrintHandler(const std::string& tag)
		: RabbitmqMessageHandler(tag)
	{}

	TestPrintHandler::~TestPrintHandler() {}

	void TestPrintHandler::handleImpl(const std::string& message) {
		InfoS << "Consume message: " << message << ", tag: " << getTag();
	}

	void TestPublisher::publish() {
		char message[128] = { '\0' };
		_count++;
		snprintf(message, sizeof(message), "Test message %d", _count);
		RabbitmqClient::getSingleton().publish("amq.direct", "test_bind", message);
	}
}