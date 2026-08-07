// Spectator.cpp

#include "Spectator.h"
#include "Player/PlayerManager.h"

namespace NiuMa
{
	Spectator::Spectator(const std::string& playerId, bool robot)
		: _playerId(playerId)
		, _robot(robot)
		, _sex(0)
		, _offlineTick(0)
	{}

	const std::string& Spectator::getPlayerId() const {
		return _playerId;
	}

	const std::string& Spectator::getNickname() const {
		return _nickname;
	}

	void Spectator::setNickname(const std::string& s) {
		_nickname = s;
	}

	const std::string& Spectator::getPhone() const {
		return _phone;
	}

	void Spectator::setPhone(const std::string& s) {
		_phone = s;
	}

	int Spectator::getSex() const {
		return _sex;
	}

	void Spectator::setSex(int s) {
		_sex = s;
	}

	const std::string& Spectator::getHeadUrl() const {
		return _headUrl;
	}

	void Spectator::setHeadUrl(const std::string& s) {
		_headUrl = s;
	}

	bool Spectator::isRobot() const {
		return _robot;
	}

	bool Spectator::isOffline() {
		if (_robot)
			return false;
		Session::Ptr sess = getSession();
		if (sess && sess->isValid())
			return false;
		return true;
	}

	void Spectator::setSession(const Session::Ptr& session) {
		if (session)
			_session = session;
		else
			_session.reset();
	}

	Session::Ptr Spectator::getSession() {
		Session::Ptr sess = _session.lock();
		if (sess)
			return sess;
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(_playerId);
		if (!player)
			return sess;
		return player->getSession();
	}

	void Spectator::setOfflineTick(time_t t) {
		_offlineTick = t;
	}

	time_t Spectator::getOfflineTick() const {
		return _offlineTick;
	}
}