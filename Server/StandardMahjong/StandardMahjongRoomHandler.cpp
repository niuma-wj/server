// StandardMahjongRoomHandler.cpp

#include "StandardMahjongRoomHandler.h"
#include "StandardMahjongMessages.h"

#include "../GameDefines.h"

namespace NiuMa
{
	StandardMahjongRoomHandler::StandardMahjongRoomHandler(const MessageQueue::Ptr& queue)
		: MahjongRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgMahjongSync::TYPE);

		// 添加游戏类型
		addGameType(static_cast<int>(GameType::Mahjong));
	}

	StandardMahjongRoomHandler::~StandardMahjongRoomHandler() {}
}