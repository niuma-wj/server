// PlayerMessages.cpp

#include "PlayerMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgPlayerConnect::TYPE("MsgPlayerConnect");
	const std::string MsgPlayerSignatureError::TYPE("MsgPlayerSignatureError");

	void PlayerMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgPlayerConnect>());
		MessageManager::getSingleton().registCreator(MsgPlayerConnect::TYPE, creator);
	}
}