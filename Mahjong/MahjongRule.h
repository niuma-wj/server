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
	 * �齫��Ϸ����
	 */
	class MahjongAvatar;
	class MahjongRule : public std::enable_shared_from_this<MahjongRule>
	{
	public:
		MahjongRule();
		virtual ~MahjongRule();

		typedef std::shared_ptr<MahjongRule> Ptr;

	protected:
		bool _13Lan;	// �Ƿ�֧��13������

	public:
		// �Ƿ������
		virtual bool hasFlower() const;

		// 
		void set13Lan(bool s);

		// 
		bool is13Lan() const;

		// ������ƣ�ע�⴫�������������Ǿ��������
		// ����allGotTilesΪ������ϼ������ܳ������ﵽ4�����������б���ǰΪ���㷨������ò������ƽ���Ϊ�ܵ���
		void checkTingPai(const MahjongTileArray& tiles, const MahjongTile::TileArray& allGotTiles, MahjongGenre::TingPaiArray& tps, MahjongAvatar* pAvatar) const;

		// ��⴫����������Ƿ�Ϊ��һɫ
		bool checkQingYiSe(const MahjongTileArray& tiles) const;

		// ��⴫����������Ƿ�Ϊ��һɫ
		bool checkZiYiSe(const MahjongTileArray& tiles) const;

		// ��⴫����������Ƿ�Ϊ��������ע�⴫�������������Ǿ��������
		bool checkPengPengHu(const MahjongTileArray& tiles) const;

		// ����Ƿ�Ϊ��С�ԣ�ע�⴫�������������Ǿ��������
		bool checkQiXiaoDui(const MahjongTileArray& tiles) const;

		// ��������С�ԣ�ע�⴫�������������Ǿ��������
		int checkHaoHuaQiXiaoDui(const MahjongTileArray& tiles) const;

	private:
		typedef struct __PingHuStyle
		{
			MahjongTile::Tile pai;
			unsigned int style;

			__PingHuStyle() : style(0) {}

		}PingHuStyle;

	protected:
		// �ж��Ƿ�Ϊʮ���ۣ�ע�⴫�������������Ǿ��������
		virtual bool checkShiSanYao(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// �ж��Ƿ�Ϊʮ���ã�ע�⴫�������������Ǿ��������
		virtual bool checkShiSanLan(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// �ж��Ƿ�Ϊ��С��
		virtual bool checkQiXiaoDui(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps) const;

		// �ж��Ƿ�Ϊƽ��
		virtual bool checkPingHu(const MahjongTileArray& tiles, MahjongGenre::TingPaiArray& tps, PingHuStyle* pStyle = nullptr) const;

		// �����չ�ĺ�����ʽ
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