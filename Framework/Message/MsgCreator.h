// MsgCreator.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.24

#ifndef _NIU_MA_MSG_CREATOR_H_
#define _NIU_MA_MSG_CREATOR_H_

#include "MsgBase.h"
#include "MsgWrapper.h"

namespace NiuMa {
	// 消息生成器基类
	class IMsgCreator
	{
	public:
		IMsgCreator() = default;
		virtual ~IMsgCreator() = default;

		typedef std::shared_ptr<IMsgCreator> Ptr;

	public:
		virtual MsgBase::Ptr deserialize(const MsgWrapper::Ptr& wrapper) const = 0;
	};

	template <typename T>
	class MsgCreator : public IMsgCreator
	{
	public:
		MsgCreator() = default;
		virtual ~MsgCreator() = default;

	public:
		virtual MsgBase::Ptr deserialize(const MsgWrapper::Ptr& wrapper) const override {
			T* ptr = new T();
			wrapper->unpackMessage(*ptr);
			MsgBase::Ptr msg(ptr);
			return msg;
		}
	};
}

#endif // !_NIU_MA_MSG_CREATOR_H_