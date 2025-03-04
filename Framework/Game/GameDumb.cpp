// GameDumb.cpp

#include "GameDumb.h"
#include "MySql/MysqlPool.h"

#include <mysql/jdbc.h>
#include <boost/locale.hpp>

namespace NiuMa {
	GameDumb::GameDumb(const std::string& id, const std::string& name, int maxPlayers)
		: Venue(id, 1)
		, _name(name)
		, _maxPlayers(maxPlayers)
	{}

	GameDumb::~GameDumb() {}

	void GameDumb::onDisconnect(const std::string& playerId) {
		Venue::onDisconnect(playerId);
		// 玩家离线，直接离开场地
		//std::string errMsg;
		//onLeave(playerId, errMsg);
	}

	bool GameDumb::hasPlayer(const std::string& playerId) {
		std::unordered_set<std::string>::const_iterator it = _playerIds.find(playerId);
		return (it != _playerIds.end());
	}

	void GameDumb::getPlayerIds(std::vector<std::string>& playerIds) {
		playerIds.clear();
		for (const std::string& playerId : _playerIds)
			playerIds.push_back(playerId);
	}

	int GameDumb::getPlayerCount() {
		return static_cast<int>(_playerIds.size());
	}

	bool GameDumb::enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) {
		int count = getPlayerCount();
		if (count < _maxPlayers) {
			_playerIds.insert(playerId);
			return true;
		}
		errMsg = "The number of players has reached upper limit";
		return false;
	}

	int GameDumb::leaveImpl(const std::string& playerId, std::string& errMsg) {
		std::unordered_set<std::string>::const_iterator it = _playerIds.find(playerId);
		if (it != _playerIds.end())
			_playerIds.erase(it);
		return 0;
	}

	GameDumbLoader::GameDumbLoader()
		: VenueLoader(1)
	{}

	GameDumbLoader::~GameDumbLoader() {}

	Venue::Ptr GameDumbLoader::load(const std::string& id) {
		class GameDumbTask : public MysqlQueryTask {
		public:
			GameDumbTask(const std::string& id)
				: _id(id)
				, _maxPlayers(0)
			{}

			virtual ~GameDumbTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `name`, `max_players` from `dumb` where `venue_id` = \"" + _id + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) override {
				int rows = 0;
				while (res->next()) {
					_name = res->getString("name");
#ifdef _MSC_VER
					// VC环境下utf8编码转gb2312，以方便调试和日志输出
					_name = boost::locale::conv::from_utf(_name, std::string("gb2312"));
#endif
					_maxPlayers = res->getInt("max_players");
					rows++;
				}
				return rows;
			}

		public:
			const std::string _id;
			std::string _name;
			int _maxPlayers;
		};
		MysqlQueryTask::Ptr task = std::make_shared<GameDumbTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed())
			return nullptr;
		GameDumbTask* gdt = dynamic_cast<GameDumbTask*>(task.get());
		Venue::Ptr venue = std::make_shared<GameDumb>(id, gdt->_name, gdt->_maxPlayers);
		return venue;
	}
}