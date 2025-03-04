// GameRoomHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.26

#ifndef _NIU_MA_GAME_ROOM_HANDLER_H_
#define _NIU_MA_GAME_ROOM_HANDLER_H_

#include "Venue/VenueInnerHandler.h"

namespace NiuMa {
	/**
	 * 棋牌类游戏房间消息处理器
	 */
	class GameRoomHandler : public VenueInnerHandler {
	public:
		GameRoomHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~GameRoomHandler();
	};
}

#endif // !_NIU_MA_GAME_ROOM_HANDLER_H_