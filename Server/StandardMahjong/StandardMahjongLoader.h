// StandardMahjongLoader.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.09

#ifndef _NIU_MA_STANDARD_MAHJONG_LOADER_H_
#define _NIU_MA_STANDARD_MAHJONG_LOADER_H_

#include "Venue/VenueLoader.h"
#include "MahjongRule.h"

namespace NiuMa
{
	class StandardMahjongLoader : public VenueLoader
	{
	public:
		StandardMahjongLoader();
		virtual ~StandardMahjongLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;

	private:
		MahjongRule::Ptr _rule;
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_LOADER_H_