// GameRoomHandler.cpp

#include "GameRoom.h"
#include "GameRoomHandler.h"
#include "GameMessages.h"

namespace NiuMa {
	GameRoomHandler::GameRoomHandler(const MessageQueue::Ptr& queue)
		: VenueInnerHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgJoinGame::TYPE);
		addMessage(MsgBecomeSpectator::TYPE);
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