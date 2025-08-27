// GameAvatar.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.22

#ifndef _NIU_MA_GAME_AVATAR_H_
#define _NIU_MA_GAME_AVATAR_H_

#include "Network/Session.h"

namespace NiuMa
{
	/**
	 * 玩家替身基类
	 * 代表一个玩家替身，保存玩家在游戏中的状态数据，该类中的所有数据和方法都是在同一
	 * 线程内访问的，不需要担心线程同步的问题
	 */
	class GameAvatar {
	public:
		GameAvatar(const std::string& playerId, bool robot = false);
		virtual ~GameAvatar();

		typedef std::shared_ptr<GameAvatar> Ptr;

	public:
		const std::string& getPlayerId() const;
		const std::string& getNickname() const;
		void setNickname(const std::string& s);
		const std::string& getPhone() const;
		void setPhone(const std::string& s);
		int getSex() const;
		void setSex(int s);
		const std::string& getHeadUrl() const;
		void setHeadUrl(const std::string& s);
		bool isRobot() const;
		int getSeat() const;
		void setSeat(int s);
		int64_t getGold();
		void setGold(int64_t gold);
		int64_t getCashPledge() const;
		void setCashPledge(int64_t s);
		bool isAuthorize() const;
		void setAuthorize(bool s);
		bool isReady() const;
		void setReady(bool s);
		bool isOffline();
		void setSession(const Session::Ptr& session);
		Session::Ptr getSession();
		// 获得经纬度
		void getGeolocation(double& lat, double& lon, double& alt) const;

		// 设置经纬度
		void setGeolocation(double lat, double lon, double alt);

		// 设置输赢及平局次数
		void setScoreboard(int win, int lose, int draw);

		// 获取输赢及平局次数
		void getScoreboard(int& win, int& lose, int& draw) const;

		// 增加赢局次数
		void incWinNum();

		// 增加输局次数
		void incLoseNum();

		// 增加平局次数
		void incDrawNum();

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

		// 玩家在游戏桌上的座位索引，-1表示无座位
		int _seat;

		// 金币数量，注意，该数值是用于方便查询，并不是绝对准确的，例如玩家在大厅中消费了金币，
		// 并不会立即反映该数值中。不需要担心因为该数值的准确性而影响到游戏逻辑，因为游戏逻辑
		// 中并不依赖于该数值，而是数据库中的玩家金币数量。
		int64_t _gold;

		// 当前押金数
		int64_t _cashPledge;

		// 是否托管，所谓托管即由系统自动为玩家执行游戏操作
		bool _authorize;

		// 是否已准备就绪
		bool _ready;

		// 网络会话
		std::weak_ptr<Session> _session;

		// 纬度
		double _latitude;

		// 经度
		double _longitude;

		// 海拔
		double _altitude;

		// 赢局总数
		int _winNum;

		// 输局总数
		int _loseNum;

		// 平局总数
		int _drawNum;

		// 离线线时间，单位毫秒
		time_t _offlineTick;
	};
}

#endif // !_NIU_MA_GAME_AVATAR_H_