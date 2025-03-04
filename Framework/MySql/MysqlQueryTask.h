// MysqlQueryTask.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.31

#ifndef _NIU_MA_MYSQL_QUERY_TASK_H_
#define _NIU_MA_MYSQL_QUERY_TASK_H_

#include <memory>
#include <string>

namespace sql {
	// mysql���������
	class ResultSet;
}

namespace NiuMa {
	/**
	 * mysql��ѯ����ӿ�
	 * ���о���mysql��ѯ����ʵ�ָýӿ�
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

		// ��ѯ��������
		enum class QueryType {
			Select = 0,		// ��ѯ��¼
			Insert = 1,		// �����¼
			InsertAI = 2,	// �����¼����ȡ����ID(Auto Increment)
			Update = 3,		// ���¼�¼
			Delete = 4		// ɾ����¼
		};

	public:
		/**
		 * �����ѯ���(������ִ�е�SQL���)
		 * @param sql ����ʵ���๹��Ĳ�ѯ���
		 * @return ��������
		 */
		virtual QueryType buildQuery(std::string& sql) = 0;

		/**
		 * ��ȡ��ѯ���
		 * ����������ΪQueryType::Selectʱ�Ż���ø÷���
		 * @param res mysql�����
		 * @return ���ز�ѯ��������
		 */
		virtual int fetchResult(sql::ResultSet* res) { return 0; }

		/**
		 * ֪ͨ��ѯ�����Ѿ�ִ��
		 * ���ܲ�ѯ�����Ƿ�ִ�гɹ��������󶼻���ø÷���
		 * ע�⣡���첽��ѯ��ʱ�������������ǲ�ߣ�������ǲ���߳��е��ø÷���
		 * ���û�����ã�����ִ�����߳��е��ø÷���
		 */
		virtual void onQueried() {};

		/**
		 * ����ִ�гɹ���־ 
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
		 * �����²����¼������id
		 */
		void setAutoInc(const int64_t& ai) {
			_autoInc = ai;
		}

		const int64_t& getAutoInc() const {
			return _autoInc;
		}

	private:
		// ����ɹ�ִ�б�־
		bool _succeed;

		// ִ��select���ص�����
		int _rows;

		// ִ��insert��update��delete��Ӱ�������
		int _affectedRecords;

		// �²����¼������ID
		int64_t _autoInc;
	};

	/**
	 * mysqlͨ�ò�ѯ����
	 * ����в�ѯ�������̳и�����ж�ȡ
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
		// sql����
		const std::string _sql;

		// ��ѯ����
		const QueryType _type;
	};

	/**
	 * ��ѯ��¼��������
	 * ����select count(*) ...
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