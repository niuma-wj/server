// GetCapitalTask.cpp

#include "GetCapitalTask.h"

#include <mysql/jdbc.h>

namespace NiuMa {
	GetCapitalTask::GetCapitalTask(const std::string& playerId)
		: _playerId(playerId)
		, _gold(0LL)
		, _deposit(0LL)
		, _diamond(0LL)
		, _version(0LL)
	{}

	GetCapitalTask::~GetCapitalTask() {}

	MysqlQueryTask::QueryType GetCapitalTask::buildQuery(std::string& sql) {
		sql = "select `gold`, `deposit`, `diamond`, `version` from `capital` where `player_id` = \"";
		sql = sql + _playerId + "\"";
		return QueryType::Select;
	}

	int GetCapitalTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		if (res->next()) {
			_gold = res->getInt64("gold");
			_deposit = res->getInt64("deposit");
			_diamond = res->getInt64("diamond");
			_version = res->getInt64("version");
			rows++;
		}
		return rows;
	}

	const std::string& GetCapitalTask::getPlayerId() const {
		return _playerId;
	}

	int64_t GetCapitalTask::getGold() const {
		return _gold;
	}

	int64_t GetCapitalTask::getDeposit() const {
		return _deposit;
	}

	int64_t GetCapitalTask::getDiamond() const {
		return _diamond;
	}

	int64_t GetCapitalTask::getVersion() const {
		return _version;
	}
}