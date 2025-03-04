// Player.cpp

#include "Base/BaseUtils.h"
#include "Player.h"

namespace NiuMa {
	Player::Player(const std::string& id)
		: _id(id)
		, _sex(0)
		, _offline(false)
		, _offlineTime(0LL)
	{
		_referenceTime = BaseUtils::getCurrentSecond();
	}

	Player::~Player() {}

	const std::string& Player::getId() const {
		return _id;
	}

	void Player::setName(const std::string& name) {
		_name = name;
	}

	const std::string& Player::getName() const {
		return _name;
	}

	void Player::setNickname(const std::string& nickame) {
		_nickname = nickame;
	}

	const std::string& Player::getNickname() const {
		return _nickname;
	}

	void Player::setPhone(const std::string& phone) {
		_phone = phone;
	}

	const std::string& Player::getPhone() const {
		return _phone;
	}

	void Player::setSex(int sex) {
		_sex = sex;
	}

	int Player::getSex() const {
		return _sex;
	}

	void Player::setAvatar(const std::string& avatar) {
		_avatar = avatar;
	}

	const std::string& Player::getAvatar() const {
		return _avatar;
	}

	void Player::setSecret(const std::string& secret) {
		std::lock_guard<std::mutex> lck(_mtx);

		_secret = secret;
	}

	void Player::getSecret(std::string& secret) {
		std::lock_guard<std::mutex> lck(_mtx);

		secret = _secret;
	}

	bool Player::testNonce(const std::string& nonce, const time_t& timestamp) {
		std::lock_guard<std::mutex> lck(_mtx);

		time_t delta = 0;
		std::unordered_set<std::string>::const_iterator it;
		while (!(_nonceSequence.empty())) {
			const std::pair<std::string, time_t>& pair = _nonceSequence.front();
			delta = timestamp - pair.second;
			if (delta > 60L) {
				it = _nonceSet.find(pair.first);
				if (it != _nonceSet.end())
					_nonceSet.erase(it);
				_nonceSequence.pop_front();
			}
			else
				break;
		}
		it = _nonceSet.find(nonce);
		if (it != _nonceSet.end())
			return false;
		_nonceSet.insert(nonce);
		_nonceSequence.emplace_back(std::pair<std::string, time_t>(nonce, timestamp));
		return true;
	}

	void Player::setSession(const Session::Ptr& session) {
		std::lock_guard<std::mutex> lck(_mtx);

		_session = session;
	}

	Session::Ptr Player::getSession() {
		std::lock_guard<std::mutex> lck(_mtx);

		Session::Ptr ret = _session.lock();
		return ret;
	}

	bool Player::getSessionId(std::string& sessionId) {
		std::lock_guard<std::mutex> lck(_mtx);

		Session::Ptr session = _session.lock();
		if (session) {
			session->getId(sessionId);
			return true;
		}
		return false;
	}

	void Player::setVenueId(const std::string& venueId) {
		std::lock_guard<std::mutex> lck(_mtx);

		_venueId = venueId;
	}

	void Player::getVenueId(std::string& venueId) {
		std::lock_guard<std::mutex> lck(_mtx);

		venueId = _venueId;
	}

	bool Player::getIp(std::string& ip) {
		Session::Ptr session = getSession();
		if (session)
			ip = session->getRemoteIp();
		else
			return false;
		return !(ip.empty());
	}

	void Player::setOffline(bool offline) {
		std::lock_guard<std::mutex> lck(_mtx);

		_offline = offline;
		if (offline) {
			_session.reset();
			_offlineTime = BaseUtils::getCurrentSecond();
		}
	}

	bool Player::getOffline() {
		std::lock_guard<std::mutex> lck(_mtx);

		return _offline;
	}

	void Player::getTime(time_t& offlineTime, time_t& referenceTime) {
		std::lock_guard<std::mutex> lck(_mtx);

		offlineTime = _offlineTime;
		referenceTime = _referenceTime;
	}

	void Player::touch(const time_t& nowTime) {
		std::lock_guard<std::mutex> lck(_mtx);

		_referenceTime = nowTime;
	}
}