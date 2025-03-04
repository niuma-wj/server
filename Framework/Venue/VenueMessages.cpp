// VenueMessages.cpp

#include "VenueMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgEnterVenue::TYPE("MsgEnterVenue");
	const std::string MsgEnterVenueResp::TYPE("MsgEnterVenueResp");
	const std::string MsgHeartbeat::TYPE("MsgHeartbeat");
	const std::string MsgHeartbeatResp::TYPE("MsgHeartbeatResp");
	const std::string MsgLeaveVenue::TYPE("MsgLeaveVenue");
	const std::string MsgLeaveVenueResp::TYPE("MsgLeaveVenueResp");
	
	void VenueMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgEnterVenue>());
		MessageManager::getSingleton().registCreator(MsgEnterVenue::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgHeartbeat>());
		MessageManager::getSingleton().registCreator(MsgHeartbeat::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgLeaveVenue>());
		MessageManager::getSingleton().registCreator(MsgLeaveVenue::TYPE, creator);
	}
}