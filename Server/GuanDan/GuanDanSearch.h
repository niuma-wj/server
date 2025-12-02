// GuanDanSearch.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.07.01

#ifndef _NIU_MA_GUAN_DAN_SEARCH_H_
#define _NIU_MA_GUAN_DAN_SEARCH_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace NiuMa
{
	/**
	 * 检索数据
	 */
	class GuanDanSearchData
	{
	public:
		GuanDanSearchData(int id);
		virtual ~GuanDanSearchData();

	public:
		int getId() const;
		int getDamages() const;
		void setDamages(int damages);
		int getUndamages() const;
		void setUndamages(int undamages);
		int getNet() const;
		int getVariableCards() const;
		void setVariableCards(int num);
		const std::unordered_map<int, int>& getPointOrderNums() const;
		void setPointOrderNums(const std::unordered_map<int, int>& graph);
		int getPointOrderNum(int order) const;
		void addPointOrderNum(int order, int num);
		void subtractGraph(const std::unordered_map<int, int>& graph);

	public:
		// 清理
		virtual void clear();

	private:
		// id
		const int _id;

		// 牌型破坏系数
		int _damages;

		// 牌型无损系数
		int _undamages;

		// 逢人配张数
		int _variableCards;

		// 牌值数量表
		// key-牌值顺序，value-张数
		std::unordered_map<int, int> _pointOrderNums;
	};

	/**
	 * 掼蛋检索牌型
	 * 用于牌型检索及组合优化策略算法
	 */
	class GuanDanSearch : public GuanDanSearchData
	{
	public:
		GuanDanSearch(int id);
		virtual ~GuanDanSearch();
		typedef std::shared_ptr<GuanDanSearch> Ptr;

	public:
		int getGenre() const;
		void setGenre(int genre);
		void setOfficerPointOrder(int order);
		int getOfficerPointOrder() const;
		int getStraightMultiple() const;
		void setStraightMultiple(int multiple);
		int getSameSuitNum() const;
		void addSameSuit(int suit);
		int getSameSuit(int idx) const;
		void rollbackSameSuit(int idx);
		void setAdopt();
		bool getAdopt() const;
		void setBad();
		bool isBad() const;

	public:
		virtual void clear() override;

	private:
		// 牌型
		int _genre;

		// 主牌牌值顺序
		int _officerPointOrder;

		// 顺子倍数（3-三倍顺子、2-两倍顺子、1-顺子、默认0-非顺子）
		int _straightMultiple;

		// 同花顺数量
		int _sameSuitNum;

		// 同花顺的各种花色
		int _sameSuits[3];

		// 该检索排序是否被采用
		bool _adopt;

		// 非优选组合，例如需要占用逢人配才能组成的钢板，在首位出牌的时候不会这么组(因为那样还不如直接出三带二)，
		// 但是需要检索出来，在压牌出牌的时候可能会用到
		bool _bad;
	};
}

#endif // !_NIU_MA_GUAN_DAN_SEARCH_H_