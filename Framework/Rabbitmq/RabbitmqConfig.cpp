// RabbitmqConfig.cpp

#include "Base/Log.h"
#include "RabbitmqClient.h"
#include "RabbitmqUtils.h"

namespace NiuMa {
	class ConfigItem {
	public:
		ConfigItem(bool redo)
			: _redo(redo)
		{}

		virtual ~ConfigItem() = default;

		typedef std::shared_ptr<ConfigItem> Ptr;

	public:
		/**
		 * 执行配置
		 * @param conn rabbitmq 连接
		 * @param channel 通道id，若执行配置出错导致通道关闭，则置0
		 * @return 是否配置成功
		 */
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) = 0;

		/**
		 * 是否用独占的通道来进行配置
		 * 有些配置，例如订阅需要用独占的通道进行配置，避免在完成配置之后通道后面用于其他调用发生错误导致通道关闭，进而导致配置失效
		 */
		virtual bool exclusiveChannel() { return false; }

		// 查询在断线重连后是否需要重新配置
		bool redo() { return _redo; }

	private:
		// 断线重连后是否需要重新配置
		bool _redo;
	};
	
#define CHECK_REPLY_ERROR(reply, context)	\
	int ret = RabbitmqUtils::checkReplyError(reply, context);	\
	if (ret == 0)	\
		return true;	\
	if (ret == 1) {	\
		conn->setOk(false);	\
		channel = 0;	\
	} else if (ret == 2)	\
		channel = 0;	\
	return false
		

	class ExchangeDeclare : public ConfigItem {
	public:
		ExchangeDeclare(const std::string& exchange, const std::string& type, bool durable, bool autoDelete, bool internal_, bool redo)
			: ConfigItem(redo)
			, _exchange(exchange)
			, _type(type)
			, _durable(durable)
			, _autoDelete(autoDelete)
			, _internal(internal_)
		{}

		virtual ~ExchangeDeclare() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;

			const amqp_table_t emptyTable = { 0, nullptr };
			/*const amqp_bytes_t emptyBytes = {0, nullptr};
			amqp_exchange_declare(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_exchange.c_str()), amqp_cstring_bytes(_type.c_str()), 1, 0, 0, 0, emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			if (AMQP_RESPONSE_NORMAL == reply.reply_type)
				return true;	// 交换机已存在，不再声明*/
			amqp_exchange_declare(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_exchange.c_str()), amqp_cstring_bytes(_type.c_str()),
				0, _durable ? 1 : 0, _autoDelete ? 1 : 0, _internal ? 1 : 0, emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Declaring exchange");
		}

	private:
		std::string _exchange;
		std::string _type;
		bool _durable;
		bool _autoDelete;
		bool _internal;
	};

	void RabbitmqConfig::exchangeDeclare(const std::string& exchange, const std::string& type, bool durable, bool autoDelete, bool internal_, bool redo) {
		ConfigItem::Ptr item = std::make_shared<ExchangeDeclare>(exchange, type, durable, autoDelete, internal_, redo);
		_items.push_back(item);
	}

	class ExchangeBind : public ConfigItem {
	public:
		ExchangeBind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo)
			: ConfigItem(redo)
			, _destination(destination)
			, _source(source)
			, _routingKey(routingKey)
		{}

		virtual ~ExchangeBind() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			amqp_exchange_bind(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_destination.c_str()), amqp_cstring_bytes(_source.c_str()),
				amqp_cstring_bytes(_routingKey.c_str()), emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Binding exchange");
		}

	private:
		std::string _destination;
		std::string _source;
		std::string _routingKey;
	};

	void RabbitmqConfig::exchangeBind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo) {
		ConfigItem::Ptr item = std::make_shared<ExchangeBind>(destination, source, routingKey, redo);
		_items.push_back(item);
	}

	class ExchangeUnbind : public ConfigItem {
	public:
		ExchangeUnbind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo)
			: ConfigItem(redo)
			, _destination(destination)
			, _source(source)
			, _routingKey(routingKey)
		{}

		virtual ~ExchangeUnbind() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			amqp_exchange_unbind(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_destination.c_str()), amqp_cstring_bytes(_source.c_str()),
				amqp_cstring_bytes(_routingKey.c_str()), emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Unbinding exchange");
		}

	private:
		std::string _destination;
		std::string _source;
		std::string _routingKey;
	};

	void RabbitmqConfig::exchangeUnbind(const std::string& destination, const std::string& source, const std::string& routingKey, bool redo) {
		ConfigItem::Ptr item = std::make_shared<ExchangeUnbind>(destination, source, routingKey, redo);
		_items.push_back(item);
	}

	class ExchangeDelete : public ConfigItem {
	public:
		ExchangeDelete(const std::string& exchange, bool ifUnused, bool redo)
			: ConfigItem(redo)
			, _exchange(exchange)
			, _ifUnused(ifUnused)
		{}

		virtual ~ExchangeDelete() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			amqp_exchange_delete(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_exchange.c_str()), _ifUnused ? 1 : 0);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Deleting exchange");
		}

	private:
		std::string _exchange;
		bool _ifUnused;
	};

	void RabbitmqConfig::exchangeDelete(const std::string& exchange, bool ifUnused, bool redo) {
		ConfigItem::Ptr item = std::make_shared<ExchangeDelete>(exchange, ifUnused, redo);
		_items.push_back(item);
	}

	class QueueDeclare : public ConfigItem {
	public:
		QueueDeclare(const std::string& queue, bool durable, bool exclusive, bool autoDelete, bool redo)
			: ConfigItem(redo)
			, _queue(queue)
			, _durable(durable)
			, _exclusive(exclusive)
			, _autoDelete(autoDelete)
		{}

		virtual ~QueueDeclare() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			/*amqp_queue_declare(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()), 1, 0, 0, 0, emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			if (AMQP_RESPONSE_NORMAL == reply.reply_type)
				return true;	// 队列已存在*/
			amqp_queue_declare(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()),
				0, _durable ? 1 : 0, _exclusive ? 1 : 0, _autoDelete ? 1 : 0, emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Declaring queue");
		}

	private:
		std::string _queue;
		bool _durable;
		bool _exclusive;
		bool _autoDelete;
	};

	void RabbitmqConfig::queueDeclare(const std::string& queue, bool durable, bool exclusive, bool autoDelete, bool redo) {
		ConfigItem::Ptr item = std::make_shared<QueueDeclare>(queue, durable, exclusive, autoDelete, redo);
		_items.push_back(item);
	}

	class QueueBind : public ConfigItem {
	public:
		QueueBind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo)
			: ConfigItem(redo)
			, _queue(queue)
			, _exchange(exchange)
			, _routingKey(routingKey)
		{}

		virtual ~QueueBind() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			amqp_queue_bind(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()), amqp_cstring_bytes(_exchange.c_str()),
				amqp_cstring_bytes(_routingKey.c_str()), emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Binding queue");
		}

	private:
		std::string _queue;
		std::string _exchange;
		std::string _routingKey;
	};

	void RabbitmqConfig::queueBind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo) {
		ConfigItem::Ptr item = std::make_shared<QueueBind>(queue, exchange, routingKey, redo);
		_items.push_back(item);
	}

	class QueueUnbind : public ConfigItem {
	public:
		QueueUnbind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo)
			: ConfigItem(redo)
			, _queue(queue)
			, _exchange(exchange)
			, _routingKey(routingKey)
		{}

		virtual ~QueueUnbind() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			amqp_queue_unbind(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()), amqp_cstring_bytes(_exchange.c_str()),
				amqp_cstring_bytes(_routingKey.c_str()), emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Ubinding queue");
		}

	private:
		std::string _queue;
		std::string _exchange;
		std::string _routingKey;
	};

	void RabbitmqConfig::queueUnbind(const std::string& queue, const std::string& exchange, const std::string& routingKey, bool redo) {
		ConfigItem::Ptr item = std::make_shared<QueueUnbind>(queue, exchange, routingKey, redo);
		_items.push_back(item);
	}

	class QueueDelete : public ConfigItem {
	public:
		QueueDelete(const std::string& queue, bool ifUnused, bool ifEmpty, bool redo)
			: ConfigItem(redo)
			, _queue(queue)
			, _ifUnused(ifUnused)
			, _ifEmpty(ifEmpty)
		{}

		virtual ~QueueDelete() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			amqp_queue_delete(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()), _ifUnused ? 1 : 0, _ifEmpty ? 1 : 0);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Deleting queue");
		}

	private:
		std::string _queue;
		bool _ifUnused;
		bool _ifEmpty;
	};

	void RabbitmqConfig::queueDelete(const std::string& queue, bool ifUnused, bool ifEmpty, bool redo) {
		ConfigItem::Ptr item = std::make_shared<QueueDelete>(queue, ifUnused, ifEmpty, redo);
		_items.push_back(item);
	}

	class QueueConsume : public ConfigItem {
	public:
		QueueConsume(const std::string& queue, const std::string& tag, bool noLocal, bool noAck, bool exclusive, bool redo)
			: ConfigItem(redo)
			, _queue(queue)
			, _tag(tag)
			, _noLocal(noLocal)
			, _noAck(noAck)
			, _exclusive(exclusive)
		{}

		virtual ~QueueConsume() {}

	public:
		virtual bool config(const RabbitmqConnection::Ptr& conn, int& channel) override {
			if (!conn || !(conn->isOk()) || (channel == 0))
				return false;
			amqp_connection_state_t state = reinterpret_cast<amqp_connection_state_t>(conn->getState());
			if (state == nullptr)
				return false;
			const amqp_table_t emptyTable = { 0, nullptr };
			amqp_basic_consume(state, static_cast<amqp_channel_t>(channel),
				amqp_cstring_bytes(_queue.c_str()), amqp_cstring_bytes(_tag.c_str()),
				_noLocal ? 1 : 0, _noAck ? 1 : 0, _exclusive ? 1 : 0, emptyTable);
			amqp_rpc_reply_t reply = amqp_get_rpc_reply(state);
			CHECK_REPLY_ERROR(reply, "Consuming");
		}

		virtual bool exclusiveChannel() override { return true; }

	private:
		std::string _queue;
		std::string _tag;
		bool _noLocal;
		bool _noAck;
		bool _exclusive;
	};

	void RabbitmqConfig::queueConsume(const std::string& queue, const std::string& tag, bool noLocal, bool noAck, bool exclusive, bool redo) {
		ConfigItem::Ptr item = std::make_shared<QueueConsume>(queue, tag, noLocal, noAck, exclusive, redo);
		_items.push_back(item);
	}

	bool RabbitmqConfig::config(const RabbitmqConnection::Ptr& conn) {
		if (_items.empty())
			return false;
		bool ret = true;
		bool test = false;
		int channel = 0;
		int sharedChannel = 0;
		std::list<std::shared_ptr<ConfigItem> >::iterator it;
		it = _items.begin();
		while (it != _items.end()) {
			ConfigItem::Ptr& config = *it;
			test = config->exclusiveChannel();
			if (test)
				channel = RabbitmqClient::getSingleton().openChannel();
			else {
				if (sharedChannel == 0)
					sharedChannel = RabbitmqClient::getSingleton().openChannel();
				channel = sharedChannel;
			}
			if (!config->config(conn, channel))
				ret = false;
			if (!(conn->isOk()))
				break;
			if (!test)
				sharedChannel = channel;
			if (config->redo())
				++it;
			else
				it = _items.erase(it);
		}
		return ret;
	}
}