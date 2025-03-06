**其他语言版本: [English](README.MD), [中文](README_ZH.MD).**

# NiuMa游戏平台游戏服务器(C++)
本项目是使用C++开发的棋牌游戏逻辑服务器，实现棋牌游戏的后台服务逻辑，例如麻将游戏功能逻辑与流程控制等。客户端与本服务建立TCP连接通信，并通过MessagePack序列化方案进行消息交互。本服务依赖的第三方组件包括：MySql 8.0+、Redis以及RabbitMQ，本服务支持跨平台部署，轻量化设计使得在部署时无需安装太多依赖，基于整体分布式架构设计，本服务器可以根据业务情况进行服务实例弹性横向扩展。本服务设计的一大特点是功能模块简洁明了，代码逻辑思路清晰，核心模块高度抽象和可复用，开发者很容易掌握并进行功能扩展和维护。

# 游戏服务器核心设计
游戏服务器的核心模块包括：网络IO、线程模型、消息分发及处理，这三个核心模块从根本上影响着服务器整体并发性能。本项目网络IO使用的是boost::asio框架，如下图所示，一个TCP会话(Session)代表着一个客户端与服务器的TCP网络连接，这里将一个完整的游戏逻辑单元定义为一个游戏场地(Venue)，例如一张棋牌桌就是一个游戏场地，每个游戏场地有全局唯一ID，初始化时创建一个专门处理场地内部逻辑的线程池，线程池中每个线程包含设定数量的消息处理器(Message Handler)，系统中所有消息处理器都会注册到一个单例的消息处理器列表中，当新建一个游戏场地时，该场地会根据负载情况分配到当前负载最轻的消息处理器，因此一个消息处理器维护了一个场地列表，由此一个场地的所有游戏逻辑和消息处理都在同一个线程内执行，这避免了多线程的数据同步问题，以及复杂容易出错的异步调用问题。当TCP会话收到一个消息时，网络接收线程遍历消息处理器列表，一旦列表中有一个消息处理器接收该消息即结束遍历，因为所有场地内的消息都携带了场地ID字段，可以用于哈希表匹配，所以遍历是一个非常快速的一个过程。\
![核心设计](https://gitee.com/friedrich-hegel/data/raw/master/core.png)

# 系统后端架构设计
系统包含两种不同处理方式的业务逻辑，一种为上述的游戏功能业务逻辑，由C++游戏服务器进行处理；另一种则是登录及游戏主界面(游戏大厅)的业务逻辑，由WEB服务器进行处理，通过HTTP Restful接口进行通信，同时WEB服务器还负责根据游戏服务实例的负载情况进行游戏场地分配，例如当玩家新建一个斗地主游戏房间时，会把创建请求发送到WEB服务器，WEB根据所有游戏服务实例的负载情况，将新建的房间分配到当前负载最轻的游戏服务实例，并向客户端返回该游戏服务实例的连接信息(IP、端口)，客户端再与该游戏服务实例建立TCP连接，从而完成进入该游戏房间，并开始游戏业务逻辑。

系统后端采用分布式架构设计，由WEB服务基于全局Redis的数据记录进行负载决策，游戏服务实例实时更新它的负载情况到全局Redis数据库中，每个WEB服务实例都可以独自进行负载决策，因此系统中不需要统一控制的中心和路由服务器实例，并使得游戏服务和WEB服务可以根据实际业务量进行灵活节点部署，同时也不需要担心单节点性能瓶颈问题。WEB服务实例与游戏服务实例之间通过RabbitMQ进行交互通信，服务器之间的查询调度可以简单、有序且高效地进行。\
![架构图](https://gitee.com/friedrich-hegel/data/raw/master/Framework.png)

# 编译(Ubuntu环境)
首先确保系统已经安装了cmake 3.30版本以上，按照以下步骤编译游戏服务：

1、编译安装boost库(具体可参考网上教程)

2、安装编译rabbitmq-c
```rabbitmq-c
git clone https://github.com/alanxz/rabbitmq-c.git
mkdir build
cd build
cmake ..
make
sudo make install
```

3、安装mysql-concpp
```
官网下载地址：https://dev.mysql.com/downloads/connector/cpp
选择Linux-Generic，下载Linux - Generic (glibc 2.28) (x86, 64-bit), Compressed TAR Archive
解压缩包：tar -xzvf mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit.tar.gz
创建目录：/usr/local/lib/cmake/mysql-concpp-9.2.0
将mysql-concpp-config.cmake和mysql-concpp-config-version.cmake文件复制到/usr/local/lib/cmake/mysql-concpp-9.2.0目录
将压缩包中lib64目录下的全部文件复制到/usr/local/lib目录
创建目录/usr/include/mysql-cppconn，并将压缩包中include目录下的全部文件复制到/usr/include/mysql-cppconn目录
```

4、编译游戏服务
```
mkdir build
mkdir bin
mkdir bin/log
cd build
cmake ..
make
cd Server
cp Server ../../bin/
```
将server.ini配置文件也复制到bin目录，并按照格式修改相关配置信息

# 联系方式
393817707@qq.com