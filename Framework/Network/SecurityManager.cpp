// SecurityManager.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Constant/RedisKeys.h"
#include "Redis/RedisPool.h"
#include "Rabbitmq/RabbitmqClient.h"
#include "Rabbitmq/RabbitmqConsumer.h"
#include "Rabbitmq/RabbitmqMessageJsonHandler.h"
#include "SecurityManager.h"
#include "jsoncpp/include/json/json.h"

namespace NiuMa {
	template<> SecurityManager* Singleton<SecurityManager>::_inst = nullptr;

	class AbnormalRecord {
	public:
		AbnormalRecord() {}
		virtual ~AbnormalRecord() {}

	public:
		/**
		 * 记录一次异常行为
		 * @return 是否超出阈值(即3秒内20次异常)
		 */
		bool abnormalBehavior(const time_t& nowTime) {
			std::lock_guard<std::mutex> lck(_mtx);

			time_t delta = 0;
			while (!_timestamps.empty()) {
				const time_t& t = _timestamps.front();
				delta = nowTime - t;
				if (delta > 3)
					_timestamps.pop_front();
				else
					break;
			}
			_timestamps.emplace_back(nowTime);
			return (_timestamps.size() > 19);
		}

	private:
		// 记录异常行为的时间戳列表
		std::list<time_t> _timestamps;

		// 信号量
		std::mutex _mtx;
	};

	SecurityManager::SecurityManager()
		: _initFlag(false)
	{}

	SecurityManager::~SecurityManager() {}

	void SecurityManager::init(const std::string& fanoutExchange, const std::string& consumerTag) {
		if (_initFlag)
			return;
		_initFlag = true;
		_fanoutExchange = fanoutExchange;
		_consumerTag = consumerTag;

		time_t delta = 0L;
		time_t timestamp = 0L;
		time_t nowTime = BaseUtils::getCurrentSecond();
		std::vector<std::string> fields;
		RedisPool::getSingleton().hkeys(RedisKeys::IP_BLACKLIST, fields);
		for (const std::string& ip : fields) {
			if (!RedisPool::getSingleton().hget(RedisKeys::IP_BLACKLIST, ip, timestamp))
				continue;
			delta = nowTime - timestamp;
			if (delta > 300) {
				// 超过5分钟
				RedisPool::getSingleton().hdel(RedisKeys::IP_BLACKLIST, ip);
				continue;
			}
			// 加入本地黑名单
			add2Blacklist(ip, timestamp);
		}
		// 创建RabbitMQ消息处理器
		class BlacklistHandler : public RabbitmqMessageJsonHandler {
		public:
			BlacklistHandler(const std::string& tag)
				: RabbitmqMessageJsonHandler(tag)
			{}

			virtual ~BlacklistHandler() {}

		protected:
			virtual bool receive(const std::string& message) override {
				std::string::size_type pos = message.find("MsgIpBlacklist");
				return (pos != std::string::npos);
			}

			virtual void handleImpl(const std::string& msgType, const std::string& json) override {
				SecurityManager::getSingleton().handleMessage(msgType, json);
			}
		};
		RabbitmqMessageHandler::Ptr handler(new BlacklistHandler(consumerTag));
		RabbitmqConsumer::getSingleton().addHandler(handler);
	}

	void SecurityManager::abnormalBehavior(const std::string& remoteIp) {
		std::shared_ptr<AbnormalRecord> record = getRecord(remoteIp, true);
		time_t nowTime = BaseUtils::getCurrentSecond();
		if (record->abnormalBehavior(nowTime)) {
			WarningS << "Add remote ip: " << remoteIp << " to blacklist.";
			// 超出阈值，将远端ip加入黑名单
			add2Blacklist(remoteIp, nowTime);
			// 向Redis中添加黑名单
			RedisPool::getSingleton().hset(RedisKeys::IP_BLACKLIST, remoteIp, nowTime);

			if (_fanoutExchange.empty())
				return;
			// 发送RabbitMQ广播，通知其他服务器该ip被列入黑名单
			// 打包广播消息
			Json::Value tmp(Json::objectValue);
			tmp["remoteIp"] = remoteIp;
			tmp["timestamp"] = static_cast<Json::Int64>(nowTime);
			// 发布广播消息
			RabbitmqClient::getSingleton().publishJson(_fanoutExchange, std::string(), "MsgIpBlacklistAdd", tmp.toStyledString());
		}
	}

	std::shared_ptr<AbnormalRecord> SecurityManager::getRecord(const std::string& remoteIp, bool createIfNotExist) {
		std::shared_ptr<AbnormalRecord> ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::shared_ptr<AbnormalRecord> >::const_iterator it = _abnormalRecords.find(remoteIp);
		if (it != _abnormalRecords.end())
			ret = it->second;
		else if (createIfNotExist) {
			ret = std::make_shared<AbnormalRecord>();
			_abnormalRecords.insert(std::make_pair(remoteIp, ret));
		}
		return ret;
	}

	bool SecurityManager::checkBlacklist(const std::string& remoteIp) {
		time_t timestamp = 0L;
		if (!getTimestamp(remoteIp, timestamp))
			return false;
		time_t nowTime = BaseUtils::getCurrentSecond();
		time_t delta = nowTime - timestamp;
		if (delta < 300)
			return true;
		// 超过5分钟
		RedisPool::getSingleton().hdel(RedisKeys::IP_BLACKLIST, remoteIp);
		removeFromBlacklist(remoteIp);
		// 发送RabbitMQ广播通知
		// 打包广播消息
		Json::Value tmp(Json::objectValue);
		tmp["remoteIp"] = remoteIp;
		// 发布广播消息
		RabbitmqClient::getSingleton().publishJson(_fanoutExchange, std::string(), "MsgIpBlacklistRemove", tmp.toStyledString());
		return false;
	}

	void SecurityManager::add2Blacklist(const std::string& remoteIp, const time_t& timestamp) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, time_t>::iterator it = _blacklist.find(remoteIp);
		if (it != _blacklist.end())
			it->second = timestamp;
		else
			_blacklist.insert(std::make_pair(remoteIp, timestamp));
	}

	void SecurityManager::removeFromBlacklist(const std::string& remoteIp) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, time_t>::const_iterator it = _blacklist.find(remoteIp);
		if (it != _blacklist.end())
			_blacklist.erase(it);
	}

	bool SecurityManager::getTimestamp(const std::string& remoteIp, time_t& timestamp) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, time_t>::const_iterator it = _blacklist.find(remoteIp);
		if (it != _blacklist.end()) {
			timestamp = it->second;
			return true;
		}
		return false;
	}

	void SecurityManager::handleMessage(const std::string& msgType, const std::string& json) {
		int test = 0;
		if (msgType == "MsgIpBlacklistAdd")
			test = 1;
		else if (msgType == "MsgIpBlacklistRemove")
			test = 2;
		if (test == 0)
			return;
		std::stringstream ss(json);
		Json::Value obj;
		ss >> obj;
		Json::Value& fieldIp = obj["remoteIp"];
		if (!fieldIp.isString())
			return;
		std::string remoteIp = fieldIp.asString();
		if (test == 1) {
			// 添加ip到黑名单
			Json::Value& fieldTimestamp = obj["timestamp"];
			if (!fieldTimestamp.isIntegral())
				return;
			time_t timestamp = fieldTimestamp.asInt64();
			add2Blacklist(remoteIp, timestamp);
		}
		else {
			// 从黑名单中删除ip
			removeFromBlacklist(remoteIp);
		}
	}
}