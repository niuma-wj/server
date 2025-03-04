// LoadPlayerTask.cpp

#include "LoadPlayerTask.h"

#include <mysql/jdbc.h>

namespace NiuMa {
	LoadPlayerTask::LoadPlayerTask(const std::string& playerId)
		: _playerId(playerId)
		, _sex(0)
	{}

	MysqlQueryTask::QueryType LoadPlayerTask::buildQuery(std::string& sql) {
		sql = "select `name`, `nickname`, `phone`, `sex`, `avatar` from `player` where `id` = \"" + _playerId + "\"";
		return QueryType::Select;
	}

	int LoadPlayerTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		while (res->next()) {
			_name = res->getString("name");
			_nickname = res->getString("nickname");
#ifdef _MSC_VER
			// VC环境下utf8编码转gb2312，以方便调试和日志输出
			//_nickname = boost::locale::conv::from_utf(_nickname, std::string("gb2312"));
#endif
			_phone = res->getString("phone");
			_sex = res->getInt("sex");
			_avatar = res->getString("avatar");
			rows++;
		}
		return rows;
	}
}