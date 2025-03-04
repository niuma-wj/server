// DatabaseConnection.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.06

#ifndef _NIU_MA_DATA_BASE_CONNECTION_H_
#define _NIU_MA_DATA_BASE_CONNECTION_H_

#include <memory>
#include <mutex>

namespace NiuMa {
	// 数据库连接类
	class DatabaseConnection : public std::enable_shared_from_this<DatabaseConnection> {
	public:
		DatabaseConnection();
		virtual ~DatabaseConnection() = default;

		typedef std::shared_ptr<DatabaseConnection> Ptr;

	public:
		/**
		 * 占用
		 * @param flag true-修改最近占用时间，false-不修改最近占用时间
		 */
		void occupy(bool flag = true);

		// 回收
		void recycle();

		// 返回占用标志
		bool isOccupied();

		// 设置最近查询时间
		void setQueryTime();

		// 获取最近时间
		void getLastTime(time_t& occupyTime, time_t& queryTime);

	protected:
		// 占用标志
		bool _occupied;

		// 最近占用时间
		time_t _lastOccupiedTime;

		// 最近一次查询数据库的时间
		time_t _lastQueryTime;

		//
		std::mutex _mtx;
	};
}

#endif // !_NIU_MA_DATA_BASE_CONNECTION_H_