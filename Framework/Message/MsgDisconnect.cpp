// MsgDisconnect.cpp

#include "MsgDisconnect.h"

namespace NiuMa {
	const std::string MsgDisconnect::TYPE("MsgDisconnect");

	const std::string& MsgDisconnect::getType() const {
		return MsgDisconnect::TYPE;
	}
}