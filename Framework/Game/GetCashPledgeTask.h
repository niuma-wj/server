// GetCashPledgeTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.25

#ifndef _NIU_MA_GET_CASH_PLEDGE_TASK_H_
#define _NIU_MA_GET_CASH_PLEDGE_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	/**
	 * ��ѯ���Ѻ��SQL����
	 */
	class GetCashPledgeTask : public MysqlQueryTask {
	public:
		GetCashPledgeTask(const std::string& playerId, const std::string& venueId);
		virtual ~GetCashPledgeTask();

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;

	public:
		const std::string& getPlayerId() const;
		const std::string& getVenueId() const;
		int64_t getAmount() const;

	private:
		//
		std::string _playerId;

		//
		std::string _venueId;

		// Ѻ������
		int64_t _amount;
	};
}

#endif // !_NIU_MA_GET_CASH_PLEDGE_TASK_H_