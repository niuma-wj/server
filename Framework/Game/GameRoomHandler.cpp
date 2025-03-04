// GameRoomHandler.cpp

#include "GameRoom.h"
#include "GameRoomHandler.h"
#include "GameMessages.h"

namespace NiuMa {
	GameRoomHandler::GameRoomHandler(const MessageQueue::Ptr& queue)
		: VenueInnerHandler(queue)
	{
		// ��ӽ��յ���Ϣ����
		addMessage(MsgGetAvatars::TYPE);
		addMessage(MsgGetSpectators::TYPE);
		addMessage(MsgPlayerReady::TYPE);
		addMessage(MsgPlayerAuthorize::TYPE);
		addMessage(MsgPlayerGeolocation::TYPE);
		addMessage(MsgGetDistances::TYPE);
		addMessage(MsgDisbandRequest::TYPE);
		addMessage(MsgDisbandChoose::TYPE);
		addMessage(MsgChatClient::TYPE);
		addMessage(MsgEffectClient::TYPE);
		addMessage(MsgVoiceClient::TYPE);
	}

	GameRoomHandler::~GameRoomHandler() {}
}