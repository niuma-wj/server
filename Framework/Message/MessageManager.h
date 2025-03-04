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
		 * ע����Ϣ������
		 * @param type ��Ϣ����
		 * @param creator ������
		 */
		void registCreator(const std::string& type, const IMsgCreator::Ptr& creator);

		/**
		 * ������Ϣʵ��
		 * @param wrapper ��Ϣ����
		 */
		MsgBase::Ptr createMessage(const MsgWrapper::Ptr& wrapper);

		/**
		 * ע����Ϣ������
		 * @param handler ������
		 */
		void registHandler(const MessageHandler::Ptr& handler);

		/**
		 * ע����Ϣ������
		 * @param handler ������
		 */
		void unregistHandler(const MessageHandler::Ptr& handler);

		// ��ѯ��Ϣ��������������
		int getHandlerSN();

		// ��ȡȫ����Ϣ������
		void getAllHandlers(std::vector<MessageHandler::Ptr>& vec);

	private:
		// ��Ϣ��������
		// ԭ���ϸñ�ᱻ����̷߳��ʣ�������Ϊ�ñ�Ĳ������ڳ����ʼ���׶���ɵģ�
		// ���������߳�ֻ���ȡ�ñ�������޸ģ����Բ���Ҫ�ñ����߳�ͬ������
		std::unordered_map<std::string, IMsgCreator::Ptr> _creatorMap;

		// ��Ϣ��������
		std::vector<MessageHandler::Ptr> _handlers;

		// ��Ϣ��������������
		int _handlerSN = 0;

		// 
		std::mutex _mtxHandler;
	};
}

#endif // !_NIU_MA_MSG_DISPATCHER_H_