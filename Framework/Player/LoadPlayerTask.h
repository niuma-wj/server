// LoadPlayerTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_LOAD_PLAYER_TASK_H_
#define _NIU_MA_LOAD_PLAYER_TASK_H_

#include "MySql/MysqlQueryTask.h"

namespace NiuMa {
	class LoadPlayerTask : public MysqlQueryTask {
	public:
		LoadPlayerTask(const std::string& playerId);
		virtual ~LoadPlayerTask() {}

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;

	public:
		// 玩家id
		const std::string _playerId;

		// 登录账户名
		std::string _name;

		// 玩家昵称
		std::string _nickname;

		// 联系电话
		std::string _phone;

		// 性别
		int _sex;

		// 头像url
		std::string _avatar;
	};
}

#endif // !_NIU_MA_LOAD_PLAYER_TASK_H_
