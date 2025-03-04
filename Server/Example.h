// Example.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.01

#ifndef _NIU_MA_EXAMPLE_H_
#define _NIU_MA_EXAMPLE_H_

#include "MySql/MysqlQueryTask.h"
#include "Thread/ThreadWorker.h"
#include "Rabbitmq/RabbitmqMessageHandler.h"

#include <vector>

namespace NiuMa {
	// 测试查询记录
	class TestSelect : public MysqlQueryTask {
	public:
		TestSelect() = default;
		virtual ~TestSelect() = default;

		class Record {
		public:
			int _id = 0;
			std::string _field1;
			std::string _field2;
		};

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual int fetchResult(sql::ResultSet* res) override;
		virtual void onQueried() override;

	private:
		std::vector<Record> _records;
	};

	// 测试插入记录并返回自增ID
	class TestAutoIncrement : public MysqlQueryTask {
	public:
		TestAutoIncrement() = default;
		virtual ~TestAutoIncrement() = default;

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual void onQueried() override;
	};

	// 测试更新记录
	class TestUpdate : public MysqlQueryTask {
	public:
		TestUpdate() = default;
		virtual ~TestUpdate() = default;

	public:
		virtual QueryType buildQuery(std::string& sql) override;
		virtual void onQueried() override;
	};

	class TestSelectWorker : public ThreadWorker {
	public:
		TestSelectWorker(const ThreadStopFlag::Ptr& flag);
		virtual ~TestSelectWorker() = default;

	protected:
		virtual int oneLoopEx() override;

	private:
		bool _first;
	};

	class TestPrintHandler : public RabbitmqMessageHandler {
	public:
		TestPrintHandler(const std::string& tag);
		virtual ~TestPrintHandler();

	protected:
		virtual void handleImpl(const std::string& message) override;
	};

	class TestPublisher {
	public:
		TestPublisher() = default;
		virtual ~TestPublisher() = default;

	public:
		void publish();

	private:
		int _count = 0;
	};
}

#endif // !_NIU_MA_EXAMPLE_H_