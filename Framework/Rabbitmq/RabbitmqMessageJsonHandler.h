// RabbitmqMessageJsonHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.30

#ifndef _NIU_MA_RABBITMQ_MESSAGE_JSON_HANDLER_H_
#define _NIU_MA_RABBITMQ_MESSAGE_JSON_HANDLER_H_

#include "RabbitmqMessageHandler.h"

namespace NiuMa {
	/**
	 * Json格式消息处理器
	 */
	class RabbitmqMessageJsonHandler : public RabbitmqMessageHandler {
	public:
		RabbitmqMessageJsonHandler(const std::string& tag);
		virtual ~RabbitmqMessageJsonHandler();

	protected:
		virtual void handleImpl(const std::string& message) override;

		/**
		 * 处理消息
		 * @param msgType 消息类型
		 * @param json json消息体
		 */
		virtual void handleImpl(const std::string& msgType, const std::string& json) = 0;
	};
}

#endif // !_NIU_MA_RABBITMQ_MESSAGE_JSON_HANDLER_H_