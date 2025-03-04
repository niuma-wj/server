// VenueInnerHandler.cpp

#include "Base/BaseUtils.h"
#include "VenueManager.h"
#include "VenueMessages.h"

namespace NiuMa 
{
	VenueInnerHandler::VenueInnerHandler(const MessageQueue::Ptr& queue)
		: PlayerSignatureHandler(queue)
		, _totalCount(0)
		, _counterMap(0)
		, _counterList(0)
	{
		// 添加空游戏类型
		addGameType(1);

		// 添加接收的消息类型
		addMessage(MsgHeartbeat::TYPE);
	}

	VenueInnerHandler::~VenueInnerHandler() {}

	bool VenueInnerHandler::checkGameType(int gameType) const {
		std::unordered_set<int>::const_iterator it = _gameTypes.find(gameType);
		return (it != _gameTypes.end());
	}

	void VenueInnerHandler::addGameType(int gameType) {
		_gameTypes.insert(gameType);
	}

	void VenueInnerHandler::addVenue(const Venue::Ptr& venue) {
		std::lock_guard<std::mutex> lck(_mtx);

		_venues.insert(std::make_pair(venue->getId(), venue));
		_counterMap++;
		if (_counterMap > 99999)
			_counterMap = 0;
	}

	Venue::Ptr VenueInnerHandler::getVenue(const std::string& id) const {
		Venue::Ptr ret;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it;
		it = _venues.find(id);
		if (it != _venues.end())
			ret = it->second;
		return ret;
	}

	void VenueInnerHandler::removeVenue(const std::string& id) {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it;
		it = _venues.find(id);
		if (it != _venues.end()) {
			_venues.erase(it);
			_counterMap++;
			if (_counterMap > 99999)
				_counterMap = 0;
		}
	}

	bool VenueInnerHandler::hasVenue(const std::string& id) const {
		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it = _venues.find(id);
		return (it != _venues.end());
	}

	void VenueInnerHandler::setPlayerCount(const std::string& id, int count) {
		if (id.empty())
			return;

		std::lock_guard<std::mutex> lck(_mtx);

		std::unordered_map<std::string, Venue::Ptr>::const_iterator it1 = _venues.find(id);
		if (it1 == _venues.end())
			return;
		int oldCount = 0;
		std::unordered_map<std::string, int>::iterator it2 = _playerCounts.find(id);
		if (it2 != _playerCounts.end()) {
			oldCount = it2->second;
			if (count == 0)
				_playerCounts.erase(it2);
			else
				it2->second = count;
		}
		else if (count > 0)
			_playerCounts.insert(std::make_pair(id, count));
		_totalCount -= oldCount;
		_totalCount += count;
	}

	int VenueInnerHandler::getTotalCount() const {
		std::lock_guard<std::mutex> lck(_mtx);

		return _totalCount;
	}

	void VenueInnerHandler::initialize() {
		ThreadWorker::Ptr worker = getWorker();
		if (!worker)
			return;
		std::shared_ptr<VenueInnerHandler> thiz = std::dynamic_pointer_cast<VenueInnerHandler>(shared_from_this());
		std::weak_ptr<VenueInnerHandler> weakThiz = thiz;
		worker->addSyncTimer(30, [weakThiz] {
			std::shared_ptr<VenueInnerHandler> strong = weakThiz.lock();
			if (!strong)
				return true;
			return strong->onTimer();
		});
	}

	bool VenueInnerHandler::receive(const NetMessage::Ptr& netMsg) const {
		if (!MessageHandler::receive(netMsg))
			return false;
		MsgVenueInner* msg = dynamic_cast<MsgVenueInner*>(netMsg->getMessage().get());
		if (msg == nullptr)
			return false;
		return hasVenue(msg->getVenueId());
	}

	bool VenueInnerHandler::onMessage(const NetMessage::Ptr& netMsg) {
		MsgVenueInner* msg = dynamic_cast<MsgVenueInner*>(netMsg->getMessage().get());
		if (msg == nullptr)
			return false;
		Venue::Ptr venue = getVenue(msg->getVenueId());
		if (!venue || venue->isObsolete())
			return true;
		return venue->onMessage(netMsg);
	}

	bool VenueInnerHandler::onTimer() {
		updateVenueList();

		time_t nowTime = 0LL;
		time_t delta = 0LL;
		std::vector<Venue::Ptr>::const_iterator it = _venueList.begin();
		while (it != _venueList.end()) {
			const Venue::Ptr& venue = *it;
			if (venue->isObsolete()) {
				if (nowTime == 0)
					nowTime = BaseUtils::getCurrentMillisecond();
				delta = nowTime - venue->getObsoleteTime();
				delta /= 1000LL;
				if (delta > 30LL) {
					removeVenue(venue->getId());
					VenueManager::getSingleton().removeVenue(venue->getId());
				}
			} else
				venue->onTimer();
			++it;
		}
		return false;
	}

	void VenueInnerHandler::updateVenueList() {
		std::lock_guard<std::mutex> lck(_mtx);

		if (_counterMap == _counterList)
			return;
		_counterList = _counterMap;
		_venueList.clear();
		std::unordered_map<std::string, Venue::Ptr>::const_iterator it = _venues.begin();
		while (it != _venues.end()) {
			const Venue::Ptr& venue = it->second;
			_venueList.push_back(venue);
			++it;
		}
	}

	std::vector<Venue::Ptr>& VenueInnerHandler::getVenueList() {
		return _venueList;
	}
}