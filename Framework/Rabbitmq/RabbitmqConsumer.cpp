// RabbitmqConsumer.cpp

#include "Base/Log.h"
#include "Timer/TimerManager.h"
#include "RabbitmqConsumer.h"

namespace NiuMa {
    template<> RabbitmqConsumer* Singleton<RabbitmqConsumer>::_inst = nullptr;

	RabbitmqConsumer::RabbitmqConsumer()
        : _startFlag(false)
        , _changed(false)
    {}

	RabbitmqConsumer::~RabbitmqConsumer() {}

    void RabbitmqConsumer::start() {
        if (_startFlag)
            return;
        _startFlag = true;
        _timer = std::make_shared<int>();
        std::weak_ptr<int> weak(_timer);
        // 添加定时任务
        TimerManager::getSingleton().addAsyncTimer(50, [weak] {
            std::shared_ptr<int> strong = weak.lock();
            if (!strong)
                return true;
            return RabbitmqConsumer::getSingleton().onTimer();
        });
    }

    void RabbitmqConsumer::addHandler(const RabbitmqMessageHandler::Ptr& handler) {
        if (!handler)
            return;
        if (hasHandler(handler))
            return;
        std::lock_guard<std::mutex> lck(_mtx);

        std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> >::iterator it = _handlers.find(handler->getTag());
        if (it == _handlers.end()) {
            _handlers.insert(std::make_pair(handler->getTag(), std::unordered_set<RabbitmqMessageHandler::Ptr>()));
            it = _handlers.find(handler->getTag());
        }
        if (it != _handlers.end()) {
            std::unordered_set<RabbitmqMessageHandler::Ptr>& setRef = it->second;
            setRef.insert(handler);
        }
        _changed = true;
    }

    bool RabbitmqConsumer::removeHandler(const RabbitmqMessageHandler::Ptr& handler) {
        if (!handler)
            return false;

        bool ret = false;

        std::lock_guard<std::mutex> lck(_mtx);

        std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> >::iterator it = _handlers.find(handler->getTag());
        if (it != _handlers.end()) {
            std::unordered_set<RabbitmqMessageHandler::Ptr>& setRef = it->second;
            std::unordered_set<RabbitmqMessageHandler::Ptr>::iterator it_s = setRef.find(handler);
            if (it_s != setRef.end()) {
                setRef.erase(it_s);
                ret = true;
                _changed = true;
            }
            if (setRef.empty()) {
                _handlers.erase(it);
            }
        }
        return false;
    }

    void RabbitmqConsumer::getHandlers(const std::string& tag, std::vector<RabbitmqMessageHandler::Ptr>& handlers) {
        std::lock_guard<std::mutex> lck(_mtx);

        std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> >::iterator it = _handlers.find(tag);
        if (it != _handlers.end()) {
            const std::unordered_set<RabbitmqMessageHandler::Ptr>& setRef = it->second;
            for (const RabbitmqMessageHandler::Ptr& handler : setRef) {
                handlers.emplace_back(handler);
            }
        }
    }

    bool RabbitmqConsumer::hasHandler(const RabbitmqMessageHandler::Ptr& handler) {
        std::lock_guard<std::mutex> lck(_mtx);

        std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> >::iterator it = _handlers.find(handler->getTag());
        if (it == _handlers.end())
            return false;
        const std::unordered_set<RabbitmqMessageHandler::Ptr>& setRef = it->second;
        std::unordered_set<RabbitmqMessageHandler::Ptr>::const_iterator it_s = setRef.find(handler);
        return (it_s != setRef.end());
    }

    void RabbitmqConsumer::consume(const std::shared_ptr<std::string>& message, const std::string& tag) {
        std::vector<RabbitmqMessageHandler::Ptr> handlers;
        getHandlers(tag, handlers);
        if (handlers.empty())
            return;

        for (const RabbitmqMessageHandler::Ptr& handler : handlers)
            handler->push(message);
    }

    bool RabbitmqConsumer::isChanged() {
        std::lock_guard<std::mutex> lck(_mtx);

        return _changed;
    }

    void RabbitmqConsumer::syncHandlerList() {
        _handlerList.clear();

        std::lock_guard<std::mutex> lck(_mtx);
        std::unordered_map<std::string, std::unordered_set<RabbitmqMessageHandler::Ptr> >::const_iterator it;
        for (it = _handlers.begin(); it != _handlers.end(); it++) {
            const std::unordered_set<RabbitmqMessageHandler::Ptr>& setRef = it->second;
            for (const RabbitmqMessageHandler::Ptr& handler : setRef)
                _handlerList.emplace_back(handler);
        }
        _changed = false;
    }

    bool RabbitmqConsumer::onTimer() {
        if (isChanged())
            syncHandlerList();
        if (_handlerList.empty())
            return false;
        std::vector<RabbitmqMessageHandler::Ptr>::const_iterator it;
        for (it = _handlerList.begin(); it != _handlerList.end(); it++) {
            const RabbitmqMessageHandler::Ptr& handler = *it;
            try {
                handler->handle();
            }
            catch (std::exception& ex) {
                ErrorS << "Handle message error: " << ex.what();
            }
        }
        return false;
    }
}