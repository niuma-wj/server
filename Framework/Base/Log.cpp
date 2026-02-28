// Log.cpp

#include "Log.h"
#include "Base/BaseUtils.h"

#include <boost/log/trivial.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread/thread.hpp>
// BOOST 1.86及之后的版本不要包含process头文件，以免链接失败
#if (BOOST_VERSION < 108600)
#include <boost/process.hpp>
#endif

namespace NiuMa {
	/**
	 * 日志
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.11
	 */
	class Logger
	{
	public:
		Logger(const std::string& fileName) {
			namespace expr = boost::log::expressions;
			namespace keywords = boost::log::keywords;
			namespace attr = boost::log::attributes;

			boost::log::add_common_attributes();
			
			std::string logName = fileName + "_%Y-%m-%d_%N.log";

			// Create a backend and initialize it with a stream
			boost::shared_ptr<boost::log::sinks::text_file_backend> backend_file =
				boost::make_shared<boost::log::sinks::text_file_backend>(
					keywords::file_name = logName,
					keywords::open_mode = std::ios_base::out | std::ios_base::app,
					keywords::rotation_size = 30 * 1024 * 1024, // 超过30M滚动Log
					keywords::time_based_rotation = boost::log::sinks::file::rotation_at_time_point(0, 0, 0));
			// Enable auto-flushing after each log record written
			backend_file->auto_flush(true);

			// Create a backend and attach a couple of streams to it
			boost::shared_ptr<boost::log::sinks::text_ostream_backend> backend_print =
				boost::make_shared<boost::log::sinks::text_ostream_backend>();
			backend_print->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

			// Enable auto-flushing after each log record written
			backend_print->auto_flush(true);

			// Wrap it into the frontend and register in the core
			_sink_file = boost::make_shared<sink_file>(backend_file);
			_sink_print = boost::make_shared<sink_print>(backend_print);

			// You can manage filtering and formatting through the sink interface
			boost::log::formatter fmt = expr::stream
				<< "[" << expr::attr<attr::current_thread_id::value_type>("ThreadID")
				<< "] " << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%m-%dT%H:%M:%S.%f")
				<< " " << boost::log::trivial::severity
				//<< " " << expr::attr<attr::current_process_id::value_type>("ProcessID")
				//<< "." << expr::attr<attr::current_thread_id::value_type>("ThreadID")
				<< " " << expr::message;
			_sink_file->set_formatter(fmt);
			_sink_print->set_formatter(fmt);

			// add sink
			boost::shared_ptr<boost::log::core> core = boost::log::core::get();
			core->set_filter(boost::log::trivial::severity >= boost::log::trivial::trace);
			core->add_sink(_sink_file);
			core->add_sink(_sink_print);
		}

		virtual ~Logger()
		{
			boost::shared_ptr<boost::log::core> core = boost::log::core::get();

			if (_sink_print) {
				// Remove the sink from the core, so that no records are passed to it
				core->remove_sink(_sink_print);

				// Break the feeding loop
				_sink_print->stop();

				// Flush all log records that may have left buffered
				_sink_print->flush();

				_sink_print.reset();
			}
			if (_sink_file) {
				// Remove the sink from the core, so that no records are passed to it
				core->remove_sink(_sink_file);

				// Break the feeding loop
				_sink_file->stop();

				// Flush all log records that may have left buffered
				_sink_file->flush();

				_sink_file.reset();
			}
		}

	private:
		typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_file_backend> sink_file;
		typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend> sink_print;
		boost::shared_ptr<sink_file> _sink_file;
		boost::shared_ptr<sink_print> _sink_print;

	public:
		boost::log::sources::severity_logger<boost::log::trivial::severity_level> logger;
	};

	template<> LogManager* Singleton<LogManager>::_inst = nullptr;

	LogManager::LogManager()
		: _initialized(false)
	{}

	LogManager::~LogManager() {}

	void LogManager::initialize(const std::string& fileName) {
		if (_initialized)
			return;
		_initialized = true;
		if (fileName.empty())
			throw std::runtime_error("Specified log file name is empty.");
		std::string tmpName;
		if (BaseUtils::endWith(fileName, ".log"))
			tmpName = fileName.substr(0, fileName.length() - 4);
		else
			tmpName = fileName;
		_logger = std::make_shared<Logger>(tmpName);
	}

	void LogManager::stop() {
		if (!_initialized)
			return;
		_initialized = false;

		_logger.reset();
	}

	void LogManager::logDebug(const std::string& msg, const std::string& file, int line) {
//#if defined(DEBUG) || defined(_DEBUG)
		if (!_initialized)
			return;
		std::string text;
		NiuMa::BaseUtils::fileName(file, text);
		text = text + ":" + std::to_string(line);
		text = text + " " + msg;
		BOOST_LOG_SEV(_logger->logger, boost::log::trivial::debug) << text;
//#endif
	}

	void LogManager::logInfo(const std::string& msg, const std::string& file, int line) {
		if (!_initialized)
			return;
		std::string text;
		NiuMa::BaseUtils::fileName(file, text);
		text = text + ":" + std::to_string(line);
		text = text + " " + msg;
		BOOST_LOG_SEV(_logger->logger, boost::log::trivial::info) << text;
	}

	void LogManager::logWarning(const std::string& msg, const std::string& file, int line) {
		if (!_initialized)
			return;
		std::string text;
		NiuMa::BaseUtils::fileName(file, text);
		text = text + ":" + std::to_string(line);
		text = text + " " + msg;
		BOOST_LOG_SEV(_logger->logger, boost::log::trivial::warning) << text;
	}

	void LogManager::logError(const std::string& msg, const std::string& file, int line) {
		if (!_initialized)
			return;
		std::string text;
		NiuMa::BaseUtils::fileName(file, text);
		text = text + ":" + std::to_string(line);
		text = text + " " + msg;
		BOOST_LOG_SEV(_logger->logger, boost::log::trivial::error) << text;
	}

	LogStream::LogStream(const std::string& file, int line, int level)
		: _file(file)
		, _line(line)
		, _level(level)
	{}

	LogStream::~LogStream() {
		flush();
	}

	LogStream& LogStream::operator<<(std::ostream& (*f)(std::ostream&)) {
		flush();
		return *this;
	}

	void LogStream::clear() {
		_oss.str("");
	}

	void LogStream::flush() {
		std::string msg = _oss.str();
		if (msg.empty())
			return;
		if (LogLevel::Debug == _level)
			NiuMa::LogManager::getSingleton().logDebug(msg, _file, _line);
		else if (LogLevel::Info == _level)
			NiuMa::LogManager::getSingleton().logInfo(msg, _file, _line);
		else if (LogLevel::Warning == _level)
			NiuMa::LogManager::getSingleton().logWarning(msg, _file, _line);
		else if (LogLevel::Error == _level)
			NiuMa::LogManager::getSingleton().logError(msg, _file, _line);

		// 清空已输出日志
		_oss.str("");
	}
}
