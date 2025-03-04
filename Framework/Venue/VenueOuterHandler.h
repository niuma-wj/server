// VenueOuterHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.25

#ifndef _NIU_MA_VENUE_OUTER_HANDLER_H_
#define _NIU_MA_VENUE_OUTER_HANDLER_H_

#include "Player/PlayerSignatureHandler.h"

namespace NiuMa {
	/**
	 * 场地外部消息处理器
	 * 这类消息处理器通常使用共享的消息队列，同一类消息可以由不同的处理器实例并发处理
	 * 这类消息处理器通常用来处理场地外的逻辑，例如加载场地、进入场地、获取玩家数据等，
	 * 这类消息处理器可以直接执行阻塞任务（如数据库访问、网络IO等）
	 */
	class VenueOuterHandler : public PlayerSignatureHandler {
	public:
		VenueOuterHandler(const MessageQueue::Ptr& queue);
		virtual ~VenueOuterHandler();

	protected:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	private:
		/**
		 * 响应玩家连入消息
		 */
		void onPlayerConnect(const NetMessage::Ptr& netMsg);

		/**
		 * 响应连接断开消息
		 */
		void onDisconnect(const NetMessage::Ptr& netMsg);

		/**
		 * 响应玩家请求进入场地消息
		 */
		void onEnterVenue(const NetMessage::Ptr& netMsg);

		/**
		 * 响应离开场地消息
		 * @param netMsg 网络消息
		 */
		void onLeaveVenue(const NetMessage::Ptr& netMsg);
	};
}

#endif // !_NIU_MA_VENUE_OUTER_HANDLER_H_