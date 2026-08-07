// GameAvatar.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.22

#ifndef _NIU_MA_GAME_AVATAR_H_
#define _NIU_MA_GAME_AVATAR_H_

#include "Spectator.h"

namespace NiuMa
{
	/**
	 * 玩家替身基类
	 * 代表一个玩家替身，保存玩家在游戏中的状态数据，该类中的所有数据和方法都是在同一
	 * 线程内访问的，不需要担心线程同步的问题
	 */
	class GameAvatar : public Spectator {
	public:
		GameAvatar(const std::string& playerId, bool robot = false);
		virtual ~GameAvatar();

		using Ptr = std::shared_ptr<GameAvatar>;

	public:
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

	private:
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
	};
}

#endif // !_NIU_MA_GAME_AVATAR_H_