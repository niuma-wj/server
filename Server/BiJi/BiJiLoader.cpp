// BiJiLoader.cpp

#include "Base/Log.h"
#include "BiJiLoader.h"
#include "BiJiRoom.h"
#include "../GameDefines.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa
{
	BiJiLoader::BiJiLoader()
		: VenueLoader(static_cast<int>(GameType::LiuAnBiJi))
	{
		_rule = std::make_shared<BiJiRule>();
	}

	BiJiLoader::~BiJiLoader() {}

	Venue::Ptr BiJiLoader::load(const std::string& id) {
		class BiJiTask : public MysqlQueryTask
		{
		public:
			BiJiTask(const std::string& id)
				: _venueId(id)
				, _mode(0)
				, _diZhu(0)
			{}

			virtual ~BiJiTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `number`, `mode`, `di_zhu` from `game_bi_ji` where `venue_id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_number = res->getString("number");
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
			int _mode;

			//
			int _diZhu;
		};
		std::shared_ptr<BiJiTask> task = std::make_shared<BiJiTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			ErrorS << "加载比鸡游戏(Id: " << id << ")失败";
			return nullptr;
		}
		std::shared_ptr<BiJiRoom> room = std::make_shared<BiJiRoom>(id, task->_number, task->_mode, task->_diZhu, _rule);
		return room;
	}
}