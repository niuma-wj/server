// TcpServer.h

#ifndef _NIU_MA_TCP_SERVER_H_
#define _NIU_MA_TCP_SERVER_H_

#include "Base/Singleton.h"
#include "Session.h"

#include <atomic>
#include <thread>
#include <vector>
#include <unordered_map>
#include <list>
#include <mutex>

namespace NiuMa {
	/**
	 * TCP网络服务器
	 *
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.10
	 */
	class Service;
	class ConnectionImpl;
	class Acceptor;
	class TcpServer : public std::enable_shared_from_this<TcpServer> {
	public:
		TcpServer(const SessionCreator::Ptr& creator);
		virtual ~TcpServer();

		friend class Singleton<TcpServer>;
		friend class ConnectionImpl;
		friend class Acceptor;

	public:
		/**
		 * 启动服务器
		 * 
		 * @param port 端口号
		 * @param threadNum 接收数据线程数量(最大不超过8个线程)
		 */
		void start(unsigned short port, unsigned int threadNum = 0);

		// 关闭
		void stop();

		// 服务器是否正在关闭
		bool isStopping() const;

	private:
		// 循环获取服务
		std::shared_ptr<Service> getService();

		// 创建会话
		Session::Ptr newSession(const std::shared_ptr<Connection>& con);

		// 添加新的连接
		void addConnection(const std::shared_ptr<ConnectionImpl>& con);

		// 删除连接
		void removeConnection(const std::shared_ptr<ConnectionImpl>& con);

		// 断开全部连接
		void closeAllConnections();

		// 清空连接表
		void clearConnection();

		// 获取当前连接数量
		int getConnectionNums();

		// 按顺序从连接表中取出连接
		std::shared_ptr<ConnectionImpl> getConnectionInSequence();

		// 定时检查连接
		void checkConnection();

		// 定时任务
		bool onTimer();

	private:
		// 会话创建者
		SessionCreator::Ptr _creator;

		// 启动标志
		std::atomic<bool> _started;

		// 正在关闭
		std::atomic<bool> _stopping;

		// 线程数组
		std::vector<std::shared_ptr<std::thread> > _threads;

		// 监听器
		std::shared_ptr<Acceptor> _acceptor;

		// 服务列表
		std::vector<std::shared_ptr<Service> > _services;

		// 当前索引
		std::size_t _pos;

		// 连接表
		typedef std::unordered_map<std::string, std::shared_ptr<ConnectionImpl> > ConnectionMap;
		ConnectionMap _connections;

		// 连接表关键字顺序序列，用于顺序从连接表中取出连接
		std::list<std::string> _sequence;

		// 连接表信号量
		std::mutex _mtxCon;
	};
}

#endif // !_NIU_MA_TCP_SERVER_H_