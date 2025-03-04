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
		Invalid,			// 无效游戏
		Mahjong = 1021,		// 麻将游戏
		DouDiZhu = 1022,	// 斗地主牌桌
		NiuNiu100 = 1023,	// 百人牛牛
		NiuNiu = 1024,		// 经典牛牛
		RedBlack = 1025,	// 红黑大战
		ZhaJinHua = 1026,	// 炸金花
		LiuAnBiJi = 1027,	// 六安比鸡
		Lackey = 1028,		// 逮狗腿
		Tractor = 1029		// 拖拉机
	};


	// 游戏阶段状态
	enum class StageState : int
	{
		NotStarted,		// 未开始
		Underway,		// 进行中
		Finished		// 已结束
	};
}

#endif // !_NIU_MA_GAME_DEFINES_H_
