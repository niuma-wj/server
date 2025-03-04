// LackeyAvatarEx.cpp

#include "Base/Log.h"
#include "PokerUtilities.h"
#include "LackeyRule.h"
#include "LackeyAvatarEx.h"

namespace NiuMa
{
	LackeyAvatarEx::LackeyAvatarEx(const std::shared_ptr<LackeyRule>& rule, const std::string& playerId, int seat, bool robot)
		: LackeyAvatar(rule, playerId, seat, robot)
		, _showCard(false)
		, _score(0.0f)
		, _xiQianScore(0)
		, _winGold(0.0)
		, _wins(0)
		, _loses(0)
		, _draws(0)
		, _offlineTick(0)
	{
		for (int i = 0; i < 5; i++)
			_loseGolds[i] = 0.0;
	}

	LackeyAvatarEx::~LackeyAvatarEx()
	{}

	void LackeyAvatarEx::clear() {
		LackeyAvatar::clear();

		_showCard = false;
		_score = 0.0f;
		_xiQianScore = 0;
		_winGold = 0.0;
		for (int i = 0; i < 5; i++)
			_loseGolds[i] = 0.0;
	}

	void LackeyAvatarEx::setShowCard() {
		_showCard = true;
	}

	bool LackeyAvatarEx::isShowCard() const {
		return _showCard;
	}

	void LackeyAvatarEx::setScore(float s) {
		_score = s;
	}

	float LackeyAvatarEx::getScore() const {
		return _score;
	}

	void LackeyAvatarEx::setXiQianScore(int s) {
		_xiQianScore = s;
	}

	int LackeyAvatarEx::getXiQianScore() const {
		return _xiQianScore;
	}

	void LackeyAvatarEx::addLoseGold(int seat, double gold) {
		if (seat < 0 || seat > 4)
			return;
		_loseGolds[seat] += gold;
	}

	const double* LackeyAvatarEx::getLoseGolds() const {
		return _loseGolds;
	}

	void LackeyAvatarEx::setWinGold(double g, bool flag) {
		_winGold = g;
		if (flag) {
			if (g > 0.0)
				_wins++;
			else if (g < 0.0)
				_loses++;
			else
				_draws++;
		}
	}

	double LackeyAvatarEx::getWinGold() const {
		return _winGold;
	}

	void LackeyAvatarEx::setWinLose(int win, int lose, int draw) {
		_wins = win;
		_loses = lose;
		_draws = draw;
	}

	void LackeyAvatarEx::getWinLose(int& win, int& lose, int& draw) {
		win = _wins;
		lose = _loses;
		draw = _draws;
	}

	void LackeyAvatarEx::setOfflineTick(time_t t) {
		_offlineTick = t;
	}

	time_t LackeyAvatarEx::getOfflineTick() const {
		return _offlineTick;
	}
}