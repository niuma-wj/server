// MahjongRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.02

#ifndef _NIU_MA_MAHJONG_RULE_H_
#define _NIU_MA_MAHJONG_RULE_H_

#include "MahjongGenre.h"

#include <memory>

namespace NiuMa
{
	/**
	 * 麻将游戏规则
	 */
	class MahjongAvatar;
	class MahjongRule : public std::enable_shared_from_this<MahjongRule>
	{
	public:
		MahjongRule();
		virtual ~MahjongRule();

		typedef std::shared_ptr<MahjongRule> Ptr;

	protected:
		bool _13Lan;	// 是否支持13烂牌型

	public:
		// 是否带花牌
		virtual bool hasFlower() const;

		// 
		void set13Lan(bool s);

		// 
		bool is13Lan() const;

		// 检测听牌，注意传入的牌数组必须是经过排序的
		// 参数allGotTiles为玩家手上加上碰杠吃总数达到4个的牌类型列表，当前为简化算法，传入该参数的牌仅仅为杠的牌
		void checkTingPai(const MahjongTileArray& tiles, const MahjongTile::TileArray& allGotTiles, MahjongGenre::TingPaiArray& tps, MahjongAvatar* pAvatar) const;

		// 检测传入的牌数组是否为清一色
		bool checkQingYiSe(const MahjongTileArray& tiles) const;

		// 检测传入的牌数组是否为字一色
		bool checkZiYiSe(const MahjongTileArray& tiles) const;

		// 检测传入的牌数组是否为碰碰胡，注意传入的牌数组必须是经过排序的
		bool checkPengPengHu(const MahjongTileArray& tiles) const;

		// 检测是否为七小对，注意传入的牌数组必须是经过排序的
		bool checkQiXiaoDui(const MahjongTileArray& tiles) const;

		// 检测豪华七小对，注意传入的牌数组必须是经过排序的
		int checkHaoHuaQiXiaoDui(const MahjongTileArray& tiles) const;

	private:
		typedef struct __PingHuStyle
		{
			MahjongTile::Tile pai;
			unsigned int style;

			__PingHuStyle() : style(0) {}

		}PingHuStyle;

	protected:
		// 判断是否为十三幺，注意传入的牌数组必须是经过排序的
		virtual bool checkShiSanYao(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// 判断是否为十三烂，注意传入的牌数组必须是经过排序的
		virtual bool checkShiSanLan(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// 判断是否为七小对
		virtual bool checkQiXiaoDui(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// 判断是否为平胡
		virtual bool checkPingHu(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps, PingHuStyle* pStyle = nullptr) const;

		// 检测扩展的胡牌样式
		virtual bool checkHuStyleEx(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps, MahjongAvatar* pAvatar) const;

	private:
		bool checkPingHu(const MahjongTile::TileArray& tiles, MahjongGenre::TingPaiArray& tps, MahjongTile::Pattern pat, PingHuStyle* pStyle = nullptr) const;
		bool checkPingHu(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai, PingHuStyle* pStyle = nullptr) const;
		bool checkPingHu3n2(const MahjongTile::TileArray& tiles, PingHuStyle* pStyle = nullptr) const;
		bool checkPingHu3n2(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai, PingHuStyle* pStyle = nullptr) const;
		bool checkPingHu3n(const MahjongTile::TileArray& tiles, PingHuStyle* pStyle = nullptr) const;
		bool checkPingHu3n(const int arrNums[9], PingHuStyle* pStyle = nullptr) const;

	public:
		static int getTileNums(const MahjongTile::TileArray& tiles, MahjongTile::Tile pai);
		static int getPatternNums(const MahjongTileArray& tiles, MahjongTile::Pattern pat);
		static void getTilesByPattern(const MahjongTileArray& tiles, MahjongTile::Pattern pat, MahjongTile::TileArray& tmpTiles);
		static void getTileArrayString(const MahjongTileArray& tiles, std::string& text);
	};
}

#endif