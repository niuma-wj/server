// LackeyLoader.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.01.04

#ifndef _NIU_MA_LACKEY_LOADER_H_
#define _NIU_MA_LACKEY_LOADER_H_

#include "Venue/VenueLoader.h"
#include "LackeyRule.h"

namespace NiuMa
{
	/**
	 * 逮狗腿游戏加载器
	 */
	class LackeyLoader : public VenueLoader
	{
	public:
		LackeyLoader();
		virtual ~LackeyLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;

	private:
		std::shared_ptr<LackeyRule> _rule;
	};
}

#endif // !_NIU_MA_LACKEY_LOADER_H_