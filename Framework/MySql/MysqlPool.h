// MysqlPool.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.30

#ifndef _NIU_MA_MYSQL_POOL_H_
#define _NIU_MA_MYSQL_POOL_H_

#include "Thread/ThreadPool.h"
#include "Base/Singleton.h"
#include "MysqlQueryTask.h"

namespace NiuMa {
	class MysqlPoolImpl;
	class MysqlPool : public ThreadPool, public Singleton<MysqlPool> {
	private:
		MysqlPool() = default;

	public:
		virtual ~MysqlPool() = default;

		friend class Singleton<MysqlPool>;

	public:
		/**
		 * ����
		 * @param hostName ���ݿ��������ַ(�磺tcp://127.0.0.1:3306)
		 * @param userName �û���
		 * @param password ����
		 * @param schemaName ���ݿ���
		 * @param keepConnections ��������������
		 * @param maxConnections �����������(����������������������ݿⱨ��)
		 * @param threadNum �첽�߳�����(��СΪ1�����Ϊ8)
		 */
		void start(const std::string& hostName,
			const std::string& userName,
			const std::string& password,
			const std::string& schemaName,
			int keepConnections,
			int maxConnections,
			int threadNum);

		/**
		 * ֹͣ
		 */
		virtual void stop() override;

		/**
		 * ͬ��ִ��mysql��ѯ����
		 * @param task mysql��ѯ����
		 */
		void syncQuery(const MysqlQueryTask::Ptr& task);

		/**
		 * ͬ��ִ�в���Ҫ���ؽ����sql��䣬����insert��update��delete
		 * @param sql sql���
		 * @param type ��ѯ����
		 */
		void syncQuery(const std::string& sql, MysqlQueryTask::QueryType type);

		/**
		 * �첽ִ��mysql��ѯ����
		 * @param task mysql��ѯ����
		 * @param dispatcher ��ǲ���߳�(����Ϊ�գ����ڸ��߳��е���MysqlTask::onQueried����)
		 */
		void asyncQuery(const MysqlQueryTask::Ptr& task, const ThreadWorker::Ptr& dispatcher = nullptr);

		/**
		 * �첽ִ�в���Ҫ���ؽ����sql��䣬����insert��update��delete
		 * @param sql sql���
		 * @param type ��ѯ����
		 * @param dispatcher ��ǲ���߳�(����Ϊ�գ����ڸ��߳��е���MysqlTask::onQueried����)
		 */
		void asyncQuery(const std::string& sql, MysqlQueryTask::QueryType type, const ThreadWorker::Ptr& dispatcher = nullptr);

	private:
		// ʵ��ʵ��
		std::shared_ptr<MysqlPoolImpl> _impl;
	};
}

#endif // !_NIU_MA_MYSQL_POOL_H_