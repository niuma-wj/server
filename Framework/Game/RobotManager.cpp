// RobotManager.cpp

#include "RobotManager.h"

#include "Base/BaseUtils.h"

namespace NiuMa
{
	template<> RobotManager* Singleton<RobotManager>::_inst = nullptr;

	bool RobotManager::request(std::string& playerId) {
		playerId = BaseUtils::EMPTY_STRING;
		return false;
	}

	void RobotManager::free(const std::string& playerId) {}
}