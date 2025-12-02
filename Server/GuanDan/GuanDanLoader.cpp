// GuanDanLoader.cpp

#include "Base/Log.h"
#include "GuanDanLoader.h"
#include "GuanDanRoom.h"
#include "../GameDefines.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa
{
	GuanDanLoader::GuanDanLoader()
		: VenueLoader(static_cast<int>(GameType::GuanDan))
	{}

	GuanDanLoader::~GuanDanLoader() {}

	Venue::Ptr GuanDanLoader::load(const std::string& id) {
		class LackeyTask : public MysqlQueryTask
		{
		public:
			LackeyTask(const std::string& id)
				: _venueId(id)
				, _level(0)
			{}

			virtual ~LackeyTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `number`, `level` from `game_guan_dan` where `venue_id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_number = res->getString("number");
					_level = res->getInt("level");
					rows++;
				}
				return rows;
			}

		public:
			const std::string _venueId;

			// 房间编号
			std::string _number;

			//
			int _level;
		};
		std::shared_ptr<LackeyTask> task = std::make_shared<LackeyTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			ErrorS << "加载掼蛋游戏(Id: " << id << ")失败";
			return nullptr;
		}
		std::shared_ptr<GuanDanRoom> room = std::make_shared<GuanDanRoom>(id, task->_number, task->_level);
		return room;
	}
}