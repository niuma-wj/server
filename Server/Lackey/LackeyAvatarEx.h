// LackeyAvatarEx.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.31

#ifndef _NIU_MA_LACKEY_AVATAR_EX_H_
#define _NIU_MA_LACKEY_AVATAR_EX_H_

#include "LackeyAvatar.h"

namespace NiuMa
{
	// 逮狗腿玩家替身
	class LackeyAvatarEx : public LackeyAvatar
	{
	public:
		LackeyAvatarEx(const std::shared_ptr<LackeyRule>& rule, const std::string& playerId, int seat, bool robot);
		virtual ~LackeyAvatarEx();

	public:
		// 清理
		virtual void clear() override;

	public:
		// 设置明牌
		void setShowCard();

		// 是否明牌
		bool isShowCard() const;

		// 
		void setScore(float s);

		// 
		float getScore() const;

		//
		void setXiQianScore(int s);

		//
		int getXiQianScore() const;

		// 
		void addLoseGold(int seat, double gold);

		// 
		const double* getLoseGolds() const;

		// 
		void setWinGold(double g, bool flag = true);

		//
		double getWinGold() const;

		// 
		void setWinLose(int win, int lose, int draw);

		// 
		void getWinLose(int& win, int& lose, int& draw);

		// 
		void setOfflineTick(time_t t);

		// 
		time_t getOfflineTick() const;

	private:
		// 是否明牌
		bool _showCard;

		// 输赢分
		float _score;

		// 喜钱分
		int _xiQianScore;

		// 一局输赢金币数量
		double _winGold;

		// 赢局总数
		int _wins;

		// 输局总数
		int _loses;

		// 平局总数
		int _draws;

		// 本玩家与所有其他玩家的负债关系表
		double _loseGolds[5];

		// 离线线时间，单位毫秒
		time_t _offlineTick;
	};
}

#endif // !_NIU_MA_LACKEY_AVATAR_EX_H_