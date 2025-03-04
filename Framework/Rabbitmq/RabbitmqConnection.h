// RabbitmqConnection.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.15

#ifndef _NIU_MA_RABBITMQ_CONNECTION_H_
#define _NIU_MA_RABBITMQ_CONNECTION_H_

#include <mutex>
#include <memory>

namespace NiuMa {
	class RabbitmqConnection {
	public:
		RabbitmqConnection();
		virtual ~RabbitmqConnection();

		typedef std::shared_ptr<RabbitmqConnection> Ptr;

	public:
		//
		virtual void* getState() = 0;

		// 
		virtual void setOk(bool setting);

		// 
		bool isOk() const;

	private:
		// 当前连接是否正常
		bool _ok;

		// 信号量
		mutable std::mutex _mtx;
	};
}

#endif // !_NIU_MA_RABBITMQ_CONNECTION_H_