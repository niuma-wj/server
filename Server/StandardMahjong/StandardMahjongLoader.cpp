// StandardMahjongLoader.cpp

#include "Base/Log.h"
#include "StandardMahjongLoader.h"
#include "StandardMahjongRoom.h"
#include "../GameDefines.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa
{
	StandardMahjongLoader::StandardMahjongLoader()
		: VenueLoader(static_cast<int>(GameType::Mahjong))
	{
		_rule = std::make_shared<MahjongRule>();
	}

	StandardMahjongLoader::~StandardMahjongLoader() {}

	Venue::Ptr StandardMahjongLoader::load(const std::string& id) {
		class StandardMahjongTask : public MysqlQueryTask
		{
		public:
			StandardMahjongTask(const std::string& id)
				: _venueId(id)
				, _mode(0)
				, _diZhu(0)
				, _config(0)
			{}

			virtual ~StandardMahjongTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `number`, `mode`, `di_zhu`, `rule` from `game_mahjong` where `venue_id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_number = res->getString("number");
					_mode = res->getInt("mode");
					_diZhu = res->getInt("di_zhu");
					_config = res->getInt("rule");
					rows++;
				}
				return rows;
			}

		public:
			const std::string _venueId;

			// 房间编号
			std::string _number;
			
			//
			int _mode;

			//
			int _diZhu;

			// 规则配置
			int _config;
		};
		std::shared_ptr<StandardMahjongTask> task = std::make_shared<StandardMahjongTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			ErrorS << "加载麻将游戏(Id: " << id << ")失败";
			return nullptr;
		}
		std::shared_ptr<StandardMahjongRoom> room = std::make_shared<StandardMahjongRoom>(_rule, id, task->_number, task->_mode, task->_diZhu, task->_config);
		return room;
	}
}