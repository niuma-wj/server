// Connection.h

#ifndef _NIU_MA_CONNECTION_H_
#define _NIU_MA_CONNECTION_H_

#include <memory>
#include <string>

namespace NiuMa {
	/**
	 * TCP�������ӽӿ�
	 * �ýӿ�Ϊ����ڲ��ӿڣ���Ҫ��¶���ⲿ�ϲ�ҵ���߼�!!!
	 *
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.17
	 */
	class Connection : public std::enable_shared_from_this<Connection>
	{
	public:
		Connection();
		virtual ~Connection();

		typedef std::shared_ptr<Connection> Ptr;

	public:
		/**
		 * ��ѯ����id
		 * @param id ��������id
		 */
		virtual void getId(std::string& id) const = 0;

		/**
		 * ��ȡԶ��IP��ַ
		 * @param remoteIp ����Զ��ip��ַ
		 */
		virtual void getRemoteIp(std::string& remoteIp) const = 0;

		/**
		 * ��������
		 * 
		 * @param buf ���ݻ���
		 * @param length ���ݳ���
		 */
		virtual void send(const char* buf, std::size_t length) = 0;

		/**
		 * ��������
		 *
		 * @param data ���ݻ���
		 */
		virtual void send(const std::shared_ptr<std::string>& data) = 0;

		/**
		 * �����Ƿ��ѶϿ�
		 */
		virtual bool isClosed() = 0;
	};
}

#endif // !_NIU_MA_CONNECTION_H_