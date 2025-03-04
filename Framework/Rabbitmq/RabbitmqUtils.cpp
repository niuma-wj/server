// RabbitmqUtils.cpp

#include "Base/Log.h"
#include "RabbitmqUtils.h"

namespace NiuMa {
	int RabbitmqUtils::checkReplyError(const amqp_rpc_reply_t& reply, const char* context) {
		if (AMQP_RESPONSE_NORMAL == reply.reply_type)
			return 0;
		int ret = 3;
		if (AMQP_RESPONSE_NONE == reply.reply_type) {
			if (context == nullptr)
				LOG_ERROR("Missing RPC reply type!");
			else {
				ErrorS << context << " missing RPC reply type!";
			}
		}
		else if (AMQP_RESPONSE_LIBRARY_EXCEPTION == reply.reply_type) {
			if (context == nullptr)
				LOG_ERROR(amqp_error_string2(reply.library_error));
			else {
				ErrorS << context << " " << amqp_error_string2(reply.library_error);
			}
		}
		else if (AMQP_RESPONSE_SERVER_EXCEPTION == reply.reply_type) {
			if (AMQP_CONNECTION_CLOSE_METHOD == reply.reply.id) {
				ret = 1;
				amqp_connection_close_t* m = (amqp_connection_close_t*)reply.reply.decoded;
				std::string msg((const char*)(m->reply_text.bytes), m->reply_text.len);
				if (context == nullptr) {
					ErrorS << "Connection error code: " << m->reply_code << ", message: " << msg;
				}
				else {
					ErrorS << context << " connection error code: " << m->reply_code << ", message: " << msg;
				}
			}
			else if (AMQP_CHANNEL_CLOSE_METHOD == reply.reply.id) {
				ret = 2;
				amqp_channel_close_t* m = (amqp_channel_close_t*)reply.reply.decoded;
				std::string msg((const char*)(m->reply_text.bytes), m->reply_text.len);
				if (context == nullptr) {
					ErrorS << "Server channel error code: " << m->reply_code << ", message: " << msg;
				}
				else {
					ErrorS << context << " server channel error code: " << m->reply_code << ", message: " << msg;
				}
			}
			else {
				char tmp[16] = { '\0' };
				snprintf(tmp, sizeof(tmp), "%08X", reply.reply.id);
				if (context == nullptr) {
					ErrorS << "Unknown server error, method id: " << tmp;
				}
				else {
					ErrorS << context << " unknown server error, method id: " << tmp;
				}
			}
		}
		return ret;
	}
}