// Session.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.07.17

#ifndef _NIU_MA_SESSION_H_
#define _NIU_MA_SESSION_H_

#include <memory>
#include <string>

namespace NiuMa {
	/**
	 * �Ự����
	 * һ��TCP�������Ӷ�Ӧһ���Ự�����ڴ���ҵ���߼�����TCP�����������������װ��
	 * �ײ�����IO�߼������Ҳ���¶���ϲ�ҵ���߼�
	 */
	class Connection;
	class Session : public std::enable_shared_from_this<Session>
	{
	public:
		Session(const std::shared_ptr<Connection>& con);
		virtual ~Session();

		typedef std::shared_ptr<Session> Ptr;

	public:
		/**
		 * ��ѯ����id
		 * 
		 * @param id ��������id
		 */
		void getId(std::string& id) const;

		/**
		 * ��ȡԶ��ip��ַ
		 * @return ����Զ��ip��ַ
		 */
		const std::string& getRemoteIp() const;

		/**
		 * ���յ������¼�
		 * 
		 * @param buf ���ݻ���
		 * @param length ���ݳ���
		 */
		virtual void onRecieve(char* buf, std::size_t length) = 0;

		/**
		 * �Ự�Ͽ��¼�
		 */
		virtual void onDisconnect();

		/**
		 * ��������
		 *
		 * @param buf ���ݻ���
		 * @param length ���ݳ���
		 */
		void send(const char* buf, std::size_t length);

		/**
		 * ��������
		 * @param data ���ݻ���
		 */
		void send(const std::shared_ptr<std::string>& data);

		/**
		 * ���ػỰ�Ƿ���Ȼ��Ծ������֧���������
		 * Ĭ��������δ�Ͽ�ʱ����true
		 */
		virtual bool isAlive(const time_t& nowTime) const;

		/**
		 * ����
		 * Ĭ�ϲ�֧���������պ���
		 */
		virtual void heartbeat();

	private:
		// ����
		std::weak_ptr<Connection> _connection;

		// Զ��ip
		std::string _remoteIp;
	};

	/**
	 * �Ự����������
	 *
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.17
	 */
	class SessionCreator
	{
	public:
		SessionCreator();
		virtual ~SessionCreator();

		typedef std::shared_ptr<SessionCreator> Ptr;

	public:
		virtual Session::Ptr create(const std::shared_ptr<Connection>& con) const = 0;
	};
}

#endif // !_NIU_MA_SESSION_H_
