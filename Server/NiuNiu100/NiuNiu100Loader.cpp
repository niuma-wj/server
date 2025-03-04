// NiuNiu100Loader.cpp

#include "Base/Log.h"
#include "NiuNiu100Loader.h"
#include "NiuNiu100Room.h"
#include "../GameDefines.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>

namespace NiuMa
{
	NiuNiu100Loader::NiuNiu100Loader()
		: VenueLoader(static_cast<int>(GameType::NiuNiu100))
	{
		_rule = std::make_shared<NiuNiuRule>(true);
	}

	NiuNiu100Loader::~NiuNiu100Loader() {}

	Venue::Ptr NiuNiu100Loader::load(const std::string& id) {
		class Niu100Task : public MysqlQueryTask
		{
		public:
			Niu100Task(const std::string& id)
				: _venueId(id)
				, _deposit(0LL)
				, _isPublic(0)
			{}

			virtual ~Niu100Task() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `number`, `deposit`, `is_public`, `banker_id` from `game_niu_niu_100` where `venue_id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_number = res->getString("number");
					_deposit = res->getInt("deposit");
					_isPublic = res->getInt("is_public");
					_bankerId = res->getString("banker_id");
					rows++;
				}
				return rows;
			}

		public:
			// 场地id
			const std::string _venueId;

			// 房间编号
			std::string _number;
			
			// 奖池押金数量
			int64_t _deposit;

			// 是否为公开房
			int _isPublic;

			// 庄家(房主)玩家id
			std::string _bankerId;
		};
		std::shared_ptr<Niu100Task> task = std::make_shared<Niu100Task>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			ErrorS << "加载百人牛牛游戏(Id: " << id << ")失败";
			return nullptr;
		}
		std::shared_ptr<NiuNiu100Room> room = std::make_shared<NiuNiu100Room>(_rule, id, task->_number, task->_bankerId, task->_deposit, false);
		return room;
	}
}