// GetAgencyTask.cpp

#include "GetAgencyTask.h"
#include <mysql/jdbc.h>

namespace NiuMa {

	GetAgencyTask::GetAgencyTask(const std::string& playerId)
		: _playerId(playerId)
	{}

	GetAgencyTask::~GetAgencyTask() {}

	MysqlQueryTask::QueryType GetAgencyTask::buildQuery(std::string& sql) {
		sql = "select `agency_id` from `player` where `id` = \"";
		sql = sql + _playerId + "\"";
		return QueryType::Select;
	}

	int GetAgencyTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		if (res->next()) {
			_agencyId = res->getString("agency_id");
			rows++;
		}
		return rows;
	}

	const std::string& GetAgencyTask::getPlayerId() const {
		return _playerId;
	}

	const std::string& GetAgencyTask::getAgencyId() const {
		return _agencyId;
	}
}