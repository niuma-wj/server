// Connection.h

#ifndef _NIU_MA_CONNECTION_H_
#define _NIU_MA_CONNECTION_H_

#include <memory>
#include <string>

namespace NiuMa {
	/**
	 * TCP网络连接接口
	 * 该接口为框架内部接口，不要暴露到外部上层业务逻辑!!!
	 *
	 * @Author wujian
	 * @Email 393817707@qq.com
	 * @Date 2024.07.17
	 */
	class Connection : public std::enable_shared_from_this<Connection>
	{
	public:
		Connection();
		virtual ~Connection();

		typedef std::shared_ptr<Connection> Ptr;

	public:
		/**
		 * 查询连接id
		 * @param id 返回连接id
		 */
		virtual void getId(std::string& id) const = 0;

		/**
		 * 获取远端IP地址
		 * @param remoteIp 返回远端ip地址
		 */
		virtual void getRemoteIp(std::string& remoteIp) const = 0;

		/**
		 * 发送数据
		 * 
		 * @param buf 数据缓存
		 * @param length 数据长度
		 */
		virtual void send(const char* buf, std::size_t length) = 0;

		/**
		 * 发送数据
		 *
		 * @param data 数据缓存
		 */
		virtual void send(const std::shared_ptr<std::string>& data) = 0;

		/**
		 * 连接是否已断开
		 */
		virtual bool isClosed() = 0;
	};
}

#endif // !_NIU_MA_CONNECTION_H_