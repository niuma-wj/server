// GuanDanLoader.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.07.17

#ifndef _NIU_MA_GUAN_DAN_LOADER_H_
#define _NIU_MA_GUAN_DAN_LOADER_H_

#include "Venue/VenueLoader.h"

namespace NiuMa
{
	/**
	 * 掼蛋游戏加载器
	 */
	class GuanDanLoader : public VenueLoader
	{
	public:
		GuanDanLoader();
		virtual ~GuanDanLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;
	};
}

#endif // !_NIU_MA_GUAN_DAN_LOADER_H_