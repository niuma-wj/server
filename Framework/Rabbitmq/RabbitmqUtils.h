// RabbitmqUtils.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.09

#ifndef _NIU_MA_RABBITMQ_UTILS_H_
#define _NIU_MA_RABBITMQ_UTILS_H_

#include "rabbitmq-c/amqp.h"

namespace NiuMa {
	class RabbitmqUtils {
	private:
		RabbitmqUtils() = default;

	public:
		/**
		 * 检查响应错误
		 * @param reply 响应
		 * @param context 上下文
		 * @param channelClosed 返回通道是否关闭
		 * @return 0-无错误，1-发生错误且连接关闭，2-发生错误且通道关闭，3-发生错误
		 */
		static int checkReplyError(const amqp_rpc_reply_t& reply, const char* context);
	};
}

#endif //!_NIU_MA_RABBITMQ_UTILS_H_