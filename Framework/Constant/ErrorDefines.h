// ErrorDefines.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.10.31

#ifndef _NIU_MA_ERROR_DEFINES_H_
#define _NIU_MA_ERROR_DEFINES_H_

namespace NiuMa {
	/**
	 * 进入场地结果码
	 */
	enum class EnterVenueResult : int {
		SUCCESS = 0,		// 成功
		UNAUTHORIZED,		// 未授权
		GAME_TYPE_ERROR,	// 游戏类型错误
		STATUS_ERROR,		// 场地状态错误
		LOAD_FAILED,		// 加载场地失败
		DISTRIBUTE_FAILED,	// 分配场地失败
		HANDLER_ERROR,		// 消息处理器错误
		ENTER_FAILED,		// 进入失败
		UNKNOWN				// 未知错误
	};
}

#endif