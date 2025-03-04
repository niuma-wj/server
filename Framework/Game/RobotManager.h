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
		 * ��ȡһ�����еĻ�����id
		 * @param playerId ���������id
		 * @return �Ƿ��ȡ�ɹ�
		 */
		bool request(std::string& playerId);

		/**
		 * �ͷ�һ��������
		 * @param playerId ���������id
		 */
		void free(const std::string& playerId);
	};
}

#endif // !_NIU_MA_ROBOT_MANAGER_H_