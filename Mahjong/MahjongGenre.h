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
	 * �齫����
	 */
	class MahjongGenre
	{
	public:
		MahjongGenre();
		virtual ~MahjongGenre();

	public:
		// ������ʽ
		enum class HuStyle : int
		{
			Invalid				= 0x00000000,				// ��Ч
			PingHu				= 0x00000001,				// ƽ��
			DanDiao				= (0x00000002 | PingHu),	// ����
			BianZhang			= (0x00000004 | PingHu),	// ����
			KaZhang				= (0x00000008 | PingHu),	// ����
			PengPengHu			= (0x00000010 | PingHu),	// ������
			QingYiSe			= 0x00000020,				// ��һɫ
			ZiYiSe				= 0x00000040,				// ��һɫ
			QiXiaoDui			= 0x00000080,				// ��С��
			QiXiaoDui1			= (0x00000100 | QiXiaoDui),	// ��������С��
			QiXiaoDui2			= (0x00000200 | QiXiaoDui),	// ˫������С��
			QiXiaoDui3			= (0x00000400 | QiXiaoDui),	// ��������С��
			ShiSanYao			= 0x00000800,				// ʮ����
			ShiSanLan			= 0x00001000,				// ʮ����
			QiXingShiSanLan		= (0x00002000 | ShiSanLan)	// ����ʮ����
		};
		
		// ���Ʒ�ʽ
		enum class HuWay : int
		{
			Invalid				= 0x00000000,				// ��Ч
			ZiMo				= 0x00000001,				// ����
			DianPao				= 0x00000002,				// ����
			MenQing				= 0x00000004,				// ����
			QuanQiuRen			= 0x00000008,				// ȫ����(���Ͻ�ʣ���һ����)
			QuanQiuPao			= (0x00000010 | DianPao),	// ȫ���ڣ����Ͻ�ʣ���һ���Ƶ���ҷ���
			TianHu				= (0x00000020 | ZiMo),		// �����ׯ�������õ�һ��������
			DiHu				= (0x00000040 | DianPao),	// �غ���ׯ�Ҵ����һ���Ʒ���
			RenHu				= 0x00000080,				// �˺����мҵ�һȦ�����������
			HaiDiLaoYue			= (0x00000100 | ZiMo),		// ��������
			HaiDiPao			= (0x00000200 | DianPao),	// ������
			MingGang			= 0x00000400,				// ���ܣ�ע���ֵ��������һ�����Ʒ�ʽ������һ����־������ϻ������һ���������������ϻ����ǰ����ϻ�
			GangShangHua1		= (0x00000800 | ZiMo),		// ���ϻ�
			GangShangHua2		= (0x00001000 | ZiMo),		// �������ϻ�
			GangShangHua3		= (0x00002000 | ZiMo),		// �������ϻ�
			GangShangHua4		= (0x00004000 | ZiMo),		// �������ϻ�
			GangShangPao1		= (0x00008000 | DianPao),	// ������
			GangShangPao2		= (0x00010000 | DianPao),	// ����������
			GangShangPao3		= (0x00020000 | DianPao),	// ����������
			GangShangPao4		= (0x00040000 | DianPao),	// ����������
			QiangGangHu1		= (0x00080000 | DianPao),	// ���ܺ�
			QiangGangHu2		= (0x00100000 | DianPao),	// �������ܺ�
			QiangGangHu3		= (0x00200000 | DianPao),	// �������ܺ�
			QiangGangHu4		= (0x00400000 | DianPao)	// �������ܺ�
		};

		// ����
		typedef struct __TingPai
		{
			MahjongTile::Tile tile;	// ��
			int style;				// ������ʽ

			__TingPai()
				: style(static_cast<int>(HuStyle::Invalid))
			{}

			__TingPai(const MahjongTile::Tile& t, HuStyle s)
				: tile(t)
				, style(static_cast<int>(s))
			{}

		}TingPai;

		// ��������
		typedef std::vector<TingPai> TingPaiArray;
	};
}

#endif