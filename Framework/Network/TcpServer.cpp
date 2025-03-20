// ServerPool.cpp

#include "TcpServer.h"
#include "Connection.h"
#include "SecurityManager.h"
#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Timer/TimerManager.h"
#include <boost/asio.hpp>
#include <boost/asio/require.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sstream>

namespace NiuMa {
    class Service {
    public:
        Service() {
            _work = boost::asio::require(_context.get_executor(),
                boost::asio::execution::outstanding_work.tracked);
        }

        virtual ~Service() {}

    public:
        void run() {
            _context.run();
            LOG_INFO("Service thread terminate.");
        }

        virtual void stop() {
            // Allow run() to exit.
            _work = boost::asio::any_io_executor();
            //_context.stop();
        }

        boost::asio::io_context& getContext() {
            return _context;
        }

    protected:
        boost::asio::io_context _context;
        boost::asio::any_io_executor _work;
    };

    // TCP网络连接实现类
    class ConnectionImpl : public Connection
    {
    public:
        ConnectionImpl(const std::shared_ptr<TcpServer>& srv, std::shared_ptr<Service>& service)
            : _server(srv)
            , _socket(service->getContext())
            , _error(false)
            , _activeClose(false)
            , _sending(false)
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            _uuid = boost::uuids::to_string(uuid);
        }

        void start() {
            std::shared_ptr<TcpServer> srv = _server.lock();
            if (srv) {
                _session = srv->newSession(shared_from_this());
                do_read();
            }
        }

        void setRemoteIp(const std::string& ip) {
            _remoteIp = ip;
        }

        /**
         * 主动关闭连接
         */
        void close() {
            _activeClose = true;
            _activeCloseTime = BaseUtils::getCurrentSecond();
            try {
                _socket.close();
            }
            catch (std::exception& ex) {
                ErrorS << "Close socket error: " << ex.what();
            }
        }

        /**
         * 是否已经主动关闭
         */
        bool isActiveClosed() {
            return _activeClose;
        }

        /**
         * 主动关闭是否已经超过1秒
         */
        bool activeCloseOver1s(const time_t& nowTime) {
            if (!_activeClose)
                return false;
            time_t delta = nowTime - _activeCloseTime;
            if (delta > 1LL)
                return true;
            return false;
        }

        friend class Acceptor;

    private:
        void do_read() {
            std::weak_ptr<ConnectionImpl> weakSelf(std::dynamic_pointer_cast<ConnectionImpl>(shared_from_this()));
            _socket.async_read_some(boost::asio::buffer(_data, 1024),
                [weakSelf] (boost::system::error_code ec, std::size_t length) {
                    std::shared_ptr<ConnectionImpl> self = weakSelf.lock();
                    if (self)
                        self->onAsyncRead(ec, length);
                });
        }

        void do_write() {
            std::shared_ptr<std::string> node = popSendNode();
            if (!node)
                return;

            std::weak_ptr<ConnectionImpl> weakSelf(std::dynamic_pointer_cast<ConnectionImpl>(shared_from_this()));
            boost::asio::async_write(_socket, boost::asio::buffer(node->data(), node->size()),
                [weakSelf](boost::system::error_code ec, std::size_t length) {
                    std::shared_ptr<ConnectionImpl> self = weakSelf.lock();
                    if (self)
                        self->onAsyncWrite(ec, length);
                });
        }

        boost::asio::ip::tcp::socket& getSocket() {
            return _socket;
        }

        void onAsyncRead(boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                if (_session)
                    _session->onRecieve(_data, length);

                do_read();
            } else/* if (ec != boost::asio::error::operation_aborted)*/ {
                // 连接断开
                onError(ec, true);
            }/* else {
                // 操作取消
                InfoS << "Session(id: " << _uuid << ") error, msg: " << ec.message();
            }*/
        }

        void onAsyncWrite(boost::system::error_code ec, std::size_t /*length*/) {
            if (!ec) {
                do_write();
            } else/* if (ec != boost::asio::error::operation_aborted)*/ {
                // 连接错误
                onError(ec, false);
            }/* else {
                // 操作取消
                InfoS << "Session(id: " << _uuid << ") error, msg: " << ec.message();
            }*/
        }

        void onError(boost::system::error_code ec, bool readOrWrite) {
            _error = true;

            std::shared_ptr<TcpServer> srv = _server.lock();
            if (srv && !(srv->isStopping())) {
                srv->removeConnection(std::dynamic_pointer_cast<ConnectionImpl>(shared_from_this()));
                if (_session)
                    _session->onDisconnect();
            }
            InfoS << "Session(id: " << _uuid << ") error, msg: " << ec.message();
        }

        bool addSendNode(const std::shared_ptr<std::string>& node) {
            std::lock_guard<std::mutex> lck(_mtxSend);

            _sendQueue.push_back(node);

            return _sending;
        }

        std::shared_ptr<std::string> popSendNode() {
            std::lock_guard<std::mutex> lck(_mtxSend);

            std::shared_ptr<std::string> node;
            if (_sendQueue.empty())
                _sending = false;
            else {
                node = _sendQueue.front();
                _sendQueue.pop_front();
                _sending = true;
            }
            return node;
        }

        void splitSend(const char* buf, std::size_t length) {
            // 将发送内容分割成最大16KB分块发送，以免发送缓冲区溢出
            bool flag = false;
            std::size_t size = 0;
            std::size_t total = 0;
            std::shared_ptr<std::string> node;
            while (total < length) {
                size = BLOCK_SIZE;
                if ((size + total) > length)
                    size = length - total;
                node = std::make_shared<std::string>((&buf[total]), size);
                flag = addSendNode(node);
                if (!flag)
                    do_write();
                total += size;
            }
        }

    public:
        virtual void getId(std::string& id) const override {
            id = _uuid;
        }

        virtual void getRemoteIp(std::string& remoteIp) const override {
            remoteIp = _remoteIp;
        }

        virtual void send(const char* buf, std::size_t length) override {
            if (_error)
                return;
            if (buf == nullptr || length == 0)
                return;
            splitSend(buf, length);
        }

        virtual void send(const std::shared_ptr<std::string>& data) override {
            if (_error)
                return;
            if (!data || data->empty())
                return;
            if (data->size() <= BLOCK_SIZE) {
                bool flag = addSendNode(data);
                if (!flag)
                    do_write();
                return;
            }
            splitSend(data->data(), data->size());
        }

        virtual bool isClosed() override {
            if (_error)
                return true;
            if (_activeClose)
                return true;
            return false;
        }

        const Session::Ptr& getSession() const {
            return _session;
        }

    private:
        // 服务器
        std::weak_ptr<TcpServer> _server;

        // 
        boost::asio::ip::tcp::socket _socket;

        // 连接(会话)ID
        std::string _uuid;

        // 远端ip地址
        std::string _remoteIp;

        // 对应的会话
        Session::Ptr _session;

        // 接收数据的缓存
        char _data[1024];

        // 连接是否发生错误
        std::atomic<bool> _error;

        // 是否已经主动关闭连接
        std::atomic<bool> _activeClose;

        // 主动关闭时间
        time_t _activeCloseTime;

        // 数据发送队列
        std::list<std::shared_ptr<std::string> > _sendQueue;

        // 是否正在发送数据
        bool _sending;

        // 发送队列信号量
        std::mutex _mtxSend;

        // 
        static const std::size_t BLOCK_SIZE;
    };

    const std::size_t ConnectionImpl::BLOCK_SIZE = 16384;

    class Acceptor : public Service {
    public:
        Acceptor(const std::shared_ptr<TcpServer>& srv, boost::asio::ip::tcp::endpoint& endpoint)
            : _server(srv)
        {
            _acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(_context, endpoint);
            do_accept();
        }

        virtual ~Acceptor() {}

        virtual void stop() {
            _acceptor->close();

            Service::stop();
        }

    private:
        void do_accept() {
            std::shared_ptr<TcpServer> strongServer = _server.lock();
            if (!strongServer)
                return;
            std::shared_ptr<Service> service = strongServer->getService();
            if (!service)
                return;
            std::shared_ptr<ConnectionImpl> connection = std::make_shared<ConnectionImpl>(strongServer, service);
            _acceptor->async_accept(connection->getSocket(),
                [this, connection](boost::system::error_code ec) {
                    onAccept(ec, connection);
                    if (_acceptor->is_open())
                        do_accept();
                });
        }

        void onAccept(boost::system::error_code ec, const std::shared_ptr<ConnectionImpl>& con) {
            if (!ec) {
                std::string remoteIp;
                std::string addr;
                try {
                    boost::asio::ip::tcp::socket& socket = con->getSocket();
                    boost::asio::ip::tcp::endpoint endpoint = socket.remote_endpoint();
                    remoteIp = endpoint.address().to_string();
                    unsigned short port = endpoint.port();
                    addr = remoteIp + ":" + std::to_string(port);
                }
                catch (std::exception& /*ex*/) {
                    addr = "unknown";
                }
                if (SecurityManager::getSingleton().checkBlacklist(remoteIp)) {
                    // 远端ip在黑名单中，断开连接
                    con->close();
                    return;
                }
                con->setRemoteIp(remoteIp);
                con->start();
                std::shared_ptr<TcpServer> strongServer = _server.lock();
                if (strongServer)
                    strongServer->addConnection(con);

                std::string id;
                con->getId(id);
                DebugS << "New connection(id: " << id << "), remote endpoint: " << addr;
            }
        }

    private:
        // 服务池所有者
        std::weak_ptr<TcpServer> _server;

        // 监听器
        std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
    };

	TcpServer::TcpServer(const SessionCreator::Ptr& creator)
        : _creator(creator)
        , _pos(0)
    {}

	TcpServer::~TcpServer() {}

	void TcpServer::start(unsigned short port, unsigned int threadNum) {
		if (_started)
			return;

        std::shared_ptr<Service> service;
        std::shared_ptr<std::thread> thread;
        unsigned int cpuNum = std::thread::hardware_concurrency();
        if (threadNum < 1)
            threadNum = cpuNum;
        // 最大8个线程做数据收发已经足够了
        if (threadNum > 8)
            threadNum = 8;
		/*for (unsigned int i = 0; i < threadNum; i++) {
            service = std::make_shared<Service>();
			thread = std::make_shared<std::thread>([service]() { service->run(); });
            _services.emplace_back(service);
            _threads.emplace_back(thread);
		}*/
        // 使用单个service多个线程，所有连接进来的socket都绑定到同一个service，而不是每个线程
        // 唯一对应一个service（即一个socket绑定到某个线程，固定由该线程做数据收发），这样做的
        // 好处是当某个socket的数据收发比较繁忙的时候，其他socket可以任意切换到其他线程来做数
        // 据收发，避免网络阻塞
        service = std::make_shared<Service>();
        _services.emplace_back(service);
        for (unsigned int i = 0; i < threadNum; i++) {
            thread = std::make_shared<std::thread>([service]() { service->run(); });
            _threads.emplace_back(thread);
        }
        boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), port);
        std::shared_ptr<TcpServer> self = shared_from_this();
        _acceptor = std::make_shared<Acceptor>(self, endpoint);
        std::shared_ptr<Acceptor> acceptor = _acceptor;
        thread = std::make_shared<std::thread>([acceptor] { acceptor->run(); });
        _threads.emplace_back(thread);

        // 添加定时任务
        std::weak_ptr<TcpServer> weakSelf = shared_from_this();
        TimerManager::getSingleton().addAsyncTimer(100, [weakSelf] {
            std::shared_ptr<TcpServer> strongSelf = weakSelf.lock();
            if (strongSelf) {
                return strongSelf->onTimer();
            }
            return true;
        });

        InfoS << "Tcp server startup, port: " << port << ", thread num: " << threadNum;

        _started = true;
	}

    void TcpServer::stop() {
        if (!_started || _stopping)
            return;

        _stopping = true;

        // 关闭监听器
        if (_acceptor)
            _acceptor->stop();
        // 关闭所有连接
        closeAllConnections();
        // 关闭会话服务
        for (std::shared_ptr<Service>& service : _services) {
            service->stop();
        }
        // 等待所有线程退出
        for (std::shared_ptr<std::thread>& thread : _threads) {
            thread->join();
        }
        _services.clear();
        _acceptor.reset();
        _threads.clear();
        // 释放所有连接
        clearConnection();
        
        _started = false;
        _stopping = false;
    }

    bool TcpServer::isStopping() const {
        return _stopping;
    }

    std::shared_ptr<Service> TcpServer::getService() {
        if (_services.empty())
            return std::shared_ptr<Service>();
        if (_pos >= _services.size())
            _pos = 0;
        std::shared_ptr<Service> ret = _services.at(_pos);
        _pos++;
        return ret;
    }

    Session::Ptr TcpServer::newSession(const std::shared_ptr<Connection>& con) {
        return _creator->create(con);
    }

    void TcpServer::addConnection(const std::shared_ptr<ConnectionImpl>& con) {
        if (!con)
            return;

        std::string id;
        con->getId(id);

        std::lock_guard<std::mutex> lck(_mtxCon);

        ConnectionMap::const_iterator it = _connections.find(id);
        if (it == _connections.end()) {
            _connections.insert(std::make_pair(id, con));
            _sequence.push_back(id);
        }
        else {
            ErrorS << "Add new session with id: " << id << " error, session with the same id already existed.";
        }
    }

    void TcpServer::removeConnection(const std::shared_ptr<ConnectionImpl>& con) {
        if (!con)
            return;

        std::string id;
        con->getId(id);

        std::lock_guard<std::mutex> lck(_mtxCon);

        ConnectionMap::const_iterator it = _connections.find(id);
        if (it != _connections.end())
            _connections.erase(it);
    }

    void TcpServer::closeAllConnections() {
        std::lock_guard<std::mutex> lck(_mtxCon);

        ConnectionMap::const_iterator it = _connections.begin();
        while (it != _connections.end()) {
            const std::shared_ptr<ConnectionImpl>& con = it->second;
            con->close();
            ++it;
        }
    }

    void TcpServer::clearConnection() {
        std::lock_guard<std::mutex> lck(_mtxCon);

        _connections.clear();
    }

    int TcpServer::getConnectionNums() {
        std::lock_guard<std::mutex> lck(_mtxCon);

        int count = static_cast<int>(_connections.size());
        return count;
    }

    std::shared_ptr<ConnectionImpl> TcpServer::getConnectionInSequence() {
        std::lock_guard<std::mutex> lck(_mtxCon);

        return BaseUtils::getInSequence(_connections, _sequence);
    }

    void TcpServer::checkConnection() {
        // 定时任务每100毫秒执行一次，要在10秒内轮询完一遍所有连接，计算每执行一次要轮询多少个连接
        int count = getConnectionNums();
        if (count == 0)
            return;
        count = count / 100;
        if (count < 1)
            count = 1;
        std::string id;
        // 当前时间戳，秒
        time_t nowTime = BaseUtils::getCurrentSecond();
        for (int i = 0; i < count; i++) {
            std::shared_ptr<ConnectionImpl> con = getConnectionInSequence();
            if (!con)
                break;
            const Session::Ptr& session = con->getSession();
            if (!session) {
                // 理论上不会执行到这里
                removeConnection(con);
            }
            else if (!(session->isAlive(nowTime))) {
                con->getId(id);
                if (con->activeCloseOver1s(nowTime)) {
                    // 已经主动关闭超过1秒，连接还存在，正常不应该出现这种情况
                    ErrorS << "Connection (uuid: " << id << ") has been active close over one second";
                    session->onDisconnect();
                    removeConnection(con);
                }
                else if (!(con->isActiveClosed())) {
                    // 主动关闭不活跃的连接
                    {
                        InfoS << "Active close connection (uuid: " << id << "), because it's inactive";
                    }
                    con->close();
                }
            }
        }
    }

    bool TcpServer::onTimer() {
        if (!_started || _stopping)
            return true;

        checkConnection();
        return false;
    }
}