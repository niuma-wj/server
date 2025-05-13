// GameAvatar.cpp

#include "GameAvatar.h"

namespace NiuMa {
	GameAvatar::GameAvatar(const std::string& playerId, bool robot)
		: _playerId(playerId)
		, _sex(0)
		, _robot(robot)
		, _seat(-1)
		, _gold(0LL)
		, _cashPledge(0LL)
		, _authorize(false)
		, _ready(false)
		, _offline(false)
		, _latitude(0.0f)
		, _longitude(0.0f)
		, _altitude(0.0f)
	{}

	GameAvatar::~GameAvatar() {}

	const std::string& GameAvatar::getPlayerId() const {
		return _playerId;
	}

	const std::string& GameAvatar::getNickname() const {
		return _nickname;
	}

	void GameAvatar::setNickname(const std::string& s) {
		_nickname = s;
	}

	const std::string& GameAvatar::getPhone() const {
		return _phone;
	}

	void GameAvatar::setPhone(const std::string& s) {
		_phone = s;
	}

	int GameAvatar::getSex() const {
		return _sex;
	}

	void GameAvatar::setSex(int s) {
		_sex = s;
	}

	const std::string& GameAvatar::getHeadUrl() const {
		return _headUrl;
	}

	void GameAvatar::setHeadUrl(const std::string& s) {
		_headUrl = s;
	}

	bool GameAvatar::isRobot() const {
		return _robot;
	}

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

	bool GameAvatar::isOffline() const {
		return _offline;
	}

	void GameAvatar::setOffline(bool s) {
		_offline = s;
	}

	void GameAvatar::setSession(const Session::Ptr& session) {
		if (session)
			_session = session;
		else
			_session.reset();
	}

	Session::Ptr GameAvatar::getSession() {
		return _session.lock();
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
}