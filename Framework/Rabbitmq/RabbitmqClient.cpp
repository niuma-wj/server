// RabbitmqClient.cpp

#include "Base/Log.h"
#include "Base/BaseUtils.h"
#include "RabbitmqClient.h"
#include "RabbitmqConsumer.h"
#include "RabbitmqUtils.h"

#include <rabbitmq-c/tcp_socket.h>

#include "jsoncpp/include/json/json.h"

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/time.h>
#endif

namespace NiuMa {
	typedef std::function<void(bool)> ConnectionListener;

	class RabbitmqConnectionImpl : public RabbitmqConnection {
	public:
		RabbitmqConnectionImpl(const std::string& host,
			int port, const std::string& vhost,
			const std::string& username,
			const std::string& password,
			const ConnectionListener& listener)
			: _host(host)
			, _port(port)
			, _vhost(vhost)
			, _username(username)
			, _password(password)
			, _listener(listener)
			, _state(nullptr)
			, _idAllocator(1)
		{}

		virtual ~RabbitmqConnectionImpl() {
			destroy();
		}

	public:
		virtual void* getState() override {
			return static_cast<void*>(_state);
		}

		virtual void setOk(bool setting) override {
			RabbitmqConnection::setOk(setting);

			if (_listener)
				_listener(setting);
		}

		void setState(amqp_connection_state_t state) {
			_state = state;
		}

		int allocateId() {
			std::lock_guard<std::mutex> lck(_mtxId);

			int id = _idAllocator;
			_idAllocator++;
			return id;
		}

		void resetId() {
			std::lock_guard<std::mutex> lck(_mtxId);

			_idAllocator = 1;
		}

		void destroy() {
			if (_state != nullptr) {
				amqp_rpc_reply_t reply = amqp_connection_close(_state, AMQP_REPLY_SUCCESS);
				RabbitmqUtils::checkReplyError(reply, "Closing connection");
				amqp_destroy_connection(_state);
				_state = nullptr;
			}
		}

	public:
		// 服务器主机地址
		std::string _host;

		// 端口
		int _port;

		// 虚拟主机
		std::string _vhost;

		// 用户名
		std::string _username;

		// 密码
		std::string _password;

		// 连接监听函数
		ConnectionListener _listener;

		// 连接内部结构体
		amqp_connection_state_t _state;

	private:
		// 信号量
		std::mutex _mtxId;

		// 通道id分配器，只能递增
		int _idAllocator;
	};

	class PublishItem : public std::enable_shared_from_this<PublishItem> {
	public:
		PublishItem(const std::string& exchange,
			const std::string& routingKey,
			const std::string& message)
			: _exchange(exchange)
			, _routingKey(routingKey)
			, _message(message)
		{}

		virtual ~PublishItem() = default;

		typedef std::shared_ptr<PublishItem> Ptr;

	public:
		// 交换机
		const std::string _exchange;

		// 路由键
		const std::string _routingKey;

		// 消息内容
		const std::string _message;
	};

	template<> RabbitmqClient* Singleton<RabbitmqClient>::_inst = nullptr;

	RabbitmqClient::RabbitmqClient()
		: _stopFlag(false)
		, _publishId(0)
	{}

	bool RabbitmqClient::start(const std::string& host,
		int port, const std::string& vhost,
		const std::string& username,
		const std::string& password,
		const RabbitmqConfig::Ptr& config) {
		if (_connection) {
			ErrorS << "Client has already started.";
			return false;
		}
		_connection = std::make_shared<RabbitmqConnectionImpl>(host, port, vhost, username, password,
			[](bool isOk) {
				RabbitmqClient::getSingleton().onConnection(isOk);
			});
		_config = config;
		if (!checkConnection()) {
			_connection.reset();
			return false;
		}
		_thread = std::make_shared<std::thread>([]() {
			RabbitmqClient::getSingleton().threadFunc();
		});
		return true;
	}

	void RabbitmqClient::stop(const RabbitmqConfig::Ptr& config) {
		if (!_connection)
			return;

		_stopFlag = true;
		if (_thread) {
			_thread->join();
			_thread.reset();
		}
		if (_connection->isOk()) {
			if (config)
				config->config(_connection);
			int id = getPublishChannel();
			if (id != 0) {
				closeChannel(id);
				setPublishChannel(0);
			}
		}
		_connection.reset();
	}

	bool RabbitmqClient::checkConnection() {
		if (_connection->isOk())
			return true;
		amqp_connection_state_t state = amqp_new_connection();
		amqp_socket_t* socket = amqp_tcp_socket_new(state);
		if (socket == nullptr) {
			ErrorS << "Create TCP socket failed.";
			amqp_destroy_connection(state);
			return false;
		}
		std::shared_ptr<RabbitmqConnectionImpl> impl = std::dynamic_pointer_cast<RabbitmqConnectionImpl>(_connection);
		impl->destroy();
		int status = amqp_socket_open(socket, impl->_host.c_str(), impl->_port);
		if (status != 0) {
			ErrorS << "Opening TCP socket failed, status: " << status;
			amqp_destroy_connection(state);
			return false;
		}
		amqp_rpc_reply_t reply = amqp_login(state, impl->_vhost.c_str(), 0, 131072, 3,
			AMQP_SASL_METHOD_PLAIN, impl->_username.c_str(), impl->_password.c_str());
		int ret = RabbitmqUtils::checkReplyError(reply, "Logging in");
		if (ret != 0) {
			amqp_destroy_connection(state);
			return false;
		}
		impl->setState(state);
		impl->resetId();
		impl->setOk(true);
		if (_config)
			_config->config(_connection);
		InfoS << "Connect to rabbitmq server (" << impl->_host << ":" << impl->_port << "), virtual host: " << impl->_vhost;
		return true;
	}

	void RabbitmqClient::onConnection(bool isOk) {
		if (_stopFlag)
			return;

		if (!isOk) {
			setPublishChannel(0);
		}
	}

	int RabbitmqClient::openChannel() {
		if (!(_connection->isOk()))
			return 0;
		std::shared_ptr<RabbitmqConnectionImpl> impl = std::dynamic_pointer_cast<RabbitmqConnectionImpl>(_connection);
		int id = impl->allocateId();
		amqp_channel_open(impl->_state, id);
		amqp_rpc_reply_t reply = amqp_get_rpc_reply(impl->_state);
		int ret = RabbitmqUtils::checkReplyError(reply, "Opening channel");
		if (ret)
			return 0;
		return id;
	}

	void RabbitmqClient::closeChannel(int channelId) {
		std::shared_ptr<RabbitmqConnectionImpl> impl = std::dynamic_pointer_cast<RabbitmqConnectionImpl>(_connection);
		amqp_rpc_reply_t reply = amqp_channel_close(impl->_state,
			static_cast<amqp_channel_t>(channelId), AMQP_REPLY_SUCCESS);
		RabbitmqUtils::checkReplyError(reply, "Closing channel");
	}

	bool RabbitmqClient::isConnectionOk() const {
		if (!_connection)
			return false;
		return _connection->isOk();
	}

	bool RabbitmqClient::publish(const std::string& exchange, const std::string& routingKey, const std::string& message) {
		if (!_connection || !(_connection->isOk())) {
			LOG_ERROR("Publish message error, connection is not ready.");
			return false;
		}
		if (exchange.empty() || message.empty())
			return false;
		PublishItem::Ptr item = std::make_shared<PublishItem>(exchange, routingKey, message);

		std::lock_guard<std::mutex> lck(_mtxPublish);

		if (_publishItems.size() > 2999) {
			// 最多仅缓存3000条消息
			LOG_ERROR("Cached message quantity exceed maximum(3000), the head of publish queue will be discarded, this will cause message lost.");
			while (_publishItems.size() > 2999) {
				_publishItems.pop();
			}
		}
		_publishItems.push(item);
		return true;
	}

	bool RabbitmqClient::publishJson(const std::string& exchange, const std::string& routingKey, const std::string& msgType, const std::string& json) {
		std::string base64;
		if (!BaseUtils::encodeBase64(base64, json.c_str(), static_cast<int>(json.size()))) {
			LOG_ERROR("Encode base64 failed.");
			return false;
		}
		Json::Value msg(Json::objectValue);
		msg["msgType"] = msgType;
		msg["msgPack"] = base64;
		std::string message = msg.toStyledString();
		// 发布广播消息
		return publish(exchange, routingKey, message);
	}

	bool RabbitmqClient::publishInner() {
		int id = checkPublish();
		if (id == 0) 
			return false;
		PublishItem::Ptr item;
		amqp_rpc_reply_t reply;
		while (true) {
			item = popPublishItem();
			if (!item)
				break;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(_connection->getState());
			int ret = amqp_basic_publish(state, static_cast<amqp_channel_t>(id),
				amqp_cstring_bytes(item->_exchange.c_str()), amqp_cstring_bytes(item->_routingKey.c_str()),
				0, 0, nullptr, amqp_cstring_bytes(item->_message.c_str()));
			if (ret < 0) {
				ErrorS << "Publish error: " << ret;
				reply = amqp_get_rpc_reply(state);
				ret = RabbitmqUtils::checkReplyError(reply, "Publishing");
				if (ret == 1) {
					_connection->setOk(false);
					return true;
				}
				else if (ret == 2) {
					closeChannel(id);
					setPublishChannel(0);
					break;
				}
			}
		}
		return false;
	}

	std::shared_ptr<PublishItem> RabbitmqClient::popPublishItem() {
		std::lock_guard<std::mutex> lck(_mtxPublish);

		if (_publishItems.empty())
			return nullptr;

		PublishItem::Ptr item = _publishItems.front();
		_publishItems.pop();
		return item;
	}

	int RabbitmqClient::getPublishChannel() {
		std::lock_guard<std::mutex> lck(_mtx);

		return _publishId;
	}

	void RabbitmqClient::setPublishChannel(int id) {
		std::lock_guard<std::mutex> lck(_mtx);

		_publishId = id;
	}

	int RabbitmqClient::checkPublish() {
		int id = getPublishChannel();
		if (id != 0)
			return id;
		id = openChannel();
		setPublishChannel(id);
		return id;
	}

	static int consumeLibraryException(amqp_connection_state_t state, const amqp_rpc_reply_t& reply) {
		amqp_frame_t frame;
		if (AMQP_STATUS_UNEXPECTED_STATE == reply.library_error) {
			if (AMQP_STATUS_OK != amqp_simple_wait_frame(state, &frame))
				return 0;
			if (AMQP_FRAME_METHOD == frame.frame_type) {
				switch (frame.payload.method.id) {
				case AMQP_BASIC_ACK_METHOD:
					/* if we've turned publisher confirms on, and we've
					 * published a message here is a message being confirmed.
					 */
					break;
				case AMQP_BASIC_RETURN_METHOD:
					/* if a published message couldn't be routed and the
					 * mandatory flag was set this is what would be returned.
					 * The message then needs to be read.
					 */
					{
						amqp_message_t message;
						amqp_rpc_reply_t tmp = amqp_read_message(state, frame.channel, &message, 0);
						if (AMQP_RESPONSE_NORMAL != tmp.reply_type)
							return 0;
						amqp_destroy_message(&message);
					}
					break;
				case AMQP_CHANNEL_CLOSE_METHOD:
					/* a channel.close method happens when a channel exception
					 * occurs, this can happen by publishing to an exchange that
					 * doesn't exist for example.
					 *
					 * In this case you would need to open another channel
					 * redeclare any queues that were declared auto-delete, and
					 * restart any consumers that were attached to the previous
					 * channel.
					 */
					LOG_ERROR("Channel close.");
					return 2;
				case AMQP_CONNECTION_CLOSE_METHOD:
					/* a connection.close method happens when a connection
					 * exception occurs, this can happen by trying to use a
					 * channel that isn't open for example.
					 *
					 * In this case the whole connection must be restarted.
					 */
					LOG_ERROR("Connection close.");
					return 1;
				case AMQP_CHANNEL_OPEN_OK_METHOD:
					return 0;
				default:
					{
						char tmp[16] = { '\0' };
						snprintf(tmp, sizeof(tmp), "%08X", frame.payload.method.id);
						ErrorS << "An unexpected method was received " << tmp;
					}
					break;
				}
			}
		}
		else if (AMQP_STATUS_CONNECTION_CLOSED == reply.library_error ||
			AMQP_STATUS_SOCKET_ERROR == reply.library_error ||
			AMQP_STATUS_SOCKET_CLOSED == reply.library_error) {
			LOG_ERROR("Connection close.");
			return 1;
		}
		return 0;
	}

	static int consumeServerException(amqp_connection_state_t state, const amqp_rpc_reply_t& reply) {
		if (AMQP_CONNECTION_CLOSE_METHOD == reply.reply.id) {
			LOG_ERROR("Connection close.");
			return 1;
		}
		else if (AMQP_CHANNEL_CLOSE_METHOD == reply.reply.id) {
			LOG_ERROR("Channel close.");
			return 2;
		}
		return 0;
	}

	void RabbitmqClient::threadFunc() {
		amqp_rpc_reply_t reply;
		amqp_envelope_t envelope;
		std::shared_ptr<RabbitmqConnectionImpl> impl = std::dynamic_pointer_cast<RabbitmqConnectionImpl>(_connection);
		struct timeval tv = { 0, 10000 };
		int count = 0;
		while (!_stopFlag) {
			if (!(_connection->isOk())) {
				// 等待100毫秒
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				count++;
				if (count > 49) {
					count = 0;
					checkConnection();
				}
				continue;
			}
			count = 0;
			if (publishInner())
				continue;

			amqp_maybe_release_buffers(impl->_state);
			reply = amqp_consume_message(impl->_state, &envelope, &tv, 0);
			if (AMQP_RESPONSE_NORMAL == reply.reply_type) {
				std::shared_ptr<std::string> message = std::make_shared<std::string>((const char*)envelope.message.body.bytes, envelope.message.body.len);
				std::string tag((const char*)envelope.consumer_tag.bytes, envelope.consumer_tag.len);
				// 发送ACK
				amqp_basic_ack(impl->_state, envelope.channel, envelope.delivery_tag, 0);
				RabbitmqConsumer::getSingleton().consume(message, tag);
				amqp_destroy_envelope(&envelope);
			}
			else if (AMQP_RESPONSE_LIBRARY_EXCEPTION == reply.reply_type) {
				int ret = consumeLibraryException(impl->_state, reply);
				if (ret != 0) {
					ErrorS << "Consume message error: " << amqp_error_string2(reply.library_error);
					_connection->setOk(false);
				}
			}
			else if (AMQP_RESPONSE_SERVER_EXCEPTION == reply.reply_type) {
				int ret = consumeServerException(impl->_state, reply);
				if (ret != 0)
					_connection->setOk(false);
			}
		}
	}
}