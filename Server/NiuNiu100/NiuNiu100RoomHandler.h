// NiuNiu100RoomHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2025.02.19

#ifndef _NIU_MA_NIU_NIU_100_ROOM_HANDLER_H_
#define _NIU_MA_NIU_NIU_100_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	class NiuNiu100RoomHandler : public GameRoomHandler
	{
	public:
		NiuNiu100RoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~NiuNiu100RoomHandler();
	};
}

#endif // !_NIU_MA_NIU_NIU_100_ROOM_HANDLER_H_