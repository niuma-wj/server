// MahjongGenre.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.02

#ifndef _NIU_MA_MAHJONG_GENRE_H_
#define _NIU_MA_MAHJONG_GENRE_H_

#include "MahjongTile.h"

namespace NiuMa
{
	/**
	 * 麻将牌型
	 */
	class MahjongGenre
	{
	public:
		MahjongGenre();
		virtual ~MahjongGenre();

	public:
		// 胡牌样式
		enum class HuStyle : int
		{
			Invalid				= 0x00000000,				// 无效
			PingHu				= 0x00000001,				// 平胡
			DanDiao				= (0x00000002 | PingHu),	// 单吊
			BianZhang			= (0x00000004 | PingHu),	// 边张
			KaZhang				= (0x00000008 | PingHu),	// 卡张
			PengPengHu			= (0x00000010 | PingHu),	// 碰碰胡
			QingYiSe			= 0x00000020,				// 清一色
			ZiYiSe				= 0x00000040,				// 字一色
			QiXiaoDui			= 0x00000080,				// 七小对
			QiXiaoDui1			= (0x00000100 | QiXiaoDui),	// 单豪华七小对
			QiXiaoDui2			= (0x00000200 | QiXiaoDui),	// 双豪华七小对
			QiXiaoDui3			= (0x00000400 | QiXiaoDui),	// 三豪华七小对
			ShiSanYao			= 0x00000800,				// 十三幺
			ShiSanLan			= 0x00001000,				// 十三烂
			QiXingShiSanLan		= (0x00002000 | ShiSanLan)	// 七星十三烂
		};
		
		// 胡牌方式
		enum class HuWay : int
		{
			Invalid				= 0x00000000,				// 无效
			ZiMo				= 0x00000001,				// 自摸
			DianPao				= 0x00000002,				// 点炮
			MenQing				= 0x00000004,				// 门清
			QuanQiuRen			= 0x00000008,				// 全求人(手上仅剩最后一张牌)
			QuanQiuPao			= (0x00000010 | DianPao),	// 全求炮，手上仅剩最后一张牌的玩家放炮
			TianHu				= (0x00000020 | ZiMo),		// 天胡，庄家起手拿第一张牌自摸
			DiHu				= (0x00000040 | DianPao),	// 地胡，庄家打出第一张牌放炮
			RenHu				= 0x00000080,				// 人胡，闲家第一圈内自摸或放炮
			HaiDiLaoYue			= (0x00000100 | ZiMo),		// 海底捞月
			HaiDiPao			= (0x00000200 | DianPao),	// 海底炮
			MingGang			= 0x00000400,				// 明杠，注意该值本身不代表一个胡牌方式，仅是一个标志，与杠上花组合在一起以区分是明杠上花还是暗杠上花
			GangShangHua1		= (0x00000800 | ZiMo),		// 杠上花
			GangShangHua2		= (0x00001000 | ZiMo),		// 二连杠上花
			GangShangHua3		= (0x00002000 | ZiMo),		// 三连杠上花
			GangShangHua4		= (0x00004000 | ZiMo),		// 四连杠上花
			GangShangPao1		= (0x00008000 | DianPao),	// 杠上炮
			GangShangPao2		= (0x00010000 | DianPao),	// 二连杠上炮
			GangShangPao3		= (0x00020000 | DianPao),	// 三连杠上炮
			GangShangPao4		= (0x00040000 | DianPao),	// 四连杠上炮
			QiangGangHu1		= (0x00080000 | DianPao),	// 抢杠胡
			QiangGangHu2		= (0x00100000 | DianPao),	// 二连抢杠胡
			QiangGangHu3		= (0x00200000 | DianPao),	// 三连抢杠胡
			QiangGangHu4		= (0x00400000 | DianPao)	// 四连抢杠胡
		};

		// 听牌
		typedef struct __TingPai
		{
			MahjongTile::Tile tile;	// 牌
			int style;				// 胡牌样式

			__TingPai()
				: style(static_cast<int>(HuStyle::Invalid))
			{}

			__TingPai(const MahjongTile::Tile& t, HuStyle s)
				: tile(t)
				, style(static_cast<int>(s))
			{}

		}TingPai;

		// 听牌数组
		typedef std::vector<TingPai> TingPaiArray;
	};
}

#endif