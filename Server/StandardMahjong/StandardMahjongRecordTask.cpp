// StandardMahjongRecordTask.cpp

#include "StandardMahjongRecordTask.h"

#include <sstream>

namespace NiuMa
{
	StandardMahjongRecordTask::StandardMahjongRecordTask()
		: _roundNo(0)
		, _banker(0)
	{
		for (int i = 0; i < 4; i++) {
			_scores[i] = 0;
			_winGolds[i] = 0;
		}
	}

	StandardMahjongRecordTask::~StandardMahjongRecordTask() {}

	MysqlQueryTask::QueryType StandardMahjongRecordTask::buildQuery(std::string& sql) {
		std::stringstream ss;
		ss << "insert into `game_mahjong_record` (`venue_id`, `round_no`, `player_id0`, `player_id1`, `player_id2`, `player_id3`, "
			"`banker`, `score0`, `score1`, `score2`, `score3`, `win_gold0`, `win_gold1`, `win_gold2`, `win_gold3`, `playback`, `time`)"
			"values (\"" << _venueId << "\", " << _roundNo;
		for (int i = 0; i < 4; i++)
			ss << ", \"" << _playerIds[i] << "\"";
		ss << ", " << _banker;
		for (int i = 0; i < 4; i++)
			ss << ", " << _scores[i];
		for (int i = 0; i < 4; i++)
			ss << ", " << _winGolds[i];
		ss << ", \"" << _playback << "\", now())";
		sql = ss.str();
		return MysqlQueryTask::QueryType::Insert;
	}
}