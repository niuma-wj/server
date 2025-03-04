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
		// 4����ע�������ע����
		int _betAmounts[4];

		// ����4����ע�������Ӯ����
		int _scores[4];

		// ��20���ۼ���ע���
		int64_t _accBets20;

		// ��20�ֵ��ۼ�Ӯ����
		int _accWins20;

		// ��20�ֵ���ע���
		std::queue<int> _bets20;

		// ��20�ֵ���Ӯ���
		std::queue<int> _wins20;

		// ����������ʱ��(��������Ч)
		time_t _joinTick;

		// ��һ����עʱ��(��������Ч)
		time_t _nextBetTick;

	public:
		void clear();
		void addBet(int zone, int amount);
		int getBetAmount(int zone) const;
		int getBetAmount() const;
		void setScore(int zone, int s);
		int getScore(int zone) const;
		int getScore() const;
		void pushBet();	// ��������עѹ���ۼƣ�ͬʱ��ձ�����ע
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