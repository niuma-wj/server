// BiJiRoomHandler.cpp

#include "BiJiRoomHandler.h"
#include "BiJiMessages.h"

#include "../GameDefines.h"

namespace NiuMa
{
	BiJiRoomHandler::BiJiRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgBiJiSync::TYPE);
		addMessage(MsgBiJiStartGame::TYPE);
		addMessage(MsgBiJiMakeDun::TYPE);
		addMessage(MsgBiJiRevocateDun::TYPE);
		addMessage(MsgBiJiResetDun::TYPE);
		addMessage(MsgBiJiFixDun::TYPE);
		addMessage(MsgBiJiGiveUp::TYPE);

		// 添加游戏类型
		addGameType(static_cast<int>(GameType::LiuAnBiJi));
	}

	BiJiRoomHandler::~BiJiRoomHandler() {}
}