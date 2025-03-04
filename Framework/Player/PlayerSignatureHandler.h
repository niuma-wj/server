// PlayerSignatureHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_PLAYER_SIGNATURE_HANDLER_H_
#define _NIU_MA_PLAYER_SIGNATURE_HANDLER_H_

#include "Message/MessageHandler.h"

namespace NiuMa {
	// 处理消息体的玩家签名数据
	class PlayerSignatureHandler : public MessageHandler {
	public:
		PlayerSignatureHandler(const MessageQueue::Ptr& queue);
		virtual ~PlayerSignatureHandler();

	protected:
		virtual bool preproccess(const NetMessage::Ptr& netMsg) override;
	};
}

#endif // !_NIU_MA_PLAYER_SIGNATURE_HANDLER_H_