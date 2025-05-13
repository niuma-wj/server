// MsgWrapper.cpp

#include "MsgWrapper.h"

namespace NiuMa
{
	MsgWrapper::MsgWrapper() {}

	MsgWrapper::MsgWrapper(const std::string& type)
		: msgType(type)
	{}

	MsgWrapper::MsgWrapper(const std::string& type, const std::string& pack)
		: msgType(type)
		, msgPack(pack)
	{}

	MsgWrapper::~MsgWrapper() {}

	const std::string& MsgWrapper::getType() const {
		return msgType;
	}

	void MsgWrapper::setType(const std::string& type) {
		msgType = type;
	}

	const std::string& MsgWrapper::getPack() const {
		return msgPack;
	}

	void MsgWrapper::setPack(const std::string& pack) {
		msgPack = pack;
	}
}