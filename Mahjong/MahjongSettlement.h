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
	 * 麻将结算数据
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
		 * 本局是否胡了(0-流局，其他胡)
		 */
		int hu;

		/**
		 * 最后一个出牌的玩家
		 */
		int actor;

		/**
		 * 所有玩家的胡牌方式
		 */
		int huWays[4];

		/**
		 * 所有玩家的胡牌样式
		 */
		int huStyles[4];

		/**
		 * 全部玩家的扩展胡牌样式
		 */
		int huStyleExs[4];

		/**
		 * 所有玩家的本局得分
		 */
		int scores[4];

		/**
		 * 胡牌玩家所胡的牌
		 */
		MahjongTile huTile;

		/**
		 * 所有玩家的手牌
		 */
		MahjongTileArray handTiles[4];

		/**
		 * 所有玩家的章数据
		 */
		MahjongChapterArray chapters[4];

		MSGPACK_DEFINE_MAP(hu, actor, huWays, huStyles, huStyleExs, scores, huTile, handTiles, chapters);
	};

	
}

#endif // !_NIU_MA_MAHJONG_SETTLEMENT_H_