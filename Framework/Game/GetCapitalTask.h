// GetCapitalTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.25

#ifndef _NIU_MA_GET_CAPITAL_TASK_H_
#define _NIU_MA_GET_CAPITAL_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	/**
	 * 获取玩家资产SQL任务
	 */
	class GetCapitalTask : public MysqlQueryTask {
	public:
		GetCapitalTask(const std::string& playerId);
		virtual ~GetCapitalTask();

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;

	public:
		const std::string& getPlayerId() const;
		int64_t getGold() const;
		int64_t getDeposit() const;
		int64_t getDiamond() const;
		int64_t getVersion() const;

	private:
		//
		std::string _playerId;

		// 金币数量
		int64_t _gold;

		// 存款余额
		int64_t _deposit;

		// 钻石数量
		int64_t _diamond;

		// 版本号，用于乐观锁
		int64_t _version;
	};
}


#endif // !_NIU_MA_GET_CAPITAL_TASK_H_
