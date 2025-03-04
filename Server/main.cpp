// main.cpp

#include <iostream>
#include <signal.h>
#include <chrono>
#include <thread>
#include "Base/Log.h"
#include "Base/IniConfig.h"
#include "Network/TcpServer.h"
#include "Network/MsgSession.h"
#include "Network/SecurityManager.h"
#include "Message/MessageThreadPool.h"
#include "Timer/TimerManager.h"
#include "MySql/MysqlPool.h"
#include "Redis/RedisPool.h"
#include "Constant/RedisKeys.h"
#include "Rabbitmq/RabbitmqClient.h"
#include "Rabbitmq/RabbitmqConsumer.h"
#include "Player/PlayerManager.h"
#include "Venue/VenueManager.h"
#include "Venue/VenueOuterHandler.h"
#include "StandardMahjong/StandardMahjongRoomHandler.h"
#include "StandardMahjong/StandardMahjongMessages.h"
#include "StandardMahjong/StandardMahjongLoader.h"
#include "BiJi/BiJiRoomHandler.h"
#include "BiJi/BiJiMessages.h"
#include "BiJi/BiJiLoader.h"
#include "Lackey/LackeyRoomHandler.h"
#include "Lackey/LackeyLoader.h"
#include "Lackey/LackeyMessages.h"
#include "NiuNiu100/NiuNiu100RoomHandler.h"
#include "NiuNiu100/NiuNiu100Loader.h"
#include "NiuNiu100/NiuNiu100Messages.h"
#include "Game/DebtLiquidation.h"

#include "Example.h"


std::atomic<bool> exitFlag(true);

void signalHandler(int num) {
    exitFlag = false;
}

void debt() {
    NiuMa::DebtNode* nodes[5] = { nullptr };
    std::unordered_map<int, NiuMa::DebtNode*> debtNet;
    for (int i = 0; i < 5; i++) {
        nodes[i] = new NiuMa::DebtNode(i, 0.0);
        debtNet.insert(std::make_pair(i, nodes[i]));
    }
    nodes[0]->tally(nodes[1], 150.0);
    nodes[0]->tally(nodes[3], 100.0);
    nodes[1]->tally(nodes[0], -150.0);
    nodes[1]->tally(nodes[2], -150.0);
    nodes[1]->tally(nodes[3], -50.0);
    nodes[1]->tally(nodes[4], -150.0);
    nodes[2]->tally(nodes[1], 150.0);
    nodes[2]->tally(nodes[3], 100.0);
    nodes[3]->tally(nodes[0], -100.0);
    nodes[3]->tally(nodes[1], 50.0);
    nodes[3]->tally(nodes[2], -100.0);
    nodes[3]->tally(nodes[4], -100.0);
    nodes[4]->tally(nodes[1], 150.0);
    nodes[4]->tally(nodes[3], 100.0);
    std::string logDebt;
    NiuMa::DebtLiquidation dl;
    dl.printDebtNet(debtNet, logDebt);
    if (!dl(debtNet)) {
        dl.releaseDebtNet(debtNet);
        LOG_ERROR("清算结果不正确，原始债务网如下:");
        LOG_ERROR(logDebt);
        return;
    }
    bool test = false;
    for (int i = 0; i < 5; i++) {
        if (nodes[i]->getCapital() < 0.0) {
            test = true;
            break;
        }
    }
    if (test) {
        // 清算完后存在现金为负的节点
        dl.releaseDebtNet(debtNet);
        LOG_ERROR("清算之后存在负数结果，原始债务网如下:");
        LOG_ERROR(logDebt);
        return;
    }
    dl.releaseDebtNet(debtNet);
}

int main(int argc, char* argv[]) {
    try {
        // 设置SIGINT信号的处理函数
        signal(SIGINT, signalHandler);

        // 加载配置文件
        NiuMa::IniConfig::getSingleton().loadIni("./server.ini");

        // 初始化日志
        NiuMa::LogManager::getSingleton().initialize("log/server");

        // 启动异步定时器管理器
        int threadNum = 0;
        NiuMa::IniConfig::getSingleton().getInt("Server", "timer_threads", threadNum);
        NiuMa::TimerManager::getSingleton().start(threadNum);

        // 启动数据库连接池
        std::string host;
        std::string username;
        std::string password;
        std::string schema;
        int keepConnections = 0;
        int maxConnections = 0;
        NiuMa::IniConfig::getSingleton().getString("Mysql", "host", host);
        NiuMa::IniConfig::getSingleton().getString("Mysql", "username", username);
        NiuMa::IniConfig::getSingleton().getString("Mysql", "password", password);
        NiuMa::IniConfig::getSingleton().getString("Mysql", "schema", schema);
        NiuMa::IniConfig::getSingleton().getInt("Mysql", "keep_connections", keepConnections);
        NiuMa::IniConfig::getSingleton().getInt("Mysql", "max_connections", maxConnections);
        NiuMa::IniConfig::getSingleton().getInt("Mysql", "thread_num", threadNum);
        NiuMa::MysqlPool::getSingleton().start(host, username, password, schema, keepConnections, maxConnections, threadNum);
        //NiuMa::MysqlPool::getSingleton().start("tcp://127.0.0.1:3306", "root", "123456", "temp", 2, 10, 2);

        // 启动Redis连接池
        int port = 0;
        int database = 0;
        NiuMa::IniConfig::getSingleton().getString("Redis", "host", host);
        NiuMa::IniConfig::getSingleton().getInt("Redis", "port", port);
        NiuMa::IniConfig::getSingleton().getString("Redis", "password", password);
        NiuMa::IniConfig::getSingleton().getInt("Redis", "database", database);
        NiuMa::IniConfig::getSingleton().getInt("Redis", "keep_connections", keepConnections);
        NiuMa::IniConfig::getSingleton().getInt("Redis", "max_connections", maxConnections);
        NiuMa::RedisPool::getSingleton().start(host, port, password, database, keepConnections, maxConnections);
        //NiuMa::RedisPool::getSingleton().start("127.0.0.1", 6379, "MwTn8NvWf&!", 9, 3, 20);
        //NiuMa::RedisPool::getSingleton().start("116.8.109.23", 10068, "MwTn8NvWf&!", 11, 3, 20);

        /*NiuMa::MysqlQueryTask::Ptr task;
        std::string sql;
        task = std::make_shared<NiuMa::TestSelect>();
        NiuMa::MysqlPool::getSingleton().syncQuery(task);
        task = std::make_shared<NiuMa::TestAutoIncrement>();
        NiuMa::MysqlPool::getSingleton().syncQuery(task);
        sql = "insert into `test`(`field1`, `field2`) values(\"aaa\", \"bbb\")";
        task = std::make_shared<NiuMa::MysqlCommonTask>(sql, NiuMa::MysqlQueryTask::QueryType::Insert);
        NiuMa::MysqlPool::getSingleton().syncQuery(task);
        if (task->getSucceed()) {
            InfoS << "Insert succeed, " << task->getAffectedRecords() << " affected records.";
        }
        else {
            InfoS << "Insert failed.";
        }
        sql = "delete from `test` where `id` = 5";
        task = std::make_shared<NiuMa::MysqlCommonTask>(sql, NiuMa::MysqlQueryTask::QueryType::Delete);
        NiuMa::MysqlPool::getSingleton().syncQuery(task);
        if (task->getSucceed()) {
            InfoS << "Delete succeed, " << task->getAffectedRecords() << " affected records.";
        }
        else {
            InfoS << "Delete failed.";
        }
        task = std::make_shared<NiuMa::TestUpdate>();
        NiuMa::MysqlPool::getSingleton().syncQuery(task);*/

        /*NiuMa::ThreadStopFlag::Ptr flag = std::make_shared<NiuMa::ThreadStopFlag>();
        std::vector<NiuMa::ThreadWorker::Ptr> workers;
        workers.push_back(std::make_shared<NiuMa::TestSelectWorker>(flag));
        NiuMa::ThreadPool::Ptr pool = std::make_shared<NiuMa::ThreadPool>();
        pool->start(flag, workers);*/

        /*NiuMa::TimerManager::getSingleton().addAsyncTimer(1000, []() {
            InfoS << "Timer test.";
            return false;
        });*/
        
        /*bool ret = false;
        bool boolVal = false;
        int64_t intVal = 0;
        std::string strVal;
        std::vector<std::string> strArr;
        std::vector<int64_t> intArr;
        ret = NiuMa::RedisPool::getSingleton().set("test1", "fuck");
        if (ret)
            ret = NiuMa::RedisPool::getSingleton().get("test1", intVal);
        ret = NiuMa::RedisPool::getSingleton().incrBy("test1", 1);
        ret = NiuMa::RedisPool::getSingleton().set("test2", 101);
        ret = NiuMa::RedisPool::getSingleton().incrBy("test2", 1);
        if (ret)
            ret = NiuMa::RedisPool::getSingleton().get("test2", strVal);
        ret = NiuMa::RedisPool::getSingleton().del("test");
        ret = NiuMa::RedisPool::getSingleton().incrBy("test3", 1);
        if (ret)
            ret = NiuMa::RedisPool::getSingleton().get("test3", intVal);
        ret = NiuMa::RedisPool::getSingleton().del("test4");
        ret = NiuMa::RedisPool::getSingleton().rpush("test4", "hello");
        ret = NiuMa::RedisPool::getSingleton().rpush("test4", "bar");
        ret = NiuMa::RedisPool::getSingleton().rpush("test4", "foo");
        strArr.push_back(std::string("zhang3"));
        strArr.push_back(std::string("li4"));
        strArr.push_back(std::string("wang5"));
        intArr.push_back(100);
        intArr.push_back(53);
        intArr.push_back(6);
        ret = NiuMa::RedisPool::getSingleton().rpushv("test4", strArr);
        ret = NiuMa::RedisPool::getSingleton().rpushv("test4", intArr);
        ret = NiuMa::RedisPool::getSingleton().lrange("test4", strArr);
        ret = NiuMa::RedisPool::getSingleton().lrange("test4", intArr);
        ret = NiuMa::RedisPool::getSingleton().hset("test5", "field1", "wujian");
        if (ret)
            ret = NiuMa::RedisPool::getSingleton().hget("test5", "field1", strVal);
        ret = NiuMa::RedisPool::getSingleton().hset("test5", "field2", "world");
        ret = NiuMa::RedisPool::getSingleton().hdel("test5", "field1");
        ret = NiuMa::RedisPool::getSingleton().hincrBy("test5", "field3", 2);
        ret = NiuMa::RedisPool::getSingleton().hset("test6", "field1", "wujian");
        ret = NiuMa::RedisPool::getSingleton().hset("test6", "field2", "wujian");
        ret = NiuMa::RedisPool::getSingleton().hset("test6", "field3", "wujian");
        ret = NiuMa::RedisPool::getSingleton().hset("test6", "field4", "wujian");
        ret = NiuMa::RedisPool::getSingleton().hkeys("test6", strArr);
        ret = NiuMa::RedisPool::getSingleton().hexists("test6", "field4", boolVal);*/

        /*strArr.push_back("wujian");
        strArr.push_back("hello");
        strArr.push_back("world");
        strArr.push_back("rich");
        strArr.push_back("man");
        ret = NiuMa::RedisPool::getSingleton().sadd("test_set1", strArr);
        ret = NiuMa::RedisPool::getSingleton().sismember("test_set1", "world", boolVal);
        ret = NiuMa::RedisPool::getSingleton().sremove("test_set1", "hello");
        strArr.clear();
        ret = NiuMa::RedisPool::getSingleton().smembers("test_set1", strArr);*/

        // 启动RabbitMQ消费者
        NiuMa::RabbitmqConsumer::getSingleton().start();
        // 启动RabbitMQ客户端
        std::string fanoutExchange;
        std::string fanoutQueue;
        std::string fanoutConsumerTag;
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "fanout_exchange", fanoutExchange);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "fanout_queue", fanoutQueue);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "fanout_consumer_tag", fanoutConsumerTag);
        NiuMa::RabbitmqConfig::Ptr config = std::make_shared<NiuMa::RabbitmqConfig>();
        // 消费广播队列消息
        config->queueDeclare(fanoutQueue, false, false, true, true);
        config->queueBind(fanoutQueue, fanoutExchange, std::string(), true);
        config->queueConsume(fanoutQueue, fanoutConsumerTag);
        // 配置定向消费队列
        std::string directExchange;
        std::string directQueue;
        std::string directConsumerTag;
        std::string serverId;
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "direct_exchange", directExchange);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "direct_queue", directQueue);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "direct_consumer_tag", directConsumerTag);
        NiuMa::IniConfig::getSingleton().getString("Server", "server_id", serverId);
        config->queueDeclare(directQueue, false, false, true, true);
        config->queueBind(directQueue, directExchange, serverId, true); // 使用服务器id作为路由键
        config->queueConsume(directQueue, directConsumerTag);
        /*config->exchangeDeclare("exchange_niuma", "direct", false, false, false, true);
        config->queueDeclare("queue_niuma", false, true, true, true);
        config->queueBind("queue_niuma", "exchange_niuma", "niuma", true);
        config->queueConsume("queue_niuma", "tag_niuma");*/
        std::string vhost;
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "host", host);
        NiuMa::IniConfig::getSingleton().getInt("RabbitMQ", "port", port);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "vhost", vhost);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "username", username);
        NiuMa::IniConfig::getSingleton().getString("RabbitMQ", "password", password);
        NiuMa::RabbitmqClient::getSingleton().start(host, port, vhost, username, password, config);
        //NiuMa::RabbitmqClient::getSingleton().start("127.0.0.1", 5672, "test", "guest", "guest", config);
        //NiuMa::RabbitmqClient::getSingleton().start("116.8.109.23", 10171, "my_vhost", "admin", "admin@353", config);
        //NiuMa::TestPrintHandler::Ptr handler = std::make_shared<NiuMa::TestPrintHandler>("tag_niuma");
        //NiuMa::RabbitmqConsumer::getSingleton().addHandler(handler);
        //NiuMa::RabbitmqClient::getSingleton().publish("amq.direct", "test_bind", "test message 1");
        //NiuMa::RabbitmqClient::getSingleton().publish("amq.direct", "test_bind", "test message 2");

        /*std::shared_ptr<NiuMa::TestPublisher> publisher = std::make_shared<NiuMa::TestPublisher>();
        std::weak_ptr<NiuMa::TestPublisher> weakPublisher = publisher;
        NiuMa::TimerManager::getSingleton().addAsyncTimer(1000, [weakPublisher]() {
            std::shared_ptr<NiuMa::TestPublisher> publisher = weakPublisher.lock();
            if (publisher) {
                publisher->publish();
                return false;
            }
            return true;
        });*/

        // 初始化安全管理器
        NiuMa::SecurityManager::getSingleton().init(fanoutExchange, fanoutConsumerTag);

        // 初始化玩家管理器
        NiuMa::PlayerManager::getSingleton().init();

        // 初始化场地管理器
        NiuMa::VenueManager::getSingleton().init(serverId, directExchange, directConsumerTag);

        // 注册场地加载器
        std::shared_ptr<NiuMa::VenueLoader> loader = std::make_shared<NiuMa::StandardMahjongLoader>();
        NiuMa::VenueManager::getSingleton().registLoader(loader);
        loader = std::make_shared<NiuMa::BiJiLoader>();
        NiuMa::VenueManager::getSingleton().registLoader(loader);
        loader = std::make_shared<NiuMa::LackeyLoader>();
        NiuMa::VenueManager::getSingleton().registLoader(loader);
        loader = std::make_shared<NiuMa::NiuNiu100Loader>();
        NiuMa::VenueManager::getSingleton().registLoader(loader);

        // 创建场地外消息处理器线程池
        NiuMa::IniConfig::getSingleton().getInt("Server", "outter_threads", threadNum);
        std::vector<NiuMa::MessageHandler::Ptr> handlers;
        // 场地外消息处理器共享消息队列
        NiuMa::MessageQueue::Ptr queue = std::make_shared<NiuMa::MessageQueue>();
        for (int i = 0; i < threadNum; i++) {
            NiuMa::MessageHandler::Ptr handler = std::make_shared<NiuMa::VenueOuterHandler>(queue);
            handler->registSelf();
            handlers.emplace_back(handler);
        }
        std::shared_ptr<NiuMa::MessageThreadPool> outterPool = std::make_shared<NiuMa::MessageThreadPool>();
        outterPool->start(threadNum, handlers);

        NiuMa::IniConfig::getSingleton().getInt("Server", "inner_threads", threadNum);
        handlers.clear();
        // 创建麻将游戏房间内部网络消息处理器
        std::shared_ptr<NiuMa::VenueInnerHandler> handler;
        for (int i = 0; i < threadNum; i++) {
            handler = std::make_shared<NiuMa::StandardMahjongRoomHandler>();
            handler->registSelf();
            handlers.push_back(handler);
            NiuMa::VenueManager::getSingleton().registHandler(handler);
        }
        // 创建比鸡游戏房间内部网络消息处理器
        for (int i = 0; i < threadNum; i++) {
            handler = std::make_shared<NiuMa::BiJiRoomHandler>();
            handler->registSelf();
            handlers.push_back(handler);
            NiuMa::VenueManager::getSingleton().registHandler(handler);
        }
        // 创建逮狗腿游戏房间内部网络消息处理器
        for (int i = 0; i < threadNum; i++) {
            handler = std::make_shared<NiuMa::LackeyRoomHandler>();
            handler->registSelf();
            handlers.push_back(handler);
            NiuMa::VenueManager::getSingleton().registHandler(handler);
        }
        // 创建百人牛牛游戏房间内部网络消息处理器
        for (int i = 0; i < threadNum; i++) {
            handler = std::make_shared<NiuMa::NiuNiu100RoomHandler>();
            handler->registSelf();
            handlers.push_back(handler);
            NiuMa::VenueManager::getSingleton().registHandler(handler);
        }
        // 创建场地内消息处理器线程池
        std::shared_ptr<NiuMa::MessageThreadPool> innerPool = std::make_shared<NiuMa::MessageThreadPool>();
        innerPool->start(threadNum, handlers);

        // 注册网络消息创建器
        NiuMa::PlayerMessages::registMessages();
        NiuMa::VenueMessages::registMessages();
        NiuMa::GameMessages::registMessages();
        NiuMa::MahjongMessages::registMessages();
        NiuMa::StandardMahjongMessages::registMessages();
        NiuMa::BiJiMessages::registMessages();
        NiuMa::LackeyMessages::registMessages();
        NiuMa::NiuNiu100Messages::registMessages();

        // 启动TCP消息服务器
        NiuMa::SessionCreator::Ptr creator = std::make_shared<NiuMa::MsgSession::Creator>(30);
        std::shared_ptr<NiuMa::TcpServer> server = std::make_shared<NiuMa::TcpServer>(creator);
        NiuMa::IniConfig::getSingleton().getInt("Server", "port", port);
        NiuMa::IniConfig::getSingleton().getInt("Server", "thread_num", threadNum);
        server->start(port, threadNum);

        // 向Redis注册服务器自身
        NiuMa::RedisPool::getSingleton().sadd(NiuMa::RedisKeys::VENUE_SERVER_SET, serverId);
        std::string redisKey = NiuMa::RedisKeys::SERVER_ACCESS_ADDRESS + serverId;
        std::string accessAddress;
        NiuMa::IniConfig::getSingleton().getString("Server", "access_address", accessAddress);
        NiuMa::RedisPool::getSingleton().set(redisKey, accessAddress);
        // 定时更新保活时间
        NiuMa::TimerManager::getSingleton().addAsyncTimer(3000, [serverId]() {
            std::string redisKey = NiuMa::RedisKeys::SERVER_KEEP_ALIVE + serverId;
            time_t nowTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            NiuMa::RedisPool::getSingleton().set(redisKey, nowTime);
            return false;
            });

        LOG_INFO("Server startup succeed.");

        while (exitFlag) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        //publisher.reset();
        // 从Redis取消注册自身
        NiuMa::RedisPool::getSingleton().sremove(NiuMa::RedisKeys::VENUE_SERVER_SET, serverId);
        NiuMa::RedisPool::getSingleton().del(redisKey);
        redisKey = NiuMa::RedisKeys::SERVER_KEEP_ALIVE + serverId;
        NiuMa::RedisPool::getSingleton().del(redisKey);

        // 关闭Rabbitmq客户端
        NiuMa::RabbitmqClient::getSingleton().stop(nullptr);

        // 关闭线程池
        //pool->stop();
        //pool.reset();

        // 关闭TCP消息服务器
        server->stop();
        server.reset();

        // 关闭场地内消息处理器线程池
        innerPool->stop();
        innerPool.reset();

        // 关闭场地外消息处理器线程池
        outterPool->stop();
        outterPool.reset();

        // 关闭定时器管理器
        NiuMa::TimerManager::getSingleton().stop();

        // 关闭Redis连接池
        NiuMa::RedisPool::getSingleton().stop();

        // 关闭数据库连接池
        NiuMa::MysqlPool::getSingleton().stop();

        // 关闭日志系统
        NiuMa::LogManager::getSingleton().stop();

        // 释放单例实例
        NiuMa::VenueManager::deinstantiate();
        NiuMa::PlayerManager::deinstantiate();
        NiuMa::SecurityManager::deinstantiate();
        NiuMa::RabbitmqClient::deinstantiate();
        NiuMa::RabbitmqConsumer::deinstantiate();
        NiuMa::RedisPool::deinstantiate();
        NiuMa::MysqlPool::deinstantiate();
        NiuMa::TimerManager::deinstantiate();
        NiuMa::LogManager::deinstantiate();

        LOG_INFO("Server stop.");
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}