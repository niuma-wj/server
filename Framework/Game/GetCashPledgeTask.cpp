// GetCashPledgeTask.cpp

#include "GetCashPledgeTask.h"

#include <mysql/jdbc.h>

namespace NiuMa {
	GetCashPledgeTask::GetCashPledgeTask(const std::string& playerId, const std::string& venueId)
		: _playerId(playerId)
		, _venueId(venueId)
		, _amount(0LL)
	{}

	GetCashPledgeTask::~GetCashPledgeTask() {}

	MysqlQueryTask::QueryType GetCashPledgeTask::buildQuery(std::string& sql) {
		char buf[256] = { '\0' };
#if defined _WIN32
		_snprintf_s(buf, 256, "select `amount` from `cash_pledge` where `player_id` = \"%s\" and `venue_id` =\"%s\"", _playerId.c_str(), _venueId.c_str());
#else
		snprintf(buf, 256, "select `amount` from `cash_pledge` where `player_id` = \"%s\" and `venue_id` =\"%s\"", _playerId.c_str(), _venueId.c_str());
#endif
		sql = buf;
		return MysqlQueryTask::QueryType::Select;
	}

	int GetCashPledgeTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		if (res->next()) {
			_amount = res->getInt64("amount");
			rows++;
		}
		return rows;
	}

	const std::string& GetCashPledgeTask::getPlayerId() const {
		return _playerId;
	}

	const std::string& GetCashPledgeTask::getVenueId() const {
		return _venueId;
	}

	int64_t GetCashPledgeTask::getAmount() const {
		return _amount;
	}
}