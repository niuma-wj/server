// MsgWrapper.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.18

#ifndef _NIU_MA_MSG_WRAPPER_H_
#define _NIU_MA_MSG_WRAPPER_H_

#include <string>
#include <sstream>
#include <memory>

#include "msgpack/msgpack.hpp"

#include "Base/BaseUtils.h"

namespace NiuMa {
	// 网络消息数据封装器
	class MsgWrapper
	{
	public:
		MsgWrapper();
		MsgWrapper(const std::string& type);
		MsgWrapper(const std::string& type, const std::string& pack);
		virtual ~MsgWrapper();

		typedef std::shared_ptr<MsgWrapper> Ptr;

	public:
		const std::string& getType() const;
		void setType(const std::string& type);
		const std::string& getPack() const;
		void setPack(const std::string& pack);

		template<typename T>
		bool packMessage(const T& msg, std::string& pack) {
			try {
				msgpack::sbuffer sbuf1;
				msgpack::pack(sbuf1, msg);
				std::string content(sbuf1.data(), sbuf1.size());
				if (!BaseUtils::encodeBase64(msgPack, content.data(), static_cast<int>(content.size())))
					return false;
				msgType = msg.getType();
				msgpack::sbuffer sbuf2;
				msgpack::pack(sbuf2, *this);
				pack.assign(sbuf2.data(), sbuf2.size());
			}
			catch (std::exception&) {
				return false;
			}
			return true;
		}

		template<typename T>
		bool unpackMessage(T& msg) const {
			try {
				std::string content;
				if (!BaseUtils::decodeBase64(msgPack, content))
					return false;
				msgpack::object_handle oh =
					msgpack::unpack(content.data(), content.size());
				const msgpack::object& obj = oh.get();
				//std::stringstream ss;
				//ss << obj;
				//std::string text = ss.str();
				obj.convert(msg);
			}
			catch (std::exception&) {
				return false;
			}
			return true;
		}

	private:
		// 消息体类型
		std::string msgType;

		// 消息体MessagePack打包数据
		std::string msgPack;

	public:
		MSGPACK_DEFINE_MAP(msgType, msgPack);
	};
}

#endif // !_XCMP_MSG_WRAPPER_H_