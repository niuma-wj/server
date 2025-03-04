// RobotManager.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.17

#ifndef _NIU_MA_ROBOT_MANAGER_H_
#define _NIU_MA_ROBOT_MANAGER_H_

#include "Base/Singleton.h"

#include <string>

namespace NiuMa
{
	class RobotManager : public Singleton<RobotManager> {
	private:
		RobotManager() = default;

	public:
		virtual ~RobotManager() = default;

		friend class Singleton<RobotManager>;

	public:
		/**
		 * 获取一个空闲的机器人id
		 * @param playerId 机器人玩家id
		 * @return 是否获取成功
		 */
		bool request(std::string& playerId);

		/**
		 * 释放一个机器人
		 * @param playerId 机器人玩家id
		 */
		void free(const std::string& playerId);
	};
}

#endif // !_NIU_MA_ROBOT_MANAGER_H_