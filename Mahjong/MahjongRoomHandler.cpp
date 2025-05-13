// MahjongRoomHandler.cpp

#include "MahjongRoomHandler.h"
#include "MahjongMessages.h"

namespace NiuMa
{
	MahjongRoomHandler::MahjongRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgDoActionOption::TYPE);
		addMessage(MsgPassActionOption::TYPE);
		addMessage(MsgNextTile::TYPE);
	}

	MahjongRoomHandler::~MahjongRoomHandler() {}
}