// VenueLoader.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.26

#ifndef _NIU_MA_VENUE_LOADER_H_
#define _NIU_MA_VENUE_LOADER_H_

#include "Venue.h"

namespace NiuMa {
	/**
	 * 场地加载器接口
	 */
	class VenueLoader : public std::enable_shared_from_this<VenueLoader> {
	public:
		VenueLoader(int gameType);
		virtual ~VenueLoader();

		typedef std::shared_ptr<VenueLoader> Ptr;

	public:
		/**
		 * 获取游戏类型
		 */
		int getGameType() const;

		/**
		 * 加载场地
		 * @param id 场地id
		 * @return 场地
		 */
		virtual Venue::Ptr load(const std::string& id) = 0;

	private:
		// 游戏类型
		const int _gameType;
	};
}

#endif // !_NIU_MA_VENUE_LOADER_H_