// MahjongPlayback.cpp

#include "MahjongPlayback.h"

namespace NiuMa
{
	MahjongPlaybackData::MahjongPlaybackData()
	{}

	MahjongPlaybackData::~MahjongPlaybackData()
	{}

	void MahjongPlaybackData::initialize() {
		for (int i = 0; i < 4; i++) {
			dealedTiles[i].clear();
			chapters[i].clear();
		}
		actions.clear();
		actors.clear();
	}
}