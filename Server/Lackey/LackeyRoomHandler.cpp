// LackeyRoomHandler.cpp

#include "LackeyRoomHandler.h"

#include "LackeyMessages.h"

#include "../GameDefines.h"

namespace NiuMa
{
	LackeyRoomHandler::LackeyRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgLackeySync::TYPE);
		addMessage(MsgDoCallLackey::TYPE);
		addMessage(MsgLackeyDoShowCard::TYPE);
		addMessage(MsgLackeyDoPlayCard::TYPE);
		addMessage(MsgLackeyHintCard::TYPE);

		// 添加游戏类型
		addGameType(static_cast<int>(GameType::Lackey));
	}

	LackeyRoomHandler::~LackeyRoomHandler() {}
}