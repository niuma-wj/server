// WebsocketServer.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "Timer/TimerManager.h"
#include "Connection.h"
#include "WebsocketServer.h"
#include "SecurityManager.h"

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace NiuMa
{
	class WebsocketConnection : public Connection
	{
	public:
		WebsocketConnection(std::shared_ptr<WebsocketServer>& server, boost::asio::ip::tcp::socket&& socket)
			: _server(server)
			, _ws(std::move(socket))
			, _error(false)
			, _activeClose(false)
			, _activeCloseTime(0LL)
			, _sending(false)
		{
			boost::uuids::uuid uuid = boost::uuids::random_generator()();
			_uuid = boost::uuids::to_string(uuid);
		}

		virtual ~WebsocketConnection() {}

	public:
		void setRemoteIp(const std::string& ip) {
			_remoteIp = ip;
		}

		void setSession(const Session::Ptr& sess) {
			_session = sess;
		}

		const Session::Ptr& getSession() const {
			return _session;
		}

		void start() {
			// We need to be executing within a strand to perform async operations
			// on the I/O objects in this session. Although not strictly necessary
			// for single-threaded contexts, this example code is written to be
			// thread-safe by default.
			std::shared_ptr<WebsocketConnection> self = std::dynamic_pointer_cast<WebsocketConnection>(shared_from_this());
			boost::asio::dispatch(_ws.get_executor(), boost::beast::bind_front_handler(&WebsocketConnection::onStart, self));
		}

		/**
		 * 主动关闭连接
		 */
		void close() {
			_activeClose = true;
			_activeCloseTime = BaseUtils::getCurrentSecond();
			try {
				_ws.close(boost::beast::websocket::close_reason(std::string("active close.")));
			}
			catch (std::exception& ex) {
				ErrorS << "Close websocket error: " << ex.what();
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

	private:
		// Start the asynchronous operation
		void onStart() {
			// Set suggested timeout settings for the websocket
			_ws.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

			// Set a decorator to change the Server of the handshake
			_ws.set_option(boost::beast::websocket::stream_base::decorator(
				[](boost::beast::websocket::response_type& res) {
					std::string version = std::string(BOOST_BEAST_VERSION_STRING) + std::string(" websocket-server-async");
					res.set(boost::beast::http::field::server, version);
				}));
			// Accept the websocket handshake
			std::shared_ptr<WebsocketConnection> self = std::dynamic_pointer_cast<WebsocketConnection>(shared_from_this());
			_ws.async_accept(boost::beast::bind_front_handler(&WebsocketConnection::onAccept, self));
		}

		void onAccept(boost::beast::error_code ec) {
			if (ec) {
				ErrorS << "Accept error, message: " << ec.message();
				return;
			}
			// Read a message
			doRead();
		}

		void doRead() {
			try {
				// Read a message into our buffer
				std::shared_ptr<WebsocketConnection> self = std::dynamic_pointer_cast<WebsocketConnection>(shared_from_this());
				_ws.async_read(_buffer, boost::beast::bind_front_handler(&WebsocketConnection::onRead, self));
			}
			catch (std::exception& ex) {
				ErrorS << "Read data error: " << ex.what();
			}
			catch (...) {
				ErrorS << "Read data error.";
			}
		}

		void doWrite() {
			if (_error)
				return;
			std::shared_ptr<std::string> node = popSendNode();
			if (!node)
				return;
			try {
				std::shared_ptr<WebsocketConnection> self = std::dynamic_pointer_cast<WebsocketConnection>(shared_from_this());
				_ws.async_write(boost::asio::buffer(node->data(), node->size()),
					boost::beast::bind_front_handler(&WebsocketConnection::onWrite, self));
			}
			catch (std::exception& ex) {
				ErrorS << "Write data error: " << ex.what();
			}
			catch (...) {
				ErrorS << "Write data error.";
			}
		}

		void onRead(boost::beast::error_code ec, std::size_t bytes_transferred) {
			boost::ignore_unused(bytes_transferred);

			if ((ec == boost::beast::websocket::error::closed) || ec)
				onError(ec);
			else {
				// 注意，下面这一行代码不能删掉，删掉后websocket会异常断开，虽然不知道有什么用
				_ws.text(_ws.got_text());
				if (_session) {
					boost::asio::mutable_buffer buf = _buffer.data();
					try {
						_session->onRecieve(reinterpret_cast<char*>(buf.data()), buf.size());
					}
					catch (std::exception& ex) {
						ErrorS << "Recieve data error: " << ex.what() << ", length: " << buf.size();
					}
					// Clear the buffer
					_buffer.consume(_buffer.size());
				}
				doRead();
			}
		}

		void onWrite(boost::beast::error_code ec, std::size_t bytes_transferred) {
			boost::ignore_unused(bytes_transferred);
			if (ec)
				onError(ec);
			else
				doWrite();
		}

		void onError(boost::beast::error_code ec) {
			_error = true;

			std::shared_ptr<WebsocketServer> srv = _server.lock();
			if (srv && !(srv->isStopping())) {
				if (_session) {
					_session->onDisconnect();
					// 不再持有会话
					_session = Session::Ptr();
				}
				srv->removeConnection(shared_from_this());
			}
			std::string errMsg = ec.message();
			// This indicates that the session was closed
			if (ec == boost::beast::websocket::error::closed) {
				InfoS << "Session(id: " << _uuid << ") close.";
			}
			else {
				ErrorS << "Session(id: " << _uuid << ") error, msg: " << errMsg;
			}
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
					doWrite();
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
					doWrite();
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

	private:
		//
		std::weak_ptr<WebsocketServer> _server;

		// 连接(会话)ID
		std::string _uuid;

		// 远端ip地址
		std::string _remoteIp;

		// 对应的会话
		Session::Ptr _session;

		// 数据输入流
		boost::beast::websocket::stream<boost::beast::tcp_stream> _ws;

		// 接收缓存
		boost::beast::flat_buffer _buffer;

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

	const std::size_t WebsocketConnection::BLOCK_SIZE = 16384;

	class WebsocketService : public std::enable_shared_from_this<WebsocketService>
	{
	public:
		WebsocketService(int threadNum)
			: _ioc(threadNum)
		{}

		virtual ~WebsocketService() {}

	public:
		virtual bool start(int port) = 0;

		void stop() {
			// Stop the `io_context`. This will cause `run()`
			// to return immediately, eventually destroying the
			// `io_context` and all of the sockets in it.
			_ioc.stop();
		}

		void run() {
			_ioc.run();
			LOG_INFO("Websocket thread terminate.");
		}

		boost::asio::io_context& getContext() {
			return _ioc;
		}

	protected:
		// The io_context is required for all I/O
		boost::asio::io_context _ioc;
	};

	class WebsocetAcceptor : public WebsocketService
	{
	public:
		WebsocetAcceptor(const std::shared_ptr<WebsocketServer>& server, int threadNum)
			: WebsocketService(threadNum)
			, _server(server)
		{
			_acceptor = std::make_shared<boost::asio::ip::tcp::acceptor>(_ioc);
		}

		virtual ~WebsocetAcceptor() {}

	public:
		virtual bool start(int port) override {
			boost::asio::ip::address address = boost::asio::ip::make_address(std::string("0.0.0.0"));
			boost::asio::ip::tcp::endpoint endpoint(address, port);
			boost::beast::error_code ec;
			// Open the acceptor
			_acceptor->open(endpoint.protocol(), ec);
			if (ec) {
				ErrorS << "Open endpoint(0.0.0.0:" << port << ") error, message: " << ec.message();
				return false;
			}
			// Allow address reuse
			_acceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
			if (ec) {
				ErrorS << "Reuse address(0.0.0.0:" << port << ") error, message: " << ec.message();
				return false;
			}
			// Bind to the server address
			_acceptor->bind(endpoint, ec);
			if (ec) {
				ErrorS << "Bind to endpoint(0.0.0.0:" << port << ") error, message: " << ec.message();
				return false;
			}
			// Start listening for connections
			_acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
			if (ec) {
				ErrorS << "Listen to endpoint(0.0.0.0:" << port << ") error, message: " << ec.message();
				return false;
			}
			doAccept();
			return true;
		}

	private:
		void doAccept() {
			std::shared_ptr<WebsocetAcceptor> thiz = std::dynamic_pointer_cast<WebsocetAcceptor>(shared_from_this());
			try {
				// The new connection gets its own strand
				_acceptor->async_accept(boost::asio::make_strand(_ioc),
					boost::beast::bind_front_handler(&WebsocetAcceptor::onAccept, thiz));
			}
			catch (std::exception& ex) {
				ErrorS << "Accept error: " << ex.what();
			}
			catch (...) {
				ErrorS << "Accept error.";
			}
		}

		void onAccept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket) {
			if (ec) {
				ErrorS << "Accept error, message: " << ec.message();

				try {
					socket.close();
				}
				catch (std::exception& ex) {
					ErrorS << "Close socket error: " << ex.what();
				}
				catch (...) {
					ErrorS << "Close socket error.";
				}
			}
			else {
				// Create the session and run it
				//std::make_shared<session>(std::move(socket))->run();
				std::shared_ptr<WebsocketServer> strongRef = _server.lock();
				if (strongRef) {
					std::string remoteIp;
					std::string addr;
					try {
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
						try {
							socket.close();
						}
						catch (std::exception& ex) {
							ErrorS << "Close socket error: " << ex.what();
						}
						catch (...) {
							ErrorS << "Close socket error.";
						}
						return;
					}
					std::shared_ptr<WebsocketConnection> con = std::make_shared<WebsocketConnection>(strongRef, std::move(socket));
					con->setRemoteIp(remoteIp);
					con->setSession(strongRef->createSession(con));
					con->start();
					strongRef->addConnection(con);

					std::string id;
					con->getId(id);
					DebugS << "New connection(id: " << id << "), remote endpoint: " << addr;
				}
			}
			// Accept another connection
			doAccept();
		}

	private:
		//
		std::weak_ptr<WebsocketServer> _server;

		//
		std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
	};

	WebsocketServer::WebsocketServer(const SessionCreator::Ptr& creator)
		: _creator(creator)
	{}

	WebsocketServer::~WebsocketServer() {}

	bool WebsocketServer::start(int port, int threadNum) {
		if (_started)
			return true;
		unsigned int cpuNum = std::thread::hardware_concurrency();
		if (threadNum < 1)
			threadNum = cpuNum;
		// 最大8个线程做数据收发已经足够了
		if (threadNum > 8)
			threadNum = 8;
		_service = std::make_shared<WebsocetAcceptor>(shared_from_this(), threadNum);
		if (!_service->start(port))
			return false;
		std::weak_ptr<WebsocketService> weakService = _service;
		std::shared_ptr<std::thread> thread;
		for (int i = 0; i < threadNum; i++) {
			thread = std::make_shared<std::thread>([weakService]() {
				std::shared_ptr<WebsocketService> strongRef = weakService.lock();
				if (strongRef)
					strongRef->run();
				});
			_threads.emplace_back(thread);
		}
		// 添加定时任务
		std::weak_ptr<WebsocketServer> weakSelf = shared_from_this();
		TimerManager::getSingleton().addAsyncTimer(100, [weakSelf] {
			std::shared_ptr<WebsocketServer> strongSelf = weakSelf.lock();
			if (strongSelf) {
				return strongSelf->onTimer();
			}
			return true;
		});

		InfoS << "Websocket server startup, port: " << port << ", thread num: " << threadNum;

		_started = true;
		return true;
	}

	void WebsocketServer::stop() {
		if (!_started || _stopping)
			return;

		_stopping = true;

		// 关闭所有连接
		closeAllConnections();
		// 关闭服务
		if (_service)
			_service->stop();
		// 等待所有线程退出
		for (std::shared_ptr<std::thread>& thread : _threads) {
			thread->join();
		}
		_service.reset();
		_threads.clear();
		// 释放所有连接
		clearConnection();

		_started = false;
		_stopping = false;
	}

	bool WebsocketServer::isStopping() const {
		return _stopping;
	}

	Session::Ptr WebsocketServer::createSession(const std::shared_ptr<Connection>& con) const {
		return _creator->create(con);
	}

	void WebsocketServer::addConnection(const std::shared_ptr<Connection>& con) {
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

	void WebsocketServer::removeConnection(const std::shared_ptr<Connection>& con) {
		if (!con)
			return;

		std::string id;
		con->getId(id);

		std::lock_guard<std::mutex> lck(_mtxCon);

		ConnectionMap::const_iterator it = _connections.find(id);
		if (it != _connections.end())
			_connections.erase(it);

		con->abandon();
		_abandonedConnections.push_back(con);
	}

	void WebsocketServer::closeAllConnections() {
		std::lock_guard<std::mutex> lck(_mtxCon);

		ConnectionMap::const_iterator it = _connections.begin();
		while (it != _connections.end()) {
			const std::shared_ptr<Connection>& con = it->second;
			std::shared_ptr<WebsocketConnection> ws_con = std::dynamic_pointer_cast<WebsocketConnection>(con);
			if (ws_con)
				ws_con->close();
			++it;
		}
	}

	void WebsocketServer::clearConnection() {
		std::lock_guard<std::mutex> lck(_mtxCon);

		_connections.clear();
	}

	int WebsocketServer::getConnectionNums() {
		std::lock_guard<std::mutex> lck(_mtxCon);

		int count = static_cast<int>(_connections.size());
		return count;
	}

	std::shared_ptr<Connection> WebsocketServer::getConnectionInSequence() {
		std::lock_guard<std::mutex> lck(_mtxCon);

		return BaseUtils::getInSequence(_connections, _sequence);
	}

	void WebsocketServer::removeAbandoned() {
		std::lock_guard<std::mutex> lck(_mtxCon);

		if (_abandonedConnections.empty())
			return;

		time_t nowTime = BaseUtils::getCurrentSecond();
		time_t delta = 0LL;
		std::list<std::shared_ptr<Connection> >::iterator it = _abandonedConnections.begin();
		while (it != _abandonedConnections.end()) {
			const std::shared_ptr<Connection>& con = *it;
			delta = nowTime - con->getAbandonTime();
			if (delta < 10LL)
				break;
			it = _abandonedConnections.erase(it);
		}
	}

	void WebsocketServer::checkConnection() {
		// 定时任务每100毫秒执行一次，要在10秒内轮询完一遍所有连接，计算每执行一次要轮询多少个连接
		int count = getConnectionNums();
		if (count == 0)
			return;
		count = count / 100;
		if (count < 1)
			count = 1;
		std::string id;
		std::shared_ptr<WebsocketConnection> ws_con;
		// 当前时间戳，秒
		time_t nowTime = BaseUtils::getCurrentSecond();
		for (int i = 0; i < count; i++) {
			std::shared_ptr<Connection> con = getConnectionInSequence();
			if (con) {
				ws_con = std::dynamic_pointer_cast<WebsocketConnection>(con);
				if (!ws_con)
					break;
			}
			else
				break;
			const Session::Ptr& session = ws_con->getSession();
			if (!session) {
				// 理论上不会执行到这里
				removeConnection(con);
			}
			else if (!(session->isAlive(nowTime))) {
				con->getId(id);
				if (ws_con->activeCloseOver1s(nowTime)) {
					// 已经主动关闭超过1秒，连接还存在，正常不应该出现这种情况
					ErrorS << "Connection (uuid: " << id << ") has been active close over one second";
					session->onDisconnect();
					removeConnection(con);
				}
				else if (!(ws_con->isActiveClosed())) {
					// 主动关闭不活跃的连接
					ws_con->close();

					InfoS << "Active close connection (uuid: " << id << "), because it's inactive";
				}
			}
		}
	}

	bool WebsocketServer::onTimer() {
		if (!_started || _stopping)
			return true;

		removeAbandoned();
		checkConnection();
		return false;
	}
}