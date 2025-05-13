// MahjongPlayback.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_PLAYBACK_H_
#define _NIU_MA_MAHJONG_PLAYBACK_H_

#include "MahjongAction.h"
#include "MahjongChapter.h"

namespace NiuMa
{
	/**
	 * 打牌录像记录
	 */
	class MahjongPlaybackData
	{
	public:
		MahjongPlaybackData();
		virtual ~MahjongPlaybackData();

	public:
		virtual void initialize();

	public:
		/**
		 * 所有玩家的初始发牌
		 */
		MahjongTileArray dealedTiles[4];

		/**
		 * 所有玩家的最终牌章
		 */
		MahjongChapterArray chapters[4];

		/**
		 * 动作列表
		 */
		MahjongActionList actions;

		/**
		 * 动作者列表
		 */
		MahjongActorList actors;
	};
}

#endif // !_NIU_MA_MAHJONG_PLAYBACK_H_