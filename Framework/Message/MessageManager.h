// MessageManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.19

#ifndef _NIU_MA_MESSAGE_MANAGER_H_
#define _NIU_MA_MESSAGE_MANAGER_H_

#include "Base/Singleton.h"
#include "MsgCreator.h"
#include "MessageHandler.h"

#include <vector>
#include <unordered_map>

namespace NiuMa {
	class MessageManager : public Singleton<MessageManager>
	{
	public:
		MessageManager();
		virtual ~MessageManager() = default;

		friend class Singleton<MessageManager>;

	public:
		/**
		 * 注册消息生成器
		 * @param type 消息类型
		 * @param creator 生成器
		 */
		void registCreator(const std::string& type, const IMsgCreator::Ptr& creator);

		/**
		 * 创建消息实例
		 * @param wrapper 消息数据
		 */
		MsgBase::Ptr createMessage(const MsgWrapper::Ptr& wrapper);

		/**
		 * 注册消息处理器
		 * @param handler 处理器
		 */
		void registHandler(const MessageHandler::Ptr& handler);

		/**
		 * 注销消息处理器
		 * @param handler 处理器
		 */
		void unregistHandler(const MessageHandler::Ptr& handler);

		// 查询消息处理器表更新序号
		int getHandlerSN();

		// 获取全部消息处理器
		void getAllHandlers(std::vector<MessageHandler::Ptr>& vec);

	private:
		// 消息生成器表
		// 原则上该表会被多个线程访问，但是因为该表的插入是在程序初始化阶段完成的，
		// 后续其他线程只会读取该表而不是修改，所以不需要该表做线程同步操作
		std::unordered_map<std::string, IMsgCreator::Ptr> _creatorMap;

		// 消息处理器表
		std::vector<MessageHandler::Ptr> _handlers;

		// 消息处理器表更新序号
		int _handlerSN = 0;

		// 
		std::mutex _mtxHandler;
	};
}

#endif // !_NIU_MA_MSG_DISPATCHER_H_