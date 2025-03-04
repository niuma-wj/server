// BiJiLoader.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.26

#ifndef _NIU_MA_BI_JI_LOADER_H_
#define _NIU_MA_BI_JI_LOADER_H_

#include "Venue/VenueLoader.h"
#include "BiJiRule.h"

namespace NiuMa
{
	/**
	 * 比鸡游戏加载器
	 */
	class BiJiLoader : public VenueLoader
	{
	public:
		BiJiLoader();
		virtual ~BiJiLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;

	private:
		std::shared_ptr<BiJiRule> _rule;
	};
}

#endif // !_NIU_MA_BI_JI_LOADER_H_