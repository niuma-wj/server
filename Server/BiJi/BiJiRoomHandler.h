// BiJiRoomHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.11

#ifndef _NIU_MA_BI_JI_ROOM_HANDLER_H_
#define _NIU_MA_BI_JI_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	class BiJiRoomHandler : public GameRoomHandler
	{
	public:
		BiJiRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~BiJiRoomHandler();
	};
}

#endif // !_NIU_MA_BI_JI_ROOM_HANDLER_H_