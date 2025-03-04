// LackeyLoader.cpp

#include "Base/Log.h"
#include "LackeyLoader.h"
#include "LackeyRoom.h"
#include "../GameDefines.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa
{
	LackeyLoader::LackeyLoader()
		: VenueLoader(static_cast<int>(GameType::Lackey))
	{
		_rule = std::make_shared<LackeyRule>();
	}

	LackeyLoader::~LackeyLoader() {}

	Venue::Ptr LackeyLoader::load(const std::string& id) {
		class LackeyTask : public MysqlQueryTask
		{
		public:
			LackeyTask(const std::string& id)
				: _venueId(id)
				, _level(0)
				, _mode(0)
				, _diZhu(0)
			{}

			virtual ~LackeyTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `number`, `level`, `mode`, `di_zhu` from `game_lackey` where `venue_id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_number = res->getString("number");
					_level = res->getInt("level");
					_mode = res->getInt("mode");
					_diZhu = res->getInt("di_zhu");
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

			//
			int _mode;

			//
			int _diZhu;
		};
		std::shared_ptr<LackeyTask> task = std::make_shared<LackeyTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			ErrorS << "加载狗腿游戏(Id: " << id << ")失败";
			return nullptr;
		}
		std::shared_ptr<LackeyRoom> room = std::make_shared<LackeyRoom>(
			_rule, id, task->_number, task->_level, task->_mode, task->_diZhu);
		return room;
	}
}