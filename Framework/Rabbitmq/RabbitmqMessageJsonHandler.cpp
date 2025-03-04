// RabbitmqMessageJsonHandler.cpp

#include "Base/BaseUtils.h"
#include "RabbitmqMessageJsonHandler.h"

#include <sstream>
#include "jsoncpp/include/json/json.h"

namespace NiuMa {
	RabbitmqMessageJsonHandler::RabbitmqMessageJsonHandler(const std::string& tag)
		: RabbitmqMessageHandler(tag)
	{}

	RabbitmqMessageJsonHandler::~RabbitmqMessageJsonHandler() {}

	void RabbitmqMessageJsonHandler::handleImpl(const std::string& message) {
		std::stringstream ss(message);
		Json::Value jsonObj;
		ss >> jsonObj;
		Json::Value& typeObj = jsonObj["msgType"];
		if (!typeObj.isString())
			return;
		Json::Value& packObj = jsonObj["msgPack"];
		if (!packObj.isString())
			return;
		std::string msgType = typeObj.asString();
		std::string msgPack = packObj.asString();
		if (msgType.empty() || msgPack.empty())
			return;
		std::string json;
		if (!BaseUtils::decodeBase64(msgPack, json))
			return;
		if (json.empty())
			return;
		handleImpl(msgType, json);
	}
}