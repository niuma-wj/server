// MahjongSettlement.cpp

#include "MahjongSettlement.h"

namespace NiuMa
{
	MahjongSettlement::MahjongSettlement()
		: hu(0)
		, actor(0)
	{
		for (int i = 0; i < 4; i++) {
			huWays[i] = 0;
			huStyles[i] = 0;
			huStyleExs[i] = 0;
			scores[i] = 0;
		}
	}

	MahjongSettlement::~MahjongSettlement()
	{}

	void MahjongSettlement::initialize() {
		hu = 0;
		actor = 0;
		for (int i = 0; i < 4; i++) {
			huWays[i] = 0;
			huStyles[i] = 0;
			huStyleExs[i] = 0;
			scores[i] = 0;
			handTiles[i].clear();
			chapters[i].clear();
		}
		huTile = MahjongTile();
	}
}