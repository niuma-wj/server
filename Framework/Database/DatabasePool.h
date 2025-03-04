// DatabasePool.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.06

#ifndef _NIU_MA_DATA_BASE_POOL_H_
#define _NIU_MA_DATA_BASE_POOL_H_

#include "DatabaseConnection.h"
#include "Thread/ThreadBlocker.h"

#include <atomic>
#include <list>
#include <queue>

namespace NiuMa {
	class DatabasePool : public std::enable_shared_from_this<DatabasePool> {
	public:
		DatabasePool(int keepConnections, int maxConnections);
		virtual ~DatabasePool() = default;

	public:
		/**
		 * 定时任务，删除超时没有被使用的连接，仅保留设定的连接数
		 * 定时发起测试查询，以免连接长时间不活动被数据库服务器关闭
		 */
		bool onTimer();

		// 设置停止标志
		void stop();

	protected:
		/**
		 * 请求数据库连接(当连接超出最大数量时阻塞调用线程)
		 * @return 数据库连接
		 */
		DatabaseConnection::Ptr getConnection();

		/**
		 * 删除数据库连接
		 * @param con 数据库连接
		 */
		void removeConnection(const DatabaseConnection::Ptr& con);

		/**
		 * 唤醒阻塞队列中第一个线程
		 */
		void notifyOne();

		//
		DatabaseConnection::Ptr onTimerImpl();

		// 清空连接
		void clearConnections();

	protected:
		/**
		 * 创建数据库连接实例
		 * @return 数据库连接实例
		 */
		virtual DatabaseConnection::Ptr createConnection() = 0;

		/**
		 * 实现类的定时任务
		 * @param con 数据库连接
		 */
		virtual void onTimerImpl(const DatabaseConnection::Ptr& con) = 0;

	private:
		/**
		 * 请求数据库连接
		 * @param blocker 连接数量超出最大限制时，返回阻塞器，用于阻塞当前调用线程
		 * @return 数据库连接
		 */
		DatabaseConnection::Ptr getConnection(ThreadBlocker::Ptr& blocker);

	private:
		// 保留的连接数量
		const int _keepConnections;

		// 最大连接数量
		const int _maxConnections;

		// mysql连接列表
		std::list<DatabaseConnection::Ptr> _connections;

		// 阻塞队列
		std::queue<ThreadBlocker::Ptr> _blockQueue;

		//
		std::mutex _mtx;

	protected:
		// 停止标志
		std::atomic_bool _stopFlag;
	};
}

#endif _NIU_MA_DATA_BASE_POOL_H_