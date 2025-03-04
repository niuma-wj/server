// EchoSession.h

#ifndef _NIU_MA_ECHO_SESSION_H_
#define _NIU_MA_ECHO_SESSION_H_

#include "Session.h"

namespace NiuMa {
	/**
	 * 回响会话(用于测试)
	 *
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.17
	 */
	class EchoSession :public Session
	{
	public:
		EchoSession(const std::shared_ptr<Connection>& con);
		virtual ~EchoSession();

		class Creator : public SessionCreator {
		public:
			virtual Session::Ptr create(const std::shared_ptr<Connection>& con) const override {
				Session::Ptr sess = std::make_shared<EchoSession>(con);
				return sess;
			}
		};

	public:
		virtual void onRecieve(char* buf, std::size_t length) override;
	};
}

#endif // !_NIU_MA_ECHO_SESSION_H_
