// VenueManager.cpp

#include "Base/Log.h"
#include "VenueManager.h"
#include "Constant/RedisKeys.h"
#include "Constant/ErrorDefines.h"
#include "Redis/RedisPool.h"
#include "Rabbitmq/RabbitmqClient.h"
#include "Rabbitmq/RabbitmqConsumer.h"
#include "Rabbitmq/RabbitmqMessageJsonHandler.h"
#include "Player/PlayerManager.h"
#include "Timer/TimerManager.h"
#include "Game/GameDumb.h"
#include "Game/GameMessages.h"
#include "MySql/MysqlPool.h"
#include <mysql/jdbc.h>
#include "jsoncpp/include/json/json.h"

#include <boost/locale.hpp>

namespace NiuMa {
	template<> VenueManager* Singleton<VenueManager>::_inst = nullptr;

	VenueManager::VenueManager() {}

	VenueManager::~VenueManager() {}

	void VenueManager::init(const std::string& serverId, const std::string& directExchange, const std::string& consumerTag) {
		_serverId = serverId;
		_directExchange = directExchange;

		// 注册空游戏加载器
		VenueLoader::Ptr loader = std::make_shared<GameDumbLoader>();
		registLoader(loader);
		
		// 创建RabbitMQ消息处理器
		class LeaveVenueHandler : public RabbitmqMessageJsonHandler {
		public:
			LeaveVenueHandler(const std::string& tag)
				: RabbitmqMessageJsonHandler(tag)
			{}

			virtual ~LeaveVenueHandler() {}

		protected:
			virtual bool receive(const std::string& message) override {
				std::string::size_type pos = message.find("MsgLeaveVenue");
				return (pos != std::string::npos);
			}

			virtual void handleImpl(const std::string& msgType, const std::string& json) override {
				VenueManager::getSingleton().handleMessage(msgType, json);
			}
		};
		RabbitmqMessageHandler::Ptr handler(new LeaveVenueHandler(consumerTag));
		RabbitmqConsumer::getSingleton().addHandler(handler);

		// 添加定时任务
		_timer = std::make_shared<int>();
		std::weak_ptr<int> weak(_timer);
		TimerManager::getSingleton().addAsyncTimer(2000, [weak] {
			std::shared_ptr<int> strong = weak.lock();
			if (!strong)
				return true;
			return VenueManager::getSingleton().onTimer();
			});
	}

	void VenueManager::registLoader(const VenueLoader::Ptr& loader) {
		std::lock_guard<std::mutex> lck(_mtx);

		_venueLoaders.insert(std::make_pair(loader->getGameType(), loader));
	}

	void VenueManager::registHandler(const std::shared_ptr<VenueInnerHandler>& handler) {
		std::lock_guard<std::mutex> lck(_mtx);

		_innerHandlers.push_back(handler);
	}

	std::shared_ptr<VenueInnerHandler> VenueManager::distributeHandler(const Venue::Ptr& venue) {
		std::shared_ptr<VenueInnerHandler> ret;
		if (!venue)
			return ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::shared_ptr<VenueInnerHandler> >::const_iterator it;
		it = _venueHandlerMap.find(venue->getId());
		if (it == _venueHandlerMap.end()) {
			int minCount = -1;
			int playerCount = 0;
			// 分配当前玩家数量最少的消息处理器
			std::vector<std::shared_ptr<VenueInnerHandler> >::const_iterator it1;
			for (it1 = _innerHandlers.begin(); it1 != _innerHandlers.end(); ++it1) {
				const std::shared_ptr<VenueInnerHandler>& handler = *it1;
				if (!handler->checkGameType(venue->getGameType()))
					continue;
				playerCount = handler->getTotalCount();
				if ((minCount < 0) || (minCount > playerCount)) {
					ret = handler;
					minCount = playerCount;
				}
			}
			if (ret)
				_venueHandlerMap.insert(std::make_pair(venue->getId(), ret));
		} else
			ret = it->second;
		return ret;
	}

	Venue::Ptr VenueManager::loadVenue(const std::string& id, int gameType, int& error) {
		Venue::Ptr venue = getVenue(id);
		if (venue) {
			if (gameType != venue->getGameType()) {
				// 已经加载，游戏类型不正确
				gameType = venue->getGameType();
				return nullptr;
			}
			// 已经加载，并且游戏类型正确
			return venue;
		}
		// 场地未加载
		class LoadVenueTask : public MysqlQueryTask {
		public:
			LoadVenueTask(const std::string& venueId)
				: _venueId(venueId)
				, _gameType(0)
				, _status(0)
			{}

			virtual ~LoadVenueTask() {}

		public:
			virtual QueryType buildQuery(std::string& sql) override {
				sql = "select `game_type`, `status` from `venue` where `id` = \"" + _venueId + "\"";
				return QueryType::Select;
			}

			virtual int fetchResult(sql::ResultSet* res) {
				int rows = 0;
				while (res->next()) {
					_gameType = res->getInt("game_type");
					_status = res->getInt("status");
					rows++;
				}
				return rows;
			}

		public:
			std::string _venueId;
			int _gameType;
			int _status;
		};
		MysqlQueryTask::Ptr task = std::make_shared<LoadVenueTask>(id);
		MysqlPool::getSingleton().syncQuery(task);
		if (!task->getSucceed()) {
			// 数据库访问失败
			error = static_cast<int>(EnterVenueResult::LOAD_FAILED);
			return nullptr;
		}
		LoadVenueTask* lvt = dynamic_cast<LoadVenueTask*>(task.get());
		if (lvt->_gameType != gameType) {
			// 游戏类型不正确
			error = static_cast<int>(EnterVenueResult::GAME_TYPE_ERROR);
			return nullptr;
		}
		if (lvt->_status != 0) {
			// 场地不可加载，可能是场地已锁定或者游戏已结束
			error = static_cast<int>(EnterVenueResult::STATUS_ERROR);
			return nullptr;
		}
		int count = 0;
		while (!lockLoading(id)) {
			if (count > 199) {
				// 等待其他线程加载场地超时(1s)，这种情况不应该发生
				ErrorS << "Wait for loading venue (id: " << id << ") timeout, this should not happen";
				return nullptr;
			}
			// 等待5毫秒
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			count++;
		}
		venue = getVenue(id);
		if (venue) {
			unlockLoading(id);
			return venue;
		}
		VenueLoader::Ptr loader = getVenueLoader(gameType);
		if (!loader) {
			unlockLoading(id);
			// 未注册相关场地加载器
			error = static_cast<int>(EnterVenueResult::LOAD_FAILED);
			return nullptr;
		}
		try {
			venue = loader->load(id);
			error = static_cast<int>(EnterVenueResult::LOAD_FAILED);
			if (venue) {
				std::shared_ptr<VenueInnerHandler> handler = distributeHandler(venue);
				if (handler) {
					ThreadWorker::Ptr worker = handler->getWorker();
					if (worker) {
						handler->addVenue(venue);
						venue->setHandler(handler);
						addVenue(venue);
						error = static_cast<int>(EnterVenueResult::SUCCESS);

						// 执行初始化
						std::weak_ptr<Venue> weakVenue = venue;
						worker->dispatch([weakVenue] () {
							Venue::Ptr strong = weakVenue.lock();
							if (!strong)
								return;
							strong->initialize();
						});
					}
					else {
						error = static_cast<int>(EnterVenueResult::HANDLER_ERROR);
						venue = nullptr;
					}
				}
				else {
					error = static_cast<int>(EnterVenueResult::DISTRIBUTE_FAILED);
					venue = nullptr;
				}
			}
		}
		catch (std::exception& ex) {
			ErrorS << "Load venue failed, error: " << ex.what();
			venue = nullptr;
		}
		unlockLoading(id);
		return venue;
	}

	Venue::Ptr VenueManager::getVenue(const std::string& id) {
		Venue::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it = _venues.find(id);
		if (it != _venues.end())
			ret = it->second;
		return ret;
	}

	bool VenueManager::hasVenue(const std::string& id) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it = _venues.find(id);
		return (it != _venues.end());
	}

	void VenueManager::addVenue(const Venue::Ptr& venue) {
		if (!venue)
			return;
		std::lock_guard<std::mutex> lck(_mtx);
		_venues.insert(std::make_pair(venue->getId(), venue));
	}

	void VenueManager::removeVenue(const std::string& id) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it;
		it = _venues.find(id);
		if (it != _venues.end())
			_venues.erase(it);
	}

	VenueLoader::Ptr VenueManager::getVenueLoader(int gameType) {
		VenueLoader::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<int, VenueLoader::Ptr>::const_iterator it = _venueLoaders.find(gameType);
		if (it != _venueLoaders.end())
			ret = it->second;
		return ret;
	}

	bool VenueManager::lockLoading(const std::string& id) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_set<std::string>::const_iterator it = _loadingVenues.find(id);
		if (it == _loadingVenues.end()) {
			_loadingVenues.insert(id);
			return true;
		}
		return false;
	}

	void VenueManager::unlockLoading(const std::string& id) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_set<std::string>::iterator it = _loadingVenues.find(id);
		if (it != _loadingVenues.end())
			_loadingVenues.erase(it);
	}

	void VenueManager::handleMessage(const std::string& msgType, const std::string& json) {
		if (msgType != "MsgLeaveVenue")
			return;
		std::stringstream ss(json);
		Json::Value jsonObj;
		ss >> jsonObj;
		std::string playerId;
		std::string venueId;
		std::string commandId;
		std::string routingKey;
		Json::Value& field1 = jsonObj["playerId"];
		if (field1.isString())
			playerId = field1.asString();
		Json::Value& field2 = jsonObj["venueId"];
		if (field2.isString())
			venueId = field2.asString();
		Json::Value& field3 = jsonObj["commandId"];
		if (field3.isString())
			commandId = field3.asString();
		Json::Value& field4 = jsonObj["routingKey"];
		if (field4.isString())
			routingKey = field4.asString();
		if (playerId.empty() || venueId.empty() || commandId.empty() || routingKey.empty()) {
			LOG_ERROR("Handle rabbitmq message error, some necessary fields are missing");
			return;
		}
		std::string curVenueId;
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (player)
			player->getVenueId(curVenueId);
		Venue::Ptr venue;
		ThreadWorker::Ptr dispThread;
		if (curVenueId == venueId) {
			venue = getVenue(venueId);
			if (venue) {
				std::shared_ptr<VenueInnerHandler> handler = venue->getHandler();
				if (handler)
					dispThread = handler->getWorker();
			}
		}
		if (dispThread) {
			std::string exchange = _directExchange;
			std::weak_ptr<Venue> weakVenue = venue;
			dispThread->dispatch([weakVenue, playerId, commandId, exchange, routingKey]() {
				int ret = 0;
				std::string errMsg = "success";
				Venue::Ptr strong = weakVenue.lock();
				if (strong) {
					ret = strong->onLeave(playerId, errMsg);
#ifdef _MSC_VER
					if (ret != 0) {
						// VC环境下gb2312编码转utf8
						errMsg = boost::locale::conv::to_utf<char>(errMsg, std::string("gb2312"));
					}
#endif
				}
				Json::Value tmp(Json::objectValue);
				tmp["commandId"] = commandId;
				tmp["result"] = ret;
				tmp["message"] = errMsg;
				RabbitmqClient::getSingleton().publishJson(exchange, routingKey, "CommandResult", tmp.toStyledString());
			});
		}
		else {
			Json::Value tmp(Json::objectValue);
			tmp["commandId"] = commandId;
			tmp["result"] = 0;
			tmp["message"] = std::string("success");
			RabbitmqClient::getSingleton().publishJson(_directExchange, routingKey, "CommandResult", tmp.toStyledString());
		}
	}

	int VenueManager::getTotalCount() {
		int total = 0;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, std::shared_ptr<VenueInnerHandler> >::const_iterator it;
		for (it = _venueHandlerMap.begin(); it != _venueHandlerMap.end(); ++it) {
			total += (it->second)->getTotalCount();
		}
		return total;
	}

	bool VenueManager::onTimer() {
		int total = getTotalCount();
		std::string redisKey = RedisKeys::SERVER_PLAYER_COUNT + _serverId;
		RedisPool::getSingleton().set(redisKey, total);
		return false;
	}
}