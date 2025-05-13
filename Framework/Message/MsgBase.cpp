// MsgBase.cpp

#include "Base/Log.h"
#include "MsgBase.h"

namespace NiuMa {
	MsgBase::MsgBase() {}

	MsgBase::~MsgBase() {}

	std::shared_ptr<std::string> MsgBase::pack() const {
		return nullptr;
	}

	void MsgBase::send(const Session::Ptr& session) const {
		if (!session)
			return;
		std::shared_ptr<std::string> data = pack();
		if (!data || data->empty()) {
			ErrorS << "Pack message(type: \"" << getType() << "\") failed, check whether the \"MSG_PACK_IMPL\" macro has been added to the message declaration.";
			return;
		}
		session->send(data);
	}
}