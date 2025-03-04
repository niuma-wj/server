// StandardMahjongPlayback.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.06

#ifndef _NIU_MA_STANDARD_MAHJONG_PLAYBACK_H_
#define _NIU_MA_STANDARD_MAHJONG_PLAYBACK_H_

#include "MahjongSettlement.h"
#include "MahjongPlayback.h"

namespace NiuMa
{
	class StandardMahjongPlaybackData : public MahjongPlaybackData
	{
	public:
		StandardMahjongPlaybackData();
		virtual ~StandardMahjongPlaybackData();

	public:
		virtual void initialize();

	public:
		// 所有玩家的本局获利的金币数量
		int winGolds[4];

		// 结算数据
		MahjongSettlement settlement;

		MSGPACK_DEFINE_MAP(dealedTiles, chapters, actions, actors, winGolds, settlement);
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_PLAYBACK_H_