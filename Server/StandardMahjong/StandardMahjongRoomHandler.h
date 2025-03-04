// StandardMahjongRoomHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.11

#ifndef _NIU_MA_STANDARD_MAHJONG_ROOM_HANDLER_H_
#define _NIU_MA_STANDARD_MAHJONG_ROOM_HANDLER_H_

#include "MahjongRoomHandler.h"

namespace NiuMa
{
	class StandardMahjongRoomHandler : public MahjongRoomHandler
	{
	public:
		StandardMahjongRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~StandardMahjongRoomHandler();
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_ROOM_HANDLER_H_