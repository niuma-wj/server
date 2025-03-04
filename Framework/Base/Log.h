// LogManager.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.07.11

#ifndef _NIU_MA_LOG_H_
#define _NIU_MA_LOG_H_

#include "Base/Singleton.h"

#include <memory>
#include <atomic>
#include <string>
#include <sstream>

namespace NiuMa {
	namespace LogLevel {
		// 日志等级
		enum {
			Debug,
			Info,
			Warning,
			Error
		};
	}

	/**
	 * 日志管理者
	 */
	class Logger;
	class LogManager : public Singleton<LogManager> {
	private:
		LogManager();

		friend class Singleton<LogManager>;

	public:
		virtual ~LogManager();

	public:
		// 初始化
		void initialize(const std::string& logFile);

		// 关闭
		void stop();

		// 调试日志
		void logDebug(const std::string& msg, const std::string& file, int line);

		// 常规日志
		void logInfo(const std::string& msg, const std::string& file, int line);

		// 警告日志
		void logWarning(const std::string& msg, const std::string& file, int line);

		// 错误日志
		void logError(const std::string& msg, const std::string& file, int line);

	private:
		// 
		std::shared_ptr<Logger> _logger;

		// 
		std::atomic<bool> _initialized;
	};

	class LogStream
	{
	public:
		LogStream(const std::string& file, int line, int level);
		virtual ~LogStream();

		/**
		 * 输入std::endl(回车符)立即输出日志
		 * @param f std::endl(回车符)
		 * @return 自身引用
		 */
		LogStream& operator<<(std::ostream& (*f)(std::ostream&));

		template<typename T>
		LogStream& operator<<(T&& data) {
			_oss << std::forward<T>(data);
			return *this;
		}

		void clear();

	private:
		void flush();

	private:
		//
		std::ostringstream _oss;

		// 源文件
		const std::string _file;

		// 行号
		const int _line;

		// 等级
		const int _level;
	};
}

// 输入日志
#define LOG_DEBUG(msg)		NiuMa::LogManager::getSingleton().logDebug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg)		NiuMa::LogManager::getSingleton().logInfo(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg)	NiuMa::LogManager::getSingleton().logWarning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg)		NiuMa::LogManager::getSingleton().logError(msg, __FILE__, __LINE__)

// 流形式输出日志，如：DebugS << 1 << "+" << 2 << '=' << 3;
#define DebugS		NiuMa::LogStream(__FILE__, __LINE__, NiuMa::LogLevel::Debug)
#define InfoS		NiuMa::LogStream(__FILE__, __LINE__, NiuMa::LogLevel::Info)
#define WarningS	NiuMa::LogStream(__FILE__, __LINE__, NiuMa::LogLevel::Warning)
#define ErrorS		NiuMa::LogStream(__FILE__, __LINE__, NiuMa::LogLevel::Error)

#endif // !_NIU_MA_LOG_MANAGER_H_
