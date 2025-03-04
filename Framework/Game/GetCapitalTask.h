// GetCapitalTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.25

#ifndef _NIU_MA_GET_CAPITAL_TASK_H_
#define _NIU_MA_GET_CAPITAL_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	/**
	 * ��ȡ����ʲ�SQL����
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

		// �������
		int64_t _gold;

		// ������
		int64_t _deposit;

		// ��ʯ����
		int64_t _diamond;

		// �汾�ţ������ֹ���
		int64_t _version;
	};
}


#endif // !_NIU_MA_GET_CAPITAL_TASK_H_
