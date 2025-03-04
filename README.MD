# NiuMa游戏平台游戏服务器(C++)
本服务是使用C++开发的棋牌游戏逻辑服务器，实现棋牌游戏的后台服务逻辑，例如麻将游戏功能逻辑与流程
控制等。客户端与本服务建立TCP连接通信，并通过MessagePack序列化方案进行消息交互。本服务依赖的第
三方组件包括：MySql 8.0+、Redis以及RabbitMQ，本服务支持跨平台部署，轻量化设计使得在部署时无需
安装太多依赖，基于整体分布式架构设计，本服务器可以根据业务情况进行服务实例弹性横向扩展。本服务设
计的一大特点是功能模块简洁明了，代码逻辑思路清晰，核心模块高度抽象和可复用，开发者很容易掌握并进
行功能扩展和维护。

# 整体架构设计
系统包含两种不同处理方式的业务逻辑，一种为上述的游戏功能业务逻辑，由C++游戏服务器进行处理；另一种
则是登录及游戏主界面(游戏大厅)的业务逻辑，由WEB服务器进行处理，通过HTTP Restful接口进行通信，同
时WEB还负责根据游戏服务实例的负载情况进行游戏场地分配，例如当玩家新建一个斗地主游戏房间时，会把创
建请求发送到WEB服务器，WEB根据所有游戏服务实例的负载情况，将新建的房间分配到当前负债最轻的游戏服
务实例，并向客户端返回该游戏服务实例的连接信息(IP、端口)，客户端再与该游戏服务实例建立TCP连接，从
而完成进入该游戏房间，并开始游戏业务逻辑。

系统后台服务采用分布式架构设计，由WEB服务基于全局Redis的数据记录进行负载决策，游戏服务实例实时更
新本节点当前的负载情况到全局Redis数据库中，每个WEB服务实例都可以独自进行负载决策，从而使得系统不
需要统一控制的中心和路由服务实例，使得游戏服务和WEB服务可以根据业务量进行灵活节点部署，同时也不需
要担心单节点性能瓶颈问题。WEB服务实例与游戏服务实例之间通过RabbitMQ进行交互通信，使服务器节点之间
的调度可以简单、有序及高效地进行。\
下图为系统后台整体架构设计概图\
![架构图](https://gitee.com/friedrich-hegel/data/raw/master/Framework.png)

# 编译(Ubuntu环境)
首先确保系统已经安装了cmake 3.30版本以上，按照以下步骤编译游戏服务：

1、安装编译rabbitmq-c
```rabbitmq-c
git clone https://github.com/alanxz/rabbitmq-c.git
mkdir build
cd build
cmake ..
make
sudo make install
```

2、安装mysql-concpp
```
官网下载地址：https://dev.mysql.com/downloads/connector/cpp
选择Linux-Generic，下载Linux - Generic (glibc 2.28) (x86, 64-bit), Compressed TAR Archive
解压缩包：tar -xzvf mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit.tar.gz
创建目录：/usr/local/lib/cmake/mysql-concpp-9.2.0
将mysql-concpp-config.cmake和mysql-concpp-config-version.cmake文件复制到/usr/local/lib/cmake/mysql-concpp-9.2.0目录
将压缩包中lib64目录下的全部文件复制到/usr/local/lib目录
创建目录/usr/include/mysql-cppconn，并将压缩包中include目录下的全部文件复制到/usr/include/mysql-cppconn目录
```

3、编译游戏服务
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