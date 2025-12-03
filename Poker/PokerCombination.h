// PokerCombination.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_POKER_COMBINATION_H_
#define _NIU_MA_POKER_COMBINATION_H_

#include <memory>
#include <vector>
#include <unordered_set>
#include <unordered_map>

namespace NiuMa
{
	/**
	 * 扑克出牌组合
	 */
	class PokerCombination
	{
	public:
		PokerCombination(int id);
		virtual ~PokerCombination();

		typedef std::shared_ptr<PokerCombination> Ptr;

	private:
		// 组合id
		const int _id;

		// 牌型
		int _genre;

		// 主牌牌值
		int _officerPoint;

		// 主牌花色
		int _officerSuit;

		// 组合破坏系数
		int _damages;

		// 组合无损系数
		int _undamages;

		// 候选顺序，越小越在前面(即越先出)
		int _candidateOrder;

		// 是否已经加入候选列表
		bool _candidate;

		// 是否为非优选出牌组合，例如为了进行压牌而不得不进行拆牌产生的组合
		bool _bad;

		// 全部牌ID
		std::unordered_set<int> _cards;

		// 全部点数顺序的张数表，key-牌值顺序，value-张数
		std::unordered_map<int, int> _orders;

	public:
		int getId() const;
		void setGenre(int g);
		int getGenre() const;
		void setOfficerPoint(int point);
		int getOfficerPoint() const;
		void setOfficerSuit(int suit);
		int getOfficerSuit() const;
		void setDamages(int d);
		int getDamages() const;
		void setUndamages(int u);
		int getUndamages() const;
		int getNet() const;
		void setCandidateOrder(int order);
		int getCandidateOrder() const;
		void setCandidate(bool s = true);
		bool isCandidate() const;
		void setBad(bool s = true);
		bool isBad() const;
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

#endif // !_NIU_MA_POKER_COMBINATION_H_