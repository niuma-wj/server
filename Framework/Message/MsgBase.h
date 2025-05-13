// MsgBase.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.19

#ifndef _NIU_MA_MSG_BASE_H_
#define _NIU_MA_MSG_BASE_H_

#include "Network/Session.h"

// 消息打包实现方法宏定义
#define MSG_PACK_IMPL	\
	virtual std::shared_ptr<std::string> pack() const override {\
		MsgWrapper mw;\
		std::shared_ptr<std::string> data = std::make_shared<std::string>();\
		if (mw.packMessage(*this, *data))\
			return data;\
		return nullptr;\
	}

namespace NiuMa {
	// 消息基类，所有消息的基类
	class MsgBase
	{
	public:
		MsgBase();
		virtual ~MsgBase();

		typedef std::shared_ptr<MsgBase> Ptr;

	public:
		/**
		 * 获取消息类型
		 */
		virtual const std::string& getType() const = 0;

		/**
		 * 消息打包
		 * @return 打包数据，默认返回nullptr
		 */
		virtual std::shared_ptr<std::string> pack() const;

	public:
		/**
		 * 发送消息
		 * @param session 连接会话
		 */
		void send(const Session::Ptr& session) const;
	};
}

#endif // !_NIU_MA_MSG_BASE_H_
