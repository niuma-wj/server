// GameDefines.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.03

#ifndef _NIU_MA_GAME_DEFINES_H_
#define _NIU_MA_GAME_DEFINES_H_

namespace NiuMa
{
	enum class GameType : int
	{
		Invalid,			// ��Ч��Ϸ
		Mahjong = 1021,		// �齫��Ϸ
		DouDiZhu = 1022,	// ����������
		NiuNiu100 = 1023,	// ����ţţ
		NiuNiu = 1024,		// ����ţţ
		RedBlack = 1025,	// ��ڴ�ս
		ZhaJinHua = 1026,	// ը��
		LiuAnBiJi = 1027,	// �����ȼ�
		Lackey = 1028,		// ������
		Tractor = 1029		// ������
	};


	// ��Ϸ�׶�״̬
	enum class StageState : int
	{
		NotStarted,		// δ��ʼ
		Underway,		// ������
		Finished		// �ѽ���
	};
}

#endif // !_NIU_MA_GAME_DEFINES_H_
