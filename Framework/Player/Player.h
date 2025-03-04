// Player.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.27

#ifndef _NIU_MA_PLAYER_H_
#define _NIU_MA_PLAYER_H_

#include <string>
#include <mutex>
#include <memory>
#include <list>
#include <unordered_set>
#include <ctime>

#include "Network/Session.h"

namespace NiuMa {
	/**
	 * 玩家类
	 * 注意，外部代码(例如在游戏逻辑代码中)不要存储该类实例，而应当只存储该类实例
	 * 的弱引用或者玩家id，否则可能会影响到玩家实例的释放
	 */
	class Player {
	public:
		Player(const std::string& id);
		virtual ~Player();
		typedef std::shared_ptr<Player> Ptr;

	public:
		/**
		 * 返回玩家id
		 */
		const std::string& getId() const;

		/**
		 * 设置登录账户名
		 */
		void setName(const std::string& name);

		/**
		 * 返回登录账户名
		 */
		const std::string& getName() const;

		/**
		 * 设置昵称
		 */
		void setNickname(const std::string& nickame);

		/**
		 * 返回昵称
		 */
		const std::string& getNickname() const;

		/**
		 * 设置联系电话
		 */
		void setPhone(const std::string& phone);

		/**
		 * 返回联系电话
		 */
		const std::string& getPhone() const;

		/**
		 * 设置性别
		 */
		void setSex(int sex);

		/**
		 * 返回性别
		 */
		int getSex() const;

		/**
		 * 设置头像url
		 */
		void setAvatar(const std::string& avatar);

		/**
		 * 返回头像url
		 */
		const std::string& getAvatar() const;

		/**
		 * 设置消息密钥
		 */
		void setSecret(const std::string& secret);

		/**
		 * 返回消息密钥
		 */
		void getSecret(std::string& secret);

		/**
		 * 测试随机串在1分钟内是否重复，若不重复则保存该随机串
		 * @param nonce 随机串
		 * @param timestamp 时间戳
		 * @return true-不重复，false-重复
		 */
		bool testNonce(const std::string& nonce, const time_t& timestamp);

		/**
		 * 设置网络会话
		 * @param session 网络会话
		 */
		void setSession(const Session::Ptr& session);

		/**
		 * 获取网络会话
		 * @return 网络会话
		 */
		Session::Ptr getSession();

		/**
		 * 获取网络会话id
		 * @param sessionId 返回网络会话id
		 * @return 是否成功获取
		 */
		bool getSessionId(std::string& sessionId);

		/**
		 * 设置当前所在场地id
		 * @param venueId 场地id
		 */
		void setVenueId(const std::string& venueId);

		/**
		 * 获取当前所在场地id
		 * @param venueId 场地id
		 */
		void getVenueId(std::string& venueId);

		/**
		 * 获取玩家id地址
		 * @param ip 返回玩家ip地址
		 * @return 是否获取成功
		 */
		bool getIp(std::string& ip);

		/**
		 * 设置离线状态
		 * @param offline 离线状态
		 */
		void setOffline(bool offline);

		/**
		 * 获取离线状态
		 * @return 离线状态
		 */
		bool getOffline();

		/**
		 * 获取离线时间和最新引用时间
		 * @param offlineTime 离线时间
		 * @param referenceTime 最新引用时间
		 */
		void getTime(time_t& offlineTime, time_t& referenceTime);

		/**
		 * 更新被引用时间
		 * @param nowTime 当前时间，该时间是不精确的
		 */
		void touch(const time_t& nowTime);

	private:
		// 玩家id
		const std::string _id;

		// 登录账户名
		std::string _name;

		// 玩家昵称
		std::string _nickname;

		// 联系电话
		std::string _phone;

		// 性别
		int _sex;

		// 头像url
		std::string _avatar;

		// 消息密钥
		std::string _secret;

		// 随机串序列
		std::list<std::pair<std::string, time_t> > _nonceSequence;

		// 随机串集合
		std::unordered_set<std::string> _nonceSet;

		// 当前网络会话
		std::weak_ptr<Session> _session;

		// 当前所在场地id
		std::string _venueId;

		// 是否离线
		bool _offline;

		// 离线时间
		time_t _offlineTime;

		// 最近一次被引用的时间
		time_t _referenceTime;

		// 信号量
		std::mutex _mtx;
	};
}

#endif // !_NIU_MA_PLAYER_H_