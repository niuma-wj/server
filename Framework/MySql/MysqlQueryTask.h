// MysqlQueryTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.31

#ifndef _NIU_MA_MYSQL_QUERY_TASK_H_
#define _NIU_MA_MYSQL_QUERY_TASK_H_

#include <memory>
#include <string>

namespace sql {
	// mysql相关类声明
	class ResultSet;
}

namespace NiuMa {
	/**
	 * mysql查询任务接口
	 * 所有具体mysql查询任务都实现该接口
	 */
	class MysqlQueryTask : public std::enable_shared_from_this<MysqlQueryTask> {
	public:
		MysqlQueryTask()
			: _succeed(false)
			, _rows(0)
			, _affectedRecords(0)
			, _autoInc(0L)
		{}

		virtual ~MysqlQueryTask() = default;

		typedef std::shared_ptr<MysqlQueryTask> Ptr;

		// 查询任务类型
		enum class QueryType {
			Select = 0,		// 查询记录
			Insert = 1,		// 插入记录
			InsertAI = 2,	// 插入记录并获取自增ID(Auto Increment)
			Update = 3,		// 更新记录
			Delete = 4		// 删除记录
		};

	public:
		/**
		 * 构造查询语句(完整可执行的SQL语句)
		 * @param sql 返回实现类构造的查询语句
		 * @return 任务类型
		 */
		virtual QueryType buildQuery(std::string& sql) = 0;

		/**
		 * 获取查询结果
		 * 仅任务类型为QueryType::Select时才会调用该方法
		 * @param res mysql结果集
		 * @return 返回查询到的行数
		 */
		virtual int fetchResult(sql::ResultSet* res) { return 0; }

		/**
		 * 通知查询任务已经执行
		 * 不管查询任务是否执行成功，结束后都会调用该方法
		 * 注意！！异步查询的时候如果设置了派遣者，则在派遣者线程中调用该方法
		 * 如果没有设置，则在执行者线程中调用该方法
		 */
		virtual void onQueried() {};

		/**
		 * 设置执行成功标志 
		 */
		void setSucceed() {
			_succeed = true;
		}

		bool getSucceed() const {
			return _succeed;
		}

		void setRows(int s) {
			_rows = s;
		}

		int getRows() const {
			return _rows;
		}

		void setAffectedRecords(int s) {
			_affectedRecords = s;
		}

		int getAffectedRecords() const {
			return _affectedRecords;
		}

		/**
		 * 设置新插入记录的自增id
		 */
		void setAutoInc(const int64_t& ai) {
			_autoInc = ai;
		}

		const int64_t& getAutoInc() const {
			return _autoInc;
		}

	private:
		// 任务成功执行标志
		bool _succeed;

		// 执行select返回的行数
		int _rows;

		// 执行insert、update、delete受影响的行数
		int _affectedRecords;

		// 新插入记录的自增ID
		int64_t _autoInc;
	};

	/**
	 * mysql通用查询任务
	 * 如果有查询结果，则继承该类进行读取
	 */
	class MysqlCommonTask : public MysqlQueryTask
	{
	public:
		MysqlCommonTask(const std::string& sql, QueryType type)
			: _sql(sql)
			, _type(type)
		{}

		virtual ~MysqlCommonTask() {}

	public:
		virtual QueryType buildQuery(std::string& sql) override {
			sql = _sql;
			return _type;
		}

	private:
		// sql命令
		const std::string _sql;

		// 查询类型
		const QueryType _type;
	};

	/**
	 * 查询记录数量任务
	 * 即：select count(*) ...
	 */
	class MysqlCountTask : public MysqlCommonTask
	{
	public:
		MysqlCountTask(const std::string& sql);
		virtual ~MysqlCountTask();

	public:
		virtual int fetchResult(sql::ResultSet* res);

	public:
		int getCount() const;

	private:
		int _count;
	};
}

#endif // !_NIU_MA_MYSQL_QUERY_TASK_H_