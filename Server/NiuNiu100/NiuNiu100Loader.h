// NiuNiu100Loader.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.19

#ifndef _NIU_MA_NIU_NIU_100_LOADER_H_
#define _NIU_MA_NIU_NIU_100_LOADER_H_

#include "Venue/VenueLoader.h"
#include "NiuNiuRule.h"

namespace NiuMa
{
	/**
	 * 百人牛牛游戏加载器
	 */
	class NiuNiu100Loader : public VenueLoader
	{
	public:
		NiuNiu100Loader();
		virtual ~NiuNiu100Loader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;

	private:
		std::shared_ptr<NiuNiuRule> _rule;
	};
}

#endif // !_NIU_MA_BI_JI_LOADER_H_