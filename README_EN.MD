**Read this in other languages: [English](README_EN.MD), [中文](README.MD).**

# Testing client

A testing client is available on the Internet, every one is welcomed to download and test it.\
[client](http://106.55.181.89:8896/download)

# Introduction

NiuMa game server is a card game server base on C++ language development, implement the card game backend logic, such as mahjong game function logic and flow control. Clients communicate with this server on TCP connections, using MessagePack serialization solution to transport messages. The dependent third part components of this project include: MySql, Redis and RabbitMQ, this server is cross-platform deployment supported, lightweight design reduces third part dependence when deploying. Based on the overall distributed architecture design, the server can flexibly expand it's instances according to the business volume. A major feature of the design of this server is that the functional modules are concise, the code logic is clear, the core module is highly abstract and reusable, and it is easy for developers to master and carry out function expansion and maintenance.

# Core Design

The core modules of this server include: Network IO, Thread model and Message distribution & processing, these three core modules fundamentally affect the overall concurrency performance of the server. The network IO module is base on boost::asio framework, which is an a popular and powerful network library. As shown in the following figure, a TCP session represents a client-server network connection, we define a complete game logic unit as a game Venue, for example a card game table is a game venue, each game venue has a globally unique ID, and a thread pool dedicated to processing the internal logic of the venue is created during initialization. Each thread in the thread pool contains a set number of message handlers. All message handlers in the system will be registered in a singleton Message Handler list. When a new game venue is created, the venue will be allocated to the message handler with the lightest load. Thus, a message handler maintains a list of venues, so that all game logic and message processing for a venue is executed in the same thread, which avoids the problem of multi-threaded data synchronization and complex error-prone asynchronous calls. When a TCP session receives a message, the network receiving thread traverses the global list of message handler. Once one of the message handlers in the list receives the message, the traversal ends. Because all venue internal messages carry the venue ID, which can be used for hash table matching, so traversal is a very fast process.\
![core design](https://github.com/niuma-wj/data/blob/main/core.png?raw=true)

# System back-end architecture design

The system contains two kinds of business logic with different processing modes. One is the above mentioned game function business logic, which is processed by C++ game server. The other is the login and the game main interface (game hall) business logic, processed by the WEB server, and communicate through HTTP Restful interface, while the WEB server is also responsible for the game venue allocation according to the load of the game server instances, for example when a player creates a new card game room. The creation request is sent to the WEB server, which allocates the newly built room to the game server instance with the lightest load of all game server instances, and returns the connection information (IP and port) of the game server instance to the client. Then the client establishes a TCP connection with the game server to complete entering the card game room, then start the game business logic.

The back-end of the system adopts a distributed architecture design, and the WEB server makes load decisions based on the global Redis data record. The game server instance updates it's current load situation to the global Redis database in real time. Each WEB server instance can make load decisions independently, so the system does not need centralized control center and routing server instances. The game server and WEB server can be flexibly deployed according to the actual business volume, and there is no need to worry about single node performance bottleneck. The WEB server instance and game server instance communicate with each other through RabbitMQ, and the query scheduling between two type of servers can be simple, orderly, and efficient.\
![Framwork](https://github.com/niuma-wj/data/blob/main/Framework.png?raw=true)

# Compilation(Ubuntu environment)

First, ensure that cmake 3.30 or above is installed on your system, and follow these steps to compile the game server:

1. Compile and install the boost library (refer to the online tutorial)\
   Download any version of the code from the boost official website, such as: boost_1_75_0.tar.gz. don't download 1.86 and later versions of boost, because new versions of boost have significant modifications on process module, and that makes link error.

```
tar -xzvf boost_1_75_0.tar.gz
./bootstrap.sh --prefix=/usr/local/boost_1_75_0
./b2 install --toolset=gcc --without-python link=static runtime-link=shared threading=multi variant=release address-model=64 -j4
```

2. Compile and install rabbitmq-c

```rabbitmq-c
git clone https://github.com/alanxz/rabbitmq-c.git
mkdir build
cd build
cmake ..
make
sudo make install
```

3. Install mysql-concpp

```
Official website download address: https://dev.mysql.com/downloads/connector/cpp
Select Linux-Generic，and download "Linux - Generic (glibc 2.28) (x86, 64-bit), Compressed TAR Archive"
Decompress the TAR archive: tar -xzvf mysql-connector-c++-9.2.0-linux-glibc2.28-x86-64bit.tar.gz
Make directory: "/usr/local/lib/cmake/mysql-concpp-9.2.0"
Copy "mysql-concpp-config.cmake" and "mysql-concpp-config-version.cmake" to the new created path "/usr/local/lib/cmake/mysql-concpp-9.2.0"
Copy all the files under the path "lib64" in the TAR archive to the path "/usr/local/lib"
Make directory "/usr/include/mysql-cppconn", and copy all the files under the path "include" in the TAR archive to the new created path "/usr/include/mysql-cppconn"
```

4. Compile game server

```
mkdir build
mkdir bin
mkdir bin/log
cd build
cmake ..
make
cd Server
cp Server ../../bin/
copy the configuration file "server.ini" to the new created path "bin", then modify the configuration information according to the format
```

# Contact

393817707@qq.com
