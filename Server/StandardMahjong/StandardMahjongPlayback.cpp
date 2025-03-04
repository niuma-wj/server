// StandardMahjongPlayback.cpp

#include "StandardMahjongPlayback.h"

namespace NiuMa
{
	StandardMahjongPlaybackData::StandardMahjongPlaybackData() {
		for (int i = 0; i < 4; i++)
			winGolds[i] = 0;
	}

	StandardMahjongPlaybackData::~StandardMahjongPlaybackData() {}

	void StandardMahjongPlaybackData::initialize() {
		MahjongPlaybackData::initialize();

		for (int i = 0; i < 4; i++)
			winGolds[i] = 0;
	}
}