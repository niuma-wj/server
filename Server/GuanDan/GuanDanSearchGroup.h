// GuanDanSearchGroup.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.07.01

#ifndef _NIU_MA_GUAN_DAN_SEARCH_GROUP_H_
#define _NIU_MA_GUAN_DAN_SEARCH_GROUP_H_

#include "GuanDanSearch.h"

namespace NiuMa
{
	/**
	 * 掼蛋检索牌型组合
	 * 用于组合优化策略算法
	 */
	class GuanDanSearchGroup : public GuanDanSearchData
	{
	public:
		GuanDanSearchGroup(int id);
		virtual ~GuanDanSearchGroup();

		typedef std::shared_ptr<GuanDanSearchGroup> Ptr;

	public:
		bool containsSearch(int id) const;
		void addSearch(const GuanDanSearch::Ptr& search);
		const std::unordered_map<int, GuanDanSearch::Ptr>& getSearches() const;
		void removeThreeWith2();
		void setAdopt();

	public:
		virtual void clear() override;

	private:
		// 检索牌型表
		std::unordered_map<int, GuanDanSearch::Ptr> _searches;
	};
}

#endif // !_NIU_MA_GUAN_DAN_SEARCH_GROUP_H_