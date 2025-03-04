// GetAgencyTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.06

#ifndef _NIU_MA_GET_AGENCY_TASK_H_
#define _NIU_MA_GET_AGENCY_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	/**
	 * ��ѯ��������������ID����
	 */
	class GetAgencyTask : public MysqlQueryTask {
	public:
		GetAgencyTask(const std::string& playerId);
		virtual ~GetAgencyTask();

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;

	public:
		const std::string& getPlayerId() const;
		const std::string& getAgencyId() const;

	private:
		// ���id
		const std::string _playerId;

		// �������id
		std::string _agencyId;
	};
}

#endif // !_NIU_MA_GET_AGENCY_TASK_H_