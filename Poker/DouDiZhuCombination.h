// DouDiZhuCombination.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_DOU_DI_ZHU_COMBINATION_H_
#define _NIU_MA_DOU_DI_ZHU_COMBINATION_H_

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace NiuMa
{
	/**
	 * 斗地主类游戏的牌型组合
	 */
	class DouDiZhuCombination
	{
	public:
		DouDiZhuCombination(const int id);
		virtual ~DouDiZhuCombination();

		typedef std::shared_ptr<DouDiZhuCombination> Ptr;

	private:
		// 组合id
		int _id;

		// 牌型
		int _genre;

		// 主牌ID
		int _officer;

		// 组合破坏系数
		int _damages;

		// 是否已经加入候选列表
		bool _candidate;

		// 全部牌ID
		std::unordered_set<int> _cards;

		// 全部点数顺序的张数表
		std::unordered_map<int, int> _orders;	

	public:
		int getId() const;
		void setGenre(int g);
		int getGenre() const;
		void setOfficer(int o);
		int getOfficer() const;
		void setDamages(int d);
		int getDamages() const;
		void setCandidate(bool s = true);
		bool isCandidate() const;
		std::unordered_set<int>& getCards();
		const std::unordered_set<int>& getCards() const;
		void getCards(std::vector<int>& ids) const;
		void addCard(int id);
		void addCards(const std::vector<int>& ids);
		bool containsCard(int id) const;
		bool containsCard(const std::vector<int>& ids) const;
		const std::unordered_map<int, int>& getOrders() const;
		void addOrder(int order, int nums);
		bool containsOrder(int order) const;
		void clear();
	};
}

#endif