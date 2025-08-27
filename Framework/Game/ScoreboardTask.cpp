// ScoreboardTask.cpp

#include "ScoreboardTask.h"

#include <sstream>

#include <mysql/jdbc.h>

namespace NiuMa {
	GetScoreboardTask::GetScoreboardTask(const std::string& playerId, int gameType)
		: _playerId(playerId)
		, _gameType(gameType)
		, _winNum(0)
		, _loseNum(0)
		, _drawNum(0)
	{}

	GetScoreboardTask::~GetScoreboardTask()
	{}

	MysqlQueryTask::QueryType GetScoreboardTask::buildQuery(std::string& sql) {
		std::stringstream ss;
		ss << "select `win_num`, `lose_num`, `draw_num` from `game_scoreboard` where `player_id` = \'";
		ss << _playerId << "\' and `game_type` = " << _gameType;
		sql = ss.str();
		return QueryType::Select;
	}

	int GetScoreboardTask::fetchResult(sql::ResultSet* res) {
		int rows = 0;
		if (res->next()) {
			_winNum = res->getInt("win_num");
			_loseNum = res->getInt("lose_num");
			_drawNum = res->getInt("draw_num");
			rows++;
		}
		return rows;
	}

	void GetScoreboardTask::getScoreboard(int& win, int& lose, int& draw) const {
		win = _winNum;
		lose = _loseNum;
		draw = _drawNum;
	}

	MysqlQueryTask::Ptr incWinNumTask(const std::string& playerId, int gameType) {
		std::stringstream ss;
		ss << "update `game_scoreboard` set `win_num` = `win_num` + 1 where `player_id` = \'";
		ss << playerId << "\' and `game_type` = " << gameType;
		std::string sql = ss.str();
		return std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
	}

	MysqlQueryTask::Ptr incLoseNumTask(const std::string& playerId, int gameType) {
		std::stringstream ss;
		ss << "update `game_scoreboard` set `lose_num` = `lose_num` + 1 where `player_id` = \'";
		ss << playerId << "\' and `game_type` = " << gameType;
		std::string sql = ss.str();
		return std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
	}

	MysqlQueryTask::Ptr incDrawNumTask(const std::string& playerId, int gameType) {
		std::stringstream ss;
		ss << "update `game_scoreboard` set `draw_num` = `draw_num` + 1 where `player_id` = \'";
		ss << playerId << "\' and `game_type` = " << gameType;
		std::string sql = ss.str();
		return std::make_shared<MysqlCommonTask>(sql, MysqlQueryTask::QueryType::Update);
	}
}