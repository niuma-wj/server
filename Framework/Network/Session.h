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
	 * 会话基类
	 * 一个TCP网络连接对应一个会话，用于处理业务逻辑，而TCP网络连接类自身则封装了
	 * 底层网络IO逻辑，并且不暴露到上层业务逻辑
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
		 * 查询连接id
		 * 
		 * @param id 返回连接id
		 */
		void getId(std::string& id) const;

		/**
		 * 获取远端ip地址
		 * @return 返回远端ip地址
		 */
		const std::string& getRemoteIp() const;

		/**
		 * 接收到数据事件
		 * 
		 * @param buf 数据缓存
		 * @param length 数据长度
		 */
		virtual void onRecieve(char* buf, std::size_t length) = 0;

		/**
		 * 会话断开事件
		 */
		virtual void onDisconnect();

		/**
		 * 发送数据
		 *
		 * @param buf 数据缓存
		 * @param length 数据长度
		 */
		void send(const char* buf, std::size_t length);

		/**
		 * 发送数据
		 * @param data 数据缓存
		 */
		void send(const std::shared_ptr<std::string>& data);

		/**
		 * 返回会话是否仍然活跃，用于支持心跳检测
		 * 默认在连接未断开时返回true
		 */
		virtual bool isAlive(const time_t& nowTime) const;

		/**
		 * 心跳
		 * 默认不支持心跳，空函数
		 */
		virtual void heartbeat();

	private:
		// 连接
		std::weak_ptr<Connection> _connection;

		// 远端ip
		std::string _remoteIp;
	};

	/**
	 * 会话构造器基类
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
