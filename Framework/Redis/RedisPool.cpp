// RedisPool.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Timer/TimerManager.h"
#include "Database/DatabasePool.h"
#include "RedisPool.h"

#include "hiredis/include/hiredis.h"

#include <atomic>
#include <list>
#include <sstream>

#ifdef _WIN32
#include <winsock2.h> /* For struct timeval */
#else
#include <sys/time.h> /* for struct timeval */
#endif

namespace NiuMa {
	static bool isOk(redisReply* reply) {
		if (reply == nullptr)
			return false;
		if (REDIS_REPLY_STATUS != reply->type)
			return false;
		const std::string OK("OK");
		if (OK != reply->str)
			return false;
		return true;
	}

	class RedisConnection : public DatabaseConnection {
	public:
		RedisConnection(redisContext* ctx)
			: _ctx(ctx)
		{}

		virtual ~RedisConnection() {
			if (_ctx != nullptr) {
				redisFree(_ctx);
				_ctx = nullptr;
			}
		}

	public:
		// 获取redis连接
		redisContext* getContext() const {
			return _ctx;
		}

	private:
		// redis连接
		redisContext* _ctx;
	};

	class RedisPoolImpl : public DatabasePool {
	public:
		RedisPoolImpl(const std::string& host, int port,
			const std::string& password, int database,
			int keepConnections, int maxConnections)
			: DatabasePool(keepConnections, maxConnections)
			, _host(host)
			, _port(port)
			, _password(password)
			, _database(database)
		{}

		virtual ~RedisPoolImpl() {}

		typedef std::shared_ptr<RedisPoolImpl> Ptr;

	public:
		void clear() {
			clearConnections();
		}

	public:
		bool get(const std::string& key, std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "GET %s", key.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "Get (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "Get (key: " << key << ") error: " << reply->str;
			}
			else if (getValue(reply, value)) {
				ret = true;
			}
			else {
				ErrorS << "Get (key: " << key << ") error: value type is not supported.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool get(const std::string& key, int64_t& value) {
			std::string tmp;
			bool ret = get(key, tmp);
			if (ret)
				value = atoll(tmp.c_str());
			return ret;
		}

		bool set(const std::string& key, const std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = true;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "SET %s %s", key.c_str(), value.c_str()));
			if (reply == nullptr) {
				ErrorS << "Set (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "Set (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool set(const std::string& key, int64_t value) {
			std::string tmp = std::to_string(value);
			return set(key, tmp);
		}

		bool hget(const std::string& key, const std::string& field, std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HGET %s %s", key.c_str(), field.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "HGet (key: " << key << ", field: " << field << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "HGet (key: " << key << ", field: " << field << ") error: " << reply->str;
			}
			else if (getValue(reply, value)) {
				ret = true;
			}
			else {
				ErrorS << "HGet (key: " << key << ", field: " << field << ") error: value type is not supported.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool hget(const std::string& key, const std::string& field, int64_t& value) {
			std::string tmp;
			bool ret = hget(key, field, tmp);
			if (ret)
				value = atoll(tmp.c_str());
			return ret;
		}

		bool hset(const std::string& key, const std::string& field, const std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = true;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HSET %s %s %s", key.c_str(), field.c_str(), value.c_str()));
			if (reply == nullptr) {
				ErrorS << "HSet (key: " << key << ", field: " << field << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "HSet (key: " << key << ", field: " << field << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool hset(const std::string& key, const std::string& field, int64_t value) {
			std::string tmp = std::to_string(value);
			return hset(key, field, tmp);
		}

		bool hexists(const std::string& key, const std::string& field, bool& result) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			std::string tmp;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HEXISTS %s %s", key.c_str(), field.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "HEXISTS (key: " << key << ", field: " << field << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "HEXISTS (key: " << key << ", field: " << field << ") error: " << reply->str;
			}
			else if (getValue(reply, tmp)) {
				ret = true;
				if (tmp == "1")
					result = true;
				else
					result = false;
			}
			else {
				ErrorS << "HEXISTS (key: " << key << ", field: " << field << ") error: value type is not supported.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool hkeys(const std::string& key, std::vector<std::string>& fields) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			std::string tmp;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HKEYS %s", key.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "HKEYS (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "HSet (key: " << key << ") error: " << reply->str;
			}
			else if ((REDIS_REPLY_ARRAY == reply->type) || (REDIS_REPLY_SET == reply->type)) {
				ret = true;
				fields.clear();
				for (size_t i = 0; i < reply->elements; i++) {
					if (getValue(reply->element[i], tmp))
						fields.push_back(tmp);
					else {
						DebugS << "Get fields (key: " << key << ") error: value type of element " << i << " is not supported.";
					}
				}
			}
			else if (getValue(reply, tmp)) {
				ret = true;
				fields.push_back(tmp);
			}
			else {
				ErrorS << "Get fields (key: " << key << ") error: value type is not hash.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool del(const std::string& key) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "DEL %s", key.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "DEL (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "DEL (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool hdel(const std::string& key, const std::string& field) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HDEL %s %s", key.c_str(), field.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "HDEL (key: " << key << ", field: " << field << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "HDEL (key: " << key << ", field: " << field << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool sadd(const std::string& key, const std::vector<std::string>& values) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			std::stringstream ss;
			ss << "SADD " << key;
			for (const std::string& value : values)
				ss << " " << value;
			std::string cmd = ss.str();
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), cmd.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "SADD (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "SADD (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool sremove(const std::string& key, const std::vector<std::string>& values) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			std::stringstream ss;
			ss << "SREM " << key;
			for (const std::string& value : values)
				ss << " " << value;
			std::string cmd = ss.str();
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), cmd.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "SREM (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "SREM (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool sismember(const std::string& key, const std::string& value, bool& result) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			std::string tmp;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "SISMEMBER %s %s", key.c_str(), value.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "SISMEMBER (key: " << key << ", value: " << value << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "SISMEMBER (key: " << key << ", value: " << value << ") error: " << reply->str;
			}
			else if (getValue(reply, tmp)) {
				ret = true;
				if (tmp == "1")
					result = true;
				else
					result = false;
			}
			else {
				ErrorS << "SISMEMBER (key: " << key << ", value: " << value << ") error: value type is not supported.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool smembers(const std::string& key, std::vector<std::string>& values) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			std::string tmp;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "SMEMBERS %s", key.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "SMEMBERS (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "SMEMBERS (key: " << key << ") error: " << reply->str;
			}
			else if ((REDIS_REPLY_ARRAY == reply->type) || (REDIS_REPLY_SET == reply->type)) {
				ret = true;
				values.clear();
				for (size_t i = 0; i < reply->elements; i++) {
					if (getValue(reply->element[i], tmp))
						values.push_back(tmp);
					else {
						DebugS << "Get members (key: " << key << ") error: value type of element " << i << " is not supported.";
					}
				}
			}
			else if (getValue(reply, tmp)) {
				ret = true;
				values.push_back(tmp);
			}
			else {
				ErrorS << "Get members (key: " << key << ") error: value type is not hash.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool expire(const std::string& key, int sec) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "EXPIRE %s %d", key.c_str(), sec));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "EXPIRE (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "EXPIRE (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool expireAt(const std::string& key, int64_t timeStamp) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "EXPIREAT %s %lld", key.c_str(), timeStamp));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "EXPIREAT (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "EXPIREAT (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool incrBy(const std::string& key, int delta) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "INCRBY %s %d", key.c_str(), delta));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "INCRBY (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "INCRBY (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool hincrBy(const std::string& key, const std::string& field, int delta) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "HINCRBY %s %s %d", key.c_str(), field.c_str(), delta));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "HINCRBY (key: " << key << ", field: " << field << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "HINCRBY (key: " << key << ", field: " << field << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool rpush(const std::string& key, const std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "RPUSH %s %s", key.c_str(), value.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "RPUSH (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "RPUSH (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool rpush(const std::string& key, int64_t value) {
			std::string tmp = std::to_string(value);
			return rpush(key, tmp);
		}

		bool rpushv(const std::string& key, const std::vector<std::string>& arr) {
			if (arr.empty())
				return true;
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			size_t num = arr.size() + 2;
			char** argv = new char* [num];
			size_t* argvlen = new size_t [num];
			argv[0] = BaseUtils::strdup("RPUSH");
			argv[1] = BaseUtils::strdup(key.c_str());
			argvlen[0] = 5;
			argvlen[1] = key.length();
			for (size_t i = 0; i < arr.size(); i++) {
				argv[i + 2] = BaseUtils::strdup(arr[i].c_str());
				argvlen[i + 2] = arr[i].length();
			}
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommandArgv(con->getContext(), static_cast<int>(num), (const char**)argv, argvlen));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "RPUSHV (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "RPUSHV (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			for (size_t i = 0; i < num; i++)
				free(argv[i]);
			delete[] argv;
			delete[] argvlen;
			return ret;
		}

		bool rpushv(const std::string& key, const std::vector<int64_t>& arr) {
			if (arr.empty())
				return true;
			std::vector<std::string> tmp;
			for (const int64_t& val : arr)
				tmp.push_back(std::to_string(val));
			return rpushv(key, tmp);
		}

		bool lpush(const std::string& key, const std::string& value) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "LPUSH %s %s", key.c_str(), value.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "LPUSH (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "LPUSH (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

		bool lpush(const std::string& key, int64_t value) {
			std::string tmp = std::to_string(value);
			return lpush(key, tmp);
		}

		bool lpushv(const std::string& key, const std::vector<std::string>& arr) {
			if (arr.empty())
				return true;
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			size_t num = arr.size() + 2;
			char** argv = new char* [num];
			size_t* argvlen = new size_t[num];
			argv[0] = BaseUtils::strdup("LPUSH");
			argv[1] = BaseUtils::strdup(key.c_str());
			argvlen[0] = 5;
			argvlen[1] = key.length();
			for (size_t i = 0; i < arr.size(); i++) {
				argv[i + 2] = BaseUtils::strdup(arr[i].c_str());
				argvlen[i + 2] = arr[i].length();
			}
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommandArgv(con->getContext(), static_cast<int>(num), (const char**)argv, argvlen));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "LPUSHV (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "LPUSHV (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			for (size_t i = 0; i < num; i++)
				free(argv[i]);
			delete[] argv;
			delete[] argvlen;
			return ret;
		}

		bool lpushv(const std::string& key, const std::vector<int64_t>& arr) {
			if (arr.empty())
				return true;
			std::vector<std::string> tmp;
			for (const int64_t& val : arr)
				tmp.push_back(std::to_string(val));
			return lpushv(key, tmp);
		}

		bool lrange(const std::string& key, std::vector<std::string>& arr) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			bool ret = false;
			bool flag = false;
			std::string tmp;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "LRANGE %s 0 -1", key.c_str()));
			if (reply == nullptr) {
				flag = true;
				ErrorS << "Get array (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				flag = true;
				ErrorS << "Get array (key: " << key << ") error: " << reply->str;
			}
			else if (REDIS_REPLY_ARRAY == reply->type) {
				ret = true;
				arr.clear();
				for (size_t i = 0; i < reply->elements; i++) {
					if (getValue(reply->element[i], tmp))
						arr.push_back(tmp);
					else {
						DebugS << "Get array (key: " << key << ") error: value type of element " << i << " is not supported.";
					}
				}
			}
			else if (getValue(reply, tmp)) {
				ret = true;
				arr.push_back(tmp);
			}
			else {
				ErrorS << "Get array (key: " << key << ") error: value type is not array.";
			}
			checkConnection(con, flag);
			return ret;
		}

		bool lrange(const std::string& key, std::vector<int64_t>& arr) {
			std::vector<std::string> tmp;
			bool ret = lrange(key, tmp);
			if (ret) {
				arr.clear();
				for (const std::string& val : tmp)
					arr.push_back(atoll(val.c_str()));
			}
			return ret;
		}

		bool persist(const std::string& key) {
			std::shared_ptr<RedisConnection> con = getConnectionEx();
			if (!con)
				return false;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "PERSIST %s", key.c_str()));
			bool ret = false;
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "PERSIST (key: " << key << ") error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "PERSIST (key: " << key << ") error: " << reply->str;
			}
			else {
				ret = true;
				flag = false;
			}
			checkConnection(con, flag);
			return ret;
		}

	protected:
		virtual DatabaseConnection::Ptr createConnection() override {
			DatabaseConnection::Ptr con;
			struct timeval timeout = { 3, 0 }; // 3 seconds
			redisContext* ctx = redisConnectWithTimeout(_host.c_str(), _port, timeout);
			if (ctx == nullptr) {
				LOG_ERROR("Create connection error: can't allocate redis context.");
				return con;
			}
			if (ctx->err != 0) {
				ErrorS << "Create connection error: " << ctx->errstr;
				redisFree(ctx);
				return con;
			}
			bool test = false;
			redisReply* reply = nullptr;
			if (!_password.empty()) {
				reply = reinterpret_cast<redisReply*>(redisCommand(ctx, "AUTH %s", _password.c_str()));
				if (reply == nullptr) {
					test = true;
					LOG_ERROR("Authorization failed, unknown error.");
				}
				else if (REDIS_REPLY_ERROR == reply->type) {
					test = true;
					ErrorS << "Authorization failed: " << reply->str;
				}
			}
			if (!test) {
				reply = reinterpret_cast<redisReply*>(redisCommand(ctx, "SELECT %d", _database));
				if (reply == nullptr) {
					test = true;
					ErrorS << "Select database " << _database << " failed, unknown error.";
				}
				else if (REDIS_REPLY_ERROR == reply->type) {
					test = true;
					ErrorS << "Select database " << _database << " failed: " << reply->str;
				}
			}
			if (test)
				redisFree(ctx);
			else
				con = std::make_shared<RedisConnection>(ctx);
			return con;
		}

		virtual void onTimerImpl(const DatabaseConnection::Ptr& conIn) override {
			std::shared_ptr<RedisConnection> con = std::dynamic_pointer_cast<RedisConnection>(conIn);
			if (!con)
				return;
			redisReply* reply = reinterpret_cast<redisReply*>(redisCommand(con->getContext(), "PING"));
			bool flag = true;
			if (reply == nullptr) {
				ErrorS << "PING server error: unknown error.";
			}
			else if (REDIS_REPLY_ERROR == reply->type) {
				ErrorS << "PING server error: " << reply->str;
			}
			else
				flag = false;
			checkConnection(con, flag);
		}

	private:
		std::shared_ptr<RedisConnection> getConnectionEx() {
			DatabaseConnection::Ptr tmp = getConnection();
			if (!tmp)
				return nullptr;
			return std::dynamic_pointer_cast<RedisConnection>(tmp);
		}

		void checkConnection(const std::shared_ptr<RedisConnection>& con, bool flag) {
			bool disconnected = false;
			if (flag) {
				redisContext* ctx = con->getContext();
				if (REDIS_ERR_IO == ctx->err || REDIS_ERR_EOF == ctx->err || REDIS_ERR_TIMEOUT == ctx->err) {
					disconnected = true;
					ErrorS << "Connection error: " << ctx->errstr;
				}
			}
			if (disconnected)
				removeConnection(con);
			else {
				con->setQueryTime();
				con->recycle();
				notifyOne();
			}
		}

		bool getValue(redisReply* reply, std::string& value) {
			bool ret = true;
			if (REDIS_REPLY_STRING == reply->type)
				value = reply->str;
			else if (REDIS_REPLY_INTEGER == reply->type)
				value = std::to_string(reply->integer);
			else
				ret = false;
			return ret;
		}

	private:
		// 服务器主机地址
		const std::string _host;

		// 端口号
		const int _port;

		// 密码
		const std::string _password;

		// 数据库号
		const int _database;
	};

	template<> RedisPool* Singleton<RedisPool>::_inst = nullptr;

	void RedisPool::start(const std::string& host, int port, const std::string& password, int database, int keepConnections, int maxConnections) {
		if (_impl)
			return;
		_impl = std::make_shared<RedisPoolImpl>(host, port, password, database, keepConnections, maxConnections);

		std::weak_ptr<RedisPoolImpl> weakImpl = _impl;
		TimerManager::getSingleton().addAsyncTimer(2000, [weakImpl]() {
			RedisPoolImpl::Ptr impl = weakImpl.lock();
			if (impl)
				return impl->onTimer();
			return true;
		});
		InfoS << "Redis pool started, host: " << host << ", port: " << port << ", database: " << database
			<< ", keep connections: " << keepConnections << ", max connections: " << maxConnections;
	}

	void RedisPool::stop() {
		if (!_impl)
			return;
		_impl->stop();
		_impl->clear();
		_impl.reset();
	}

	bool RedisPool::get(const std::string& key, std::string& value) {
		return _impl->get(key, value);
	}

	bool RedisPool::get(const std::string& key, int64_t& value) {
		return _impl->get(key, value);
	}

	bool RedisPool::set(const std::string& key, const std::string& value) {
		return _impl->set(key, value);
	}

	bool RedisPool::set(const std::string& key, int64_t value) {
		return _impl->set(key, value);
	}

	bool RedisPool::hget(const std::string& key, const std::string& field, std::string& value) {
		return _impl->hget(key, field, value);
	}

	bool RedisPool::hget(const std::string& key, const std::string& field, int64_t& value) {
		return _impl->hget(key, field, value);
	}

	bool RedisPool::hset(const std::string& key, const std::string& field, const std::string& value) {
		return _impl->hset(key, field, value);
	}

	bool RedisPool::hset(const std::string& key, const std::string& field, int64_t value) {
		return _impl->hset(key, field, value);
	}

	bool RedisPool::hexists(const std::string& key, const std::string& field, bool& result) {
		return _impl->hexists(key, field, result);
	}

	bool RedisPool::hkeys(const std::string& key, std::vector<std::string>& fields) {
		return _impl->hkeys(key, fields);
	}

	bool RedisPool::del(const std::string& key) {
		return _impl->del(key);
	}

	bool RedisPool::hdel(const std::string& key, const std::string& field) {
		return _impl->hdel(key, field);
	}

	bool RedisPool::sadd(const std::string& key, const std::vector<std::string>& values) {
		return _impl->sadd(key, values);
	}

	bool RedisPool::sadd(const std::string& key, const std::string& value) {
		std::vector<std::string> arr;
		arr.push_back(value);
		return _impl->sadd(key, arr);
	}

	bool RedisPool::sremove(const std::string& key, const std::vector<std::string>& values) {
		return _impl->sremove(key, values);
	}

	bool RedisPool::sremove(const std::string& key, const std::string& value) {
		std::vector<std::string> arr;
		arr.push_back(value);
		return _impl->sremove(key, arr);
	}

	bool RedisPool::sismember(const std::string& key, const std::string& value, bool& result) {
		return _impl->sismember(key, value, result);
	}

	bool RedisPool::smembers(const std::string& key, std::vector<std::string>& values) {
		return _impl->smembers(key, values);
	}

	bool RedisPool::expire(const std::string& key, int sec) {
		return _impl->expire(key, sec);
	}

	bool RedisPool::expireAt(const std::string& key, int64_t timeStamp) {
		return _impl->expireAt(key, timeStamp);
	}

	bool RedisPool::incrBy(const std::string& key, int delta) {
		return _impl->incrBy(key, delta);
	}

	bool RedisPool::hincrBy(const std::string& key, const std::string& field, int delta) {
		return _impl->hincrBy(key, field, delta);
	}

	bool RedisPool::persist(const std::string& key) {
		return _impl->persist(key);
	}

	bool RedisPool::rpush(const std::string& key, const std::string& value) {
		return _impl->rpush(key, value);
	}

	bool RedisPool::rpush(const std::string& key, int64_t value) {
		return _impl->rpush(key, value);
	}

	bool RedisPool::rpushv(const std::string& key, const std::vector<std::string>& arr) {
		return _impl->rpushv(key, arr);
	}

	bool RedisPool::rpushv(const std::string& key, const std::vector<int64_t>& arr) {
		return _impl->rpushv(key, arr);
	}

	bool RedisPool::lpush(const std::string& key, const std::string& value) {
		return _impl->lpush(key, value);
	}

	bool RedisPool::lpush(const std::string& key, int64_t value) {
		return _impl->lpush(key, value);
	}

	bool RedisPool::lpushv(const std::string& key, const std::vector<std::string>& arr) {
		return _impl->lpushv(key, arr);
	}

	bool RedisPool::lpushv(const std::string& key, const std::vector<int64_t>& arr) {
		return _impl->lpushv(key, arr);
	}

	bool RedisPool::lrange(const std::string& key, std::vector<std::string>& arr) {
		return _impl->lrange(key, arr);
	}

	bool RedisPool::lrange(const std::string& key, std::vector<int64_t>& arr) {
		return _impl->lrange(key, arr);
	}
}
