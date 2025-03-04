// NiuNiu100RoomHandler.cpp

#include "NiuNiu100RoomHandler.h"
#include "NiuNiu100Messages.h"

#include "../GameDefines.h"

namespace NiuMa
{
	NiuNiu100RoomHandler::NiuNiu100RoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgNiu100Sync::TYPE);
		addMessage(MsgNiu100Bet::TYPE);
		addMessage(MsgNiu100GetRankList::TYPE);
		addMessage(MsgNiu100GetTrend::TYPE);

		// 添加游戏类型
		addGameType(static_cast<int>(GameType::NiuNiu100));
	}

	NiuNiu100RoomHandler::~NiuNiu100RoomHandler() {}
}