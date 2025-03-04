// LackeyRoomHandler.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.31

#ifndef _NIU_MA_LACKEY_ROOM_HANDLER_H_
#define _NIU_MA_LACKEY_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	class LackeyRoomHandler : public GameRoomHandler
	{
	public:
		LackeyRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~LackeyRoomHandler();
	};
}

#endif // !_NIU_MA_LACKEY_ROOM_HANDLER_H_