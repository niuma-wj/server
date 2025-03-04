// MahjongRoomHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.26

#ifndef _NIU_MA_MAHJONG_ROOM_HANDLER_H_
#define _NIU_MA_MAHJONG_ROOM_HANDLER_H_

#include "Game/GameRoomHandler.h"

namespace NiuMa
{
	/**
	 * �齫��Ϸ������Ϣ������
	 */
	class MahjongRoomHandler : public GameRoomHandler
	{
	public:
		MahjongRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~MahjongRoomHandler();
	};
}

#endif // !_NIU_MA_MAHJONG_ROOM_HANDLER_H_