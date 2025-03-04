// MsgDisconnect.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.24

#ifndef _NIU_MA_MSG_DISCONNECT_H_
#define _NIU_MA_MSG_DISCONNECT_H_

#include "MsgBase.h"
#include <string>

namespace NiuMa {
	/**
	 * 网络连接断开消息
	 */
	class MsgDisconnect : public MsgBase
	{
	public:
		MsgDisconnect() = default;
		virtual ~MsgDisconnect() = default;

		static const std::string TYPE;

	public:
		virtual const std::string& getType() const override;

	public:
		void setSessionId(const std::string& sessionId) {
			_sessionId = sessionId;
		}

		const std::string& getSessionId() const {
			return _sessionId;
		}

	private:
		// 会话id
		std::string _sessionId;
	};
}

#endif // !_NIU_MA_MSG_DISCONNECT_H_