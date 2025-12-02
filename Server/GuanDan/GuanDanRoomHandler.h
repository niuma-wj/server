// GuanDanRoomHandler.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.06.05

#ifndef _NIU_MA_GUAN_DAN_ROOM_HANDLER_H_
#define _NIU_MA_GUAN_DAN_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	class GuanDanRoomHandler : public GameRoomHandler
	{
	public:
		GuanDanRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~GuanDanRoomHandler();
	};
}

#endif // !_NIU_MA_GUAN_DAN_ROOM_HANDLER_H_