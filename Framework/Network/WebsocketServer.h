// WebsocketServer.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.09.25

#ifndef _NIU_MA_WEBSOCKET_SERVER_H_
#define _NIU_MA_WEBSOCKET_SERVER_H_

#include "Session.h"

#include <unordered_map>
#include <list>
#include <mutex>

namespace NiuMa
{
	class WebsocketConnection;
	class WebsocketService;
	class WebsocetAcceptor;

	/**
	 * Websocket服务器
	 */
	class WebsocketServer : public std::enable_shared_from_this<WebsocketServer>
	{
	public:
		WebsocketServer(const SessionCreator::Ptr& creator);
		virtual ~WebsocketServer();

		friend class WebsocketConnection;
		friend class WebsocetAcceptor;

	public:
		/**
		 * 启动服务器
		 *
		 * @param port 端口号
		 * @param threadNum 接收数据线程数量(最大不超过8个线程)
		 */
		bool start(int port, int threadNum = 0);

		// 关闭
		void stop();

		// 服务器是否正在关闭
		bool isStopping() const;

	private:
		// 创建会话
		Session::Ptr createSession(const std::shared_ptr<Connection>& con) const;

		// 添加新的连接
		void addConnection(const std::shared_ptr<Connection>& con);

		// 删除连接
		void removeConnection(const std::shared_ptr<Connection>& con);

		// 断开全部连接
		void closeAllConnections();

		// 清空连接表
		void clearConnection();

		// 获取当前连接数量
		int getConnectionNums();

		// 按顺序从连接表中取出连接
		std::shared_ptr<Connection> getConnectionInSequence();

		// 删除废弃超过10秒的连接
		void removeAbandoned();

		// 定时检查连接
		void checkConnection();

		// 定时任务
		bool onTimer();

	private:
		//
		std::shared_ptr<WebsocketService> _service;

		// 会话创建者
		SessionCreator::Ptr _creator;

		// 启动标志
		std::atomic<bool> _started;

		// 正在关闭
		std::atomic<bool> _stopping;

		// 线程数组
		std::vector<std::shared_ptr<std::thread> > _threads;

		// 连接表
		typedef std::unordered_map<std::string, std::shared_ptr<Connection> > ConnectionMap;
		ConnectionMap _connections;

		// 连接表关键字顺序序列，用于顺序从连接表中取出连接
		std::list<std::string> _sequence;

		// 已废弃的连接表
		std::list<std::shared_ptr<Connection> > _abandonedConnections;

		// 连接表信号量
		std::mutex _mtxCon;
	};
}


#endif // !_NIU_MA_WEBSOCKET_SERVER_H_