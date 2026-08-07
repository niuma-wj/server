// GameAvatar.cpp

#include "GameAvatar.h"

namespace NiuMa {
	GameAvatar::GameAvatar(const std::string& playerId, bool robot)
		: Spectator(playerId, robot)
		, _seat(-1)
		, _gold(0LL)
		, _cashPledge(0LL)
		, _authorize(false)
		, _ready(false)
		, _latitude(0.0f)
		, _longitude(0.0f)
		, _altitude(0.0f)
		, _winNum(0)
		, _loseNum(0)
		, _drawNum(0)
	{}

	GameAvatar::~GameAvatar() {}

	int GameAvatar::getSeat() const {
		return _seat;
	}

	void GameAvatar::setSeat(int s) {
		_seat = s;
	}

	int64_t GameAvatar::getGold() {
		return _gold;
	}

	void GameAvatar::setGold(int64_t gold) {
		_gold = gold;
	}

	int64_t GameAvatar::getCashPledge() const {
		return _cashPledge;
	}

	void GameAvatar::setCashPledge(int64_t s) {
		_cashPledge = s;
	}

	bool GameAvatar::isAuthorize() const {
		return _authorize;
	}

	void GameAvatar::setAuthorize(bool s) {
		_authorize = s;
	}

	bool GameAvatar::isReady() const {
		return _ready;
	}

	void GameAvatar::setReady(bool s) {
		_ready = s;
	}

	void GameAvatar::getGeolocation(double& lat, double& lon, double& alt) const
	{
		lat = _latitude;
		lon = _longitude;
		alt = _altitude;
	}

	void GameAvatar::setGeolocation(double lat, double lon, double alt)
	{
		_latitude = lat;
		_longitude = lon;
		_altitude = alt;
	}

	void GameAvatar::setScoreboard(int win, int lose, int draw) {
		_winNum = win;
		_loseNum = lose;
		_drawNum = draw;
	}

	void GameAvatar::getScoreboard(int& win, int& lose, int& draw) const {
		win = _winNum;
		lose = _loseNum;
		draw = _drawNum;
	}

	void GameAvatar::incWinNum() {
		_winNum++;
	}

	void GameAvatar::incLoseNum() {
		_loseNum++;
	}

	void GameAvatar::incDrawNum() {
		_drawNum++;
	}
}