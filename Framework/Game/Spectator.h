// Spectator.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2026.08.06

#ifndef _NIU_MA_SPECTATOR_H_
#define _NIU_MA_SPECTATOR_H_

#include "Network/Session.h"

namespace NiuMa
{
	/**
	 * 旁观者
	 */
	class Spectator
	{
	public:
		Spectator(const std::string& playerId, bool robot = false);
		virtual ~Spectator() = default;

		using Ptr = std::shared_ptr<Spectator>;

	public:
		// 基本玩家信息
		const std::string& getPlayerId() const;
		const std::string& getNickname() const;
		void setNickname(const std::string& s);
		const std::string& getPhone() const;
		void setPhone(const std::string& s);
		int getSex() const;
		void setSex(int s);
		const std::string& getHeadUrl() const;
		void setHeadUrl(const std::string& s);

		//
		bool isRobot() const;

		// 设置网络会话
		void setSession(const Session::Ptr& session);

		// 获取网络会话
		Session::Ptr getSession();

		// 当前是否离线
		bool isOffline();

		// 设置离线时间
		void setOfflineTick(time_t t);

		// 获取离线时间
		time_t getOfflineTick() const;

	private:
		// 玩家id
		const std::string _playerId;

		// 玩家昵称
		std::string _nickname;

		// 联系电话
		std::string _phone;

		// 性别
		int _sex;

		// 头像url
		std::string _headUrl;

		// 是否为机器人
		const bool _robot;

		// 网络会话
		std::weak_ptr<Session> _session;

		// 离线线时间，单位毫秒
		time_t _offlineTick;
	};
}

#endif // !_NIU_MA_SPECTATOR_H_