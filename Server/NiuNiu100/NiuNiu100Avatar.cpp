// NiuNiu100Avatar.cpp

#include "NiuNiu100Avatar.h"

namespace NiuMa
{
	NiuNiu100Avatar::NiuNiu100Avatar(const std::string& playerId, int seat, bool robot)
		: GameAvatar(playerId, robot)
		, _accBets20(0L)
		, _accWins20(0)
		, _joinTick(0L)
		, _nextBetTick(0L)
	{
		setSeat(seat);
		for (int i = 0; i < 4; i++) {
			_betAmounts[i] = 0;
			_scores[i] = 0;
		}
	}

	NiuNiu100Avatar::~NiuNiu100Avatar()
	{}

	void NiuNiu100Avatar::clear() {
		for (int i = 0; i < 4; i++) {
			_betAmounts[i] = 0;
			_scores[i] = 0;
		}
		_nextBetTick = 0L;
	}

	void NiuNiu100Avatar::addBet(int zone, int amount) {
		if (zone < 0 || zone > 3)
			return;
		_betAmounts[zone] += amount;
	}

	int NiuNiu100Avatar::getBetAmount(int zone) const {
		if (zone < 0 || zone > 3)
			return 0;

		return _betAmounts[zone];
	}

	int NiuNiu100Avatar::getBetAmount() const {
		int bet = 0;
		for (int i = 0; i < 4; i++)
			bet += _betAmounts[i];
		return bet;
	}

	void NiuNiu100Avatar::setScore(int zone, int s) {
		if (zone < 0 || zone > 3)
			return;
		_scores[zone] = s;
	}

	int NiuNiu100Avatar::getScore(int zone) const {
		if (zone < 0 || zone > 3)
			return 0;
		return _scores[zone];
	}

	int NiuNiu100Avatar::getScore() const {
		int score = 0;
		for (int i = 0; i < 4; i++)
			score += _scores[i];
		return score;
	}

	void NiuNiu100Avatar::pushBet() {
		int bet = getBetAmount();
		_bets20.push(bet);
		_accBets20 += bet;
		if (_bets20.size() > 20) {
			bet = _bets20.front();
			_accBets20 -= bet;
			_bets20.pop();
		}
	}

	void NiuNiu100Avatar::pushWin(bool win) {
		int num = win ? 1 : 0;
		_accWins20 += num;
		_wins20.push(num);
		if (_wins20.size() > 20) {
			num = _wins20.front();
			_accWins20 -= num;
			_wins20.pop();
		}
	}

	int64_t NiuNiu100Avatar::getAccBets20() const {
		// 前面累计加本局下注
		return _accBets20 + getBetAmount();
	}

	int NiuNiu100Avatar::getAccWins20() const {
		return _accWins20;
	}

	void NiuNiu100Avatar::setJoinTick(time_t t) {
		_joinTick = t;
	}

	time_t NiuNiu100Avatar::getJoinTick() const {
		return _joinTick;
	}

	void NiuNiu100Avatar::setNextBetTick(time_t t) {
		_nextBetTick = t;
	}

	time_t NiuNiu100Avatar::getNextBetTick() const {
		return _nextBetTick;
	}
}