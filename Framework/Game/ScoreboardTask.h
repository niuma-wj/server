// ScoreboardTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2025.06.06

#ifndef _NIU_MA_SCOREBOARD_TASK_H_
#define _NIU_MA_SCOREBOARD_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	/**
	 * 查询游戏记分牌任务
	 */
	class GetScoreboardTask : public MysqlQueryTask
	{
	public:
		GetScoreboardTask(const std::string& playerId, int gameType);
		virtual ~GetScoreboardTask();

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;

	public:
		// 获取输赢及平局次数
		void getScoreboard(int& win, int& lose, int& draw) const;

	private:
		// 玩家id
		const std::string _playerId;

		// 游戏类型
		const int _gameType;

		// 赢局总数
		int _winNum;

		// 输局总数
		int _loseNum;

		// 平局总数
		int _drawNum;
	};

	/**
	 * 创建增加赢局次数任务
	 * @param playerId 玩家id
	 * @param gameType 游戏类型
	 * @return sql任务
	 */
	MysqlQueryTask::Ptr incWinNumTask(const std::string& playerId, int gameType);

	/**
	 * 创建增加输局次数任务
	 * @param playerId 玩家id
	 * @param gameType 游戏类型
	 * @return sql任务
	 */
	MysqlQueryTask::Ptr incLoseNumTask(const std::string& playerId, int gameType);

	/**
	 * 创建增加平局次数任务
	 * @param playerId 玩家id
	 * @param gameType 游戏类型
	 * @return sql任务
	 */
	MysqlQueryTask::Ptr incDrawNumTask(const std::string& playerId, int gameType);
}

#endif // !_NIU_MA_SCOREBOARD_TASK_H_