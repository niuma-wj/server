// GuanDanRoomHandler.cpp

#include "GuanDanRoomHandler.h"
#include "GuanDanMessages.h"
#include "../GameDefines.h"

namespace NiuMa
{
	GuanDanRoomHandler::GuanDanRoomHandler(const MessageQueue::Ptr& queue)
		: GameRoomHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgGuanDanSync::TYPE);
		addMessage(MsgGuanDanStartGame::TYPE);
		addMessage(MsgPresentTribute::TYPE);
		addMessage(MsgRefundTribute::TYPE);
		addMessage(MsgGuanDanDoPlayCard::TYPE);
		addMessage(MsgGuanDanHintCard::TYPE);
		addMessage(MsgGuanDanResetHintCard::TYPE);
		addMessage(MsgGuanDanHintStraightFlush::TYPE);

		// 添加游戏类型
		addGameType(static_cast<int>(GameType::GuanDan));
	}

	GuanDanRoomHandler::~GuanDanRoomHandler() {}
}