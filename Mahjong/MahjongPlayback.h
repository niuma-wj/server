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
	 * ����¼���¼
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
		 * ������ҵĳ�ʼ����
		 */
		MahjongTileArray dealedTiles[4];

		/**
		 * ������ҵ���������
		 */
		MahjongChapterArray chapters[4];

		/**
		 * �����б�
		 */
		MahjongActionList actions;

		/**
		 * �������б�
		 */
		MahjongActorList actors;
	};
}

#endif // !_NIU_MA_MAHJONG_PLAYBACK_H_