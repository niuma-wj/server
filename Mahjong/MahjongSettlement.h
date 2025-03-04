// MahjongSettlement.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_SETTLEMENT_H_
#define _NIU_MA_MAHJONG_SETTLEMENT_H_

#include "MahjongAction.h"
#include "MahjongChapter.h"

namespace NiuMa
{
	/**
	 * �齫��������
	 */
	class MahjongSettlement
	{
	public:
		MahjongSettlement();
		virtual ~MahjongSettlement();

	public:
		virtual void initialize();

	public:
		/**
		 * �����Ƿ����(0-���֣�������)
		 */
		int hu;

		/**
		 * ���һ�����Ƶ����
		 */
		int actor;

		/**
		 * ������ҵĺ��Ʒ�ʽ
		 */
		int huWays[4];

		/**
		 * ������ҵĺ�����ʽ
		 */
		int huStyles[4];

		/**
		 * ȫ����ҵ���չ������ʽ
		 */
		int huStyleExs[4];

		/**
		 * ������ҵı��ֵ÷�
		 */
		int scores[4];

		/**
		 * ���������������
		 */
		MahjongTile huTile;

		/**
		 * ������ҵ�����
		 */
		MahjongTileArray handTiles[4];

		/**
		 * ������ҵ�������
		 */
		MahjongChapterArray chapters[4];

		MSGPACK_DEFINE_MAP(hu, actor, huWays, huStyles, huStyleExs, scores, huTile, handTiles, chapters);
	};

	
}

#endif // !_NIU_MA_MAHJONG_SETTLEMENT_H_