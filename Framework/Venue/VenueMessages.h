// VenueMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.06

#ifndef _NIU_MA_VENUE_MESSAGES_H_
#define _NIU_MA_VENUE_MESSAGES_H_

#include "Player/PlayerMessages.h"

namespace NiuMa
{
	class VenueMessages
	{
	private:
		VenueMessages() {}

	public:
		virtual ~VenueMessages() {}

		static void registMessages();
	};

	/**
	 * 请求进入场地消息
	 * 客户端->服务器
	 */
	class MsgEnterVenue : public MsgPlayerSignature {
	public:
		MsgEnterVenue()
			: gameType(0)
		{}

		virtual ~MsgEnterVenue() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 场地id
		std::string venueId;

		// 游戏类型
		int gameType;

		// 额外信息(打包成base64)
		std::string base64;

		MSGPACK_DEFINE_MAP(/*MSGPACK_BASE_MAP(MsgPlayerSignature)*/playerId, timestamp, nonce, signature, venueId, gameType, base64);
	};

	/**
	 * 响应进入场地消息
	 * 服务器->客户端
	 */
	class MsgEnterVenueResp : public MsgBase {
	public:
		MsgEnterVenueResp()
			: code(0)
		{}

		virtual ~MsgEnterVenueResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 场地id
		std::string venueId;

		// 结果代码(0-成功，1-未授权，2-游戏类型错误，3-场地状态错误，4-场地加载失败，5-场地分配失败，6-进入失败(例如已满人)，7-其他错误)
		int code;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(venueId, code, errMsg);
	};

	/**
	 * 场地内消息基类
	 * 所有场地内消息都继承自该类
	 * 客户端->服务器
	 */
	class MsgVenueInner : public MsgPlayerSignature {
	public:
		MsgVenueInner() {}
		virtual ~MsgVenueInner() {}

	public:
		/**
		 * 返回场地id
		 */
		const std::string& getVenueId() const {
			return venueId;
		}

	protected:
		// 场地id
		std::string venueId;
	};

	/**
	 * 场地内心跳消息
	 * 玩家必须进入场地后才能发送心跳，否则心跳无意义
	 * 客户端->服务器
	 */
	class MsgHeartbeat : public MsgVenueInner
	{
	public:
		MsgHeartbeat() {}
		virtual ~MsgHeartbeat() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 计数器
		int counter = 0;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, counter);
	};

	/**
	 * 心跳响应消息
	 * 服务器->客户端
	 */
	class MsgHeartbeatResp : public MsgBase {
	public:
		MsgHeartbeatResp() {}
		virtual ~MsgHeartbeatResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 计数器
		int counter = 0;

		MSGPACK_DEFINE_MAP(counter);
	};

	/**
	 * 离开场地消息
	 * 注意，该消息虽然是场地内消息，但却由VenueOuterHandler处理，
	 * 以便在场地已经不存在(被释放)的情况下，能正常退出场地
	 * 客户端->服务器
	 */
	class MsgLeaveVenue : public MsgVenueInner
	{
	public:
		MsgLeaveVenue() {}
		virtual ~MsgLeaveVenue() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应离开场地消息
	 * 服务器->客户端
	 */
	class MsgLeaveVenueResp : public MsgBase {
	public:
		MsgLeaveVenueResp()
			: result(0)
		{}

		virtual ~MsgLeaveVenueResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 场地id
		std::string venueId;

		// 离开结果，0-成功离开，其他-离开失败
		int result;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(venueId, result, errMsg);
	};
}

#endif // !_NIU_MA_VENUE_MESSAGES_H_