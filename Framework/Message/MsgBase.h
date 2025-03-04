// MsgBase.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.19

#ifndef _NIU_MA_MSG_BASE_H_
#define _NIU_MA_MSG_BASE_H_

#include "Network/Session.h"

// ��Ϣ���ʵ�ַ����궨��
#define MSG_PACK_IMPL	\
	virtual std::shared_ptr<std::string> pack() const override {\
		MsgWrapper mw;\
		std::shared_ptr<std::string> data = std::make_shared<std::string>();\
		if (mw.packMessage(*this, *data))\
			return data;\
		return nullptr;\
	}

namespace NiuMa {
	// ��Ϣ���࣬������Ϣ�Ļ���
	class MsgBase
	{
	public:
		MsgBase();
		virtual ~MsgBase();

		typedef std::shared_ptr<MsgBase> Ptr;

	public:
		/**
		 * ��ȡ��Ϣ����
		 */
		virtual const std::string& getType() const = 0;

		/**
		 * ��Ϣ���
		 * @return ������ݣ�Ĭ�Ϸ���nullptr
		 */
		virtual std::shared_ptr<std::string> pack() const;

	public:
		/**
		 * ������Ϣ
		 * @param session ���ӻỰ
		 */
		void send(const Session::Ptr& session) const;
	};
}

#endif // !_NIU_MA_MSG_BASE_H_
