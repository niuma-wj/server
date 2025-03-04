// StandardMahjongAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.03

#ifndef _NIU_MA_STANDARD_MAHJONG_AVATAR_H_
#define _NIU_MA_STANDARD_MAHJONG_AVATAR_H_

#include "MahjongAvatar.h"

namespace NiuMa
{
	class StandardMahjongAvatar : public MahjongAvatar {
	public:
		StandardMahjongAvatar(const std::string& playerId, int seat, bool bRobot);
		virtual ~StandardMahjongAvatar();

	public:
		virtual void clear() override;
		virtual int calcHuScore() const override;

	public:
		void addLoseScore(int seat, int s);
		void getLoseScores(int loseScores[4]) const;
		void setWinGold(double g);
		double getWinGold() const;

	private:
		/**
		 * 一局中玩家需要赔付给其他玩家的赔分
		 */
		int _loseScores[4];

		/**
		 * 一局结算之后玩家赢得(或输)的金币数量
		 * 该数值是临时数值，使用浮点数存储
		 */
		double _winGold;
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_AVATAR_H_
