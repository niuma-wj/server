// NiuNiu100Avatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.13

#ifndef _NIU_MA_NIUNIU100_AVATAR_H_
#define _NIU_MA_NIUNIU100_AVATAR_H_

#include "Game/GameAvatar.h"

#include <queue>

namespace NiuMa
{
	class NiuNiu100Avatar : public GameAvatar
	{
	public:
		NiuNiu100Avatar(const std::string& playerId, int seat, bool robot);
		virtual ~NiuNiu100Avatar();

	private:
		// 4个下注区域的下注总数
		int _betAmounts[4];

		// 本局4个下注区域的输赢数量
		int _scores[4];

		// 近20局累计下注金额
		int64_t _accBets20;

		// 近20局的累计赢局数
		int _accWins20;

		// 近20局的下注金额
		std::queue<int> _bets20;

		// 近20局的输赢结果
		std::queue<int> _wins20;

		// 加入牌桌的时间(机器人有效)
		time_t _joinTick;

		// 下一次下注时间(机器人有效)
		time_t _nextBetTick;

	public:
		void clear();
		void addBet(int zone, int amount);
		int getBetAmount(int zone) const;
		int getBetAmount() const;
		void setScore(int zone, int s);
		int getScore(int zone) const;
		int getScore() const;
		void pushBet();	// 将本局下注压入累计，同时清空本局下注
		void pushWin(bool win);
		int64_t getAccBets20() const;
		int getAccWins20() const;
		void setJoinTick(time_t t);
		time_t getJoinTick() const;
		void setNextBetTick(time_t t);
		time_t getNextBetTick() const;
	};
}

#endif