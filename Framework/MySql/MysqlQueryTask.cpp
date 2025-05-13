// MysqlQueryTask.cpp

#include "MysqlQueryTask.h"

#include <mysql/jdbc.h>

namespace NiuMa {
	MysqlCountTask::MysqlCountTask(const std::string& sql)
		: MysqlCommonTask(sql, MysqlQueryTask::QueryType::Select)
		, _count(0)
	{}

	MysqlCountTask::~MysqlCountTask() {}

	int MysqlCountTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		if (res->next()) {
			_count = res->getInt(1);
			rows++;
		}
		return rows;
	}

	int MysqlCountTask::getCount() const {
		return _count;
	}
}