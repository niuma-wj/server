// PlayerMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.06

#ifndef _NIU_MA_PLAYER_MESSAGES_H_
#define _NIU_MA_PLAYER_MESSAGES_H_

#include "Message/MsgBase.h"
#include "Message/MsgWrapper.h"

namespace NiuMa
{
	class PlayerMessages
	{
	private:
		PlayerMessages() {}

	public:
		virtual ~PlayerMessages() {}

		static void registMessages();
	};

	/**
	 * 消息体的玩家签名数据
	 * 所有需要签名的消息（绝大多数的消息都需要签名，特别是游戏逻辑相关的消息）都继承自该类
	 */
	class MsgPlayerSignature : public MsgBase {
	public:
		MsgPlayerSignature() {}
		virtual ~MsgPlayerSignature() {}

	public:
		/**
		 * 返回玩家id
		 */
		const std::string& getPlayerId() const {
			return playerId;
		}

		/**
		 * 返回unix时间戳
		 */
		const std::string& getTimestamp() const {
			return timestamp;
		}

		/**
		 * 返回随机串
		 */
		const std::string& getNonce() const {
			return nonce;
		}

		/**
		 * 返回签名
		 */
		const std::string& getSignature() const {
			return signature;
		}

	protected:
		// 玩家id
		std::string playerId;

		// unix时间戳（单位秒），1分钟内有效
		std::string timestamp;

		// 客户端生成的随机串，1分钟时间内不重复(防止重放攻击)
		std::string nonce;

		// md5签名
		std::string signature;

	//public:
		//MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature);
	};

	/**
	 * 响应玩家签名失败消息
	 */
	class MsgPlayerSignatureError : public MsgBase {
	public:
		MsgPlayerSignatureError() {}
		virtual ~MsgPlayerSignatureError() {}

		static const std::string TYPE;

	public:
		virtual const std::string& getType() const override {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 时间戳是否过期
		bool outdate = false;

		MSGPACK_DEFINE_MAP(outdate);
	};

	/**
	 * 玩家连接消息，玩家客户端连接到游戏服务器之后（或断线重连之后），首先立即发送该消息
	 * 通知服务器玩家连入（或重新连入）
	 */
	class MsgPlayerConnect : public MsgPlayerSignature {
	public:
		MsgPlayerConnect() {}
		virtual ~MsgPlayerConnect() {}

		static const std::string TYPE;

	public:
		virtual const std::string& getType() const override {
			return TYPE;
		}

	public:
		MSGPACK_DEFINE_MAP(/*MSGPACK_BASE_MAP(MsgPlayerSignature)*/playerId, timestamp, nonce, signature);
	};
}

#endif // !_NIU_MA_PLAYER_MESSAGES_H_