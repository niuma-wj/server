// MahjongRoomHandler.cpp

#include "MahjongRoomHandler.h"
#include "MahjongMessages.h"

namespace NiuMa
{
	MahjongRoomHandler::MahjongRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// ��ӽ��յ���Ϣ����
		addMessage(MsgDoActionOption::TYPE);
		addMessage(MsgPassActionOption::TYPE);
		addMessage(MsgNextTile::TYPE);
	}

	MahjongRoomHandler::~MahjongRoomHandler() {}
}