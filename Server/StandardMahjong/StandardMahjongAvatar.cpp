// StandardMahjongAvatar.cpp

#include "StandardMahjongAvatar.h"

namespace NiuMa
{
	StandardMahjongAvatar::StandardMahjongAvatar(const std::string& playerId, int seat, bool bRobot)
		: MahjongAvatar(playerId, seat, bRobot)
		, _winGold(0.0)
	{
		for (int i = 0; i < 4; i++)
			_loseScores[i] = 0;
	}

	StandardMahjongAvatar::~StandardMahjongAvatar() {}

	void StandardMahjongAvatar::clear() {
		MahjongAvatar::clear();

		for (int i = 0; i < 4; i++)
			_loseScores[i] = 0;
		_winGold = 0LL;
	}

	int StandardMahjongAvatar::calcHuScore() const {
		int score = 0;
		// 算平胡分
		bool pingHu = true;
		if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::PengPengHu)) == static_cast<int>(MahjongGenre::HuStyle::PengPengHu)) {
			// 碰碰胡
			score = 2;
		}
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::DanDiao)) == static_cast<int>(MahjongGenre::HuStyle::DanDiao))	{
			// 单吊，全求人、全求炮与单吊不叠加
			if ((_huWay & static_cast<int>(MahjongGenre::HuWay::QuanQiuRen)) == static_cast<int>(MahjongGenre::HuWay::QuanQiuRen) ||
				(_huWay & static_cast<int>(MahjongGenre::HuWay::QuanQiuPao)) == static_cast<int>(MahjongGenre::HuWay::QuanQiuPao))
				score = 1;
			else
				score = 2;
		}
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::BianZhang)) == static_cast<int>(MahjongGenre::HuStyle::BianZhang))		// 边张
			score = 2;
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::KaZhang)) == static_cast<int>(MahjongGenre::HuStyle::KaZhang))			// 卡张
			score = 2;
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::PingHu)) == static_cast<int>(MahjongGenre::HuStyle::PingHu))			// 平胡
			score = 1;
		else
			pingHu = false;
		// 算七小对分
		if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui3)) == static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui3))		// 三豪华七小对
			score = 16;
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui2)) == static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui2))	// 双豪华七小对
			score = 12;
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui1)) == static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui1))	// 单豪华七小对
			score = 8;
		else if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui)) == static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui))		// 七小对
			score = 4;
		// 算十三幺分
		if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::ShiSanYao)) == static_cast<int>(MahjongGenre::HuStyle::ShiSanYao))			// 十三幺
			score = 16;

		bool menQing = false;
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::MenQing)) == static_cast<int>(MahjongGenre::HuWay::MenQing)) {
			// 七小对、十三幺、天胡、地胡与门清不重叠
			if (((_huWay & static_cast<int>(MahjongGenre::HuWay::TianHu)) != static_cast<int>(MahjongGenre::HuWay::TianHu)) &&
				((_huWay & static_cast<int>(MahjongGenre::HuWay::DiHu)) != static_cast<int>(MahjongGenre::HuWay::DiHu)) && pingHu)
				menQing = true;
		}
		// 门清翻2倍
		if (menQing)
			score *= 2;
		// 全求人、全求炮翻2倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::QuanQiuRen)) == static_cast<int>(MahjongGenre::HuWay::QuanQiuRen) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::QuanQiuPao)) == static_cast<int>(MahjongGenre::HuWay::QuanQiuPao))
			score *= 2;
		// 清一色翻2倍
		if ((_huStyle & static_cast<int>(MahjongGenre::HuStyle::QingYiSe)) == static_cast<int>(MahjongGenre::HuStyle::QingYiSe))
			score *= 2;
		// 杠上花、杠上炮、抢杠胡翻倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangHua4)) == static_cast<int>(MahjongGenre::HuWay::GangShangHua4) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangPao4)) == static_cast<int>(MahjongGenre::HuWay::GangShangPao4) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::QiangGangHu4)) == static_cast<int>(MahjongGenre::HuWay::QiangGangHu4))
			score *= 5;
		else if ((_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangHua3)) == static_cast<int>(MahjongGenre::HuWay::GangShangHua3) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangPao3)) == static_cast<int>(MahjongGenre::HuWay::GangShangPao3) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::QiangGangHu3)) == static_cast<int>(MahjongGenre::HuWay::QiangGangHu3))
			score *= 4;
		else if ((_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangHua2)) == static_cast<int>(MahjongGenre::HuWay::GangShangHua2) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangPao2)) == static_cast<int>(MahjongGenre::HuWay::GangShangPao2) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::QiangGangHu2)) == static_cast<int>(MahjongGenre::HuWay::QiangGangHu2))
			score *= 3;
		else if ((_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangHua1)) == static_cast<int>(MahjongGenre::HuWay::GangShangHua1) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::GangShangPao1)) == static_cast<int>(MahjongGenre::HuWay::GangShangPao1) ||
			(_huWay & static_cast<int>(MahjongGenre::HuWay::QiangGangHu1)) == static_cast<int>(MahjongGenre::HuWay::QiangGangHu1))
			score = 2;
		// 海底捞月翻2倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::HaiDiLaoYue)) == static_cast<int>(MahjongGenre::HuWay::HaiDiLaoYue))
			score *= 2;
		// 海底炮翻2倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::HaiDiPao)) == static_cast<int>(MahjongGenre::HuWay::HaiDiPao))
			score *= 2;
		// 天胡翻10倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::TianHu)) == static_cast<int>(MahjongGenre::HuWay::TianHu))
			score *= 10;
		// 地胡翻5倍
		if ((_huWay & static_cast<int>(MahjongGenre::HuWay::DiHu)) == static_cast<int>(MahjongGenre::HuWay::DiHu))
			score *= 5;

		return score;
	}

	void StandardMahjongAvatar::addLoseScore(int seat, int s) {
		if (seat < 0 || seat > 3)
			return;
		_loseScores[seat] += s;
	}

	void StandardMahjongAvatar::getLoseScores(int loseScores[4]) const {
		for (int i = 0; i < 4; i++)
			loseScores[i] = _loseScores[i];
	}

	void StandardMahjongAvatar::setWinGold(double g) {
		_winGold = g;
	}

	double StandardMahjongAvatar::getWinGold() const {
		return _winGold;
	}
}