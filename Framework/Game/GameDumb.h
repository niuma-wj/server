// GameDumb.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.10.16

#ifndef _NIU_MA_GAME_DUMB_H_
#define _NIU_MA_GAME_DUMB_H_

#include "Venue/VenueLoader.h"

#include <unordered_set>

namespace NiuMa {
	/**
	 * 空游戏，用于测试框架代码
	 * 该游戏没有任何内容，只有进入、离开及结束几个逻辑
	 * 该游戏类型固定为1，其他游戏的类型请勿与该游戏冲突
	 */
	class GameDumb : public Venue {
	public:
		GameDumb(const std::string& id, const std::string& name, int maxPlayers);
		virtual ~GameDumb();

	public:
		virtual void onDisconnect(const std::string& playerId) override;

	protected:
		virtual bool hasPlayer(const std::string& playerId) override;
		virtual void getPlayerIds(std::vector<std::string>& playerIds) override;
		virtual int getPlayerCount() override;
		virtual bool enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) override;
		virtual int leaveImpl(const std::string& playerId, std::string& errMsg) override;

	private:
		// 游戏名称
		const std::string _name;

		// 最大进入玩家数量
		const int _maxPlayers;

		// 玩家id集合
		std::unordered_set<std::string> _playerIds;
	};

	/**
	 * 空游戏加载器
	 */
	class GameDumbLoader : public VenueLoader {
	public:
		GameDumbLoader();
		virtual ~GameDumbLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;
	};
}

#endif // !_NIU_MA_GAME_DUMB_H_