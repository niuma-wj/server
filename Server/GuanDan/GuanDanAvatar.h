// GuanDanAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.06.05

#ifndef _NIU_MA_GUAN_DAN_AVATAR_H_
#define _NIU_MA_GUAN_DAN_AVATAR_H_

#include "PokerGenre.h"
#include "PokerAvatar.h"
#include "GuanDanSearchGroup.h"

namespace NiuMa
{
	class GuanDanRule;
	// 掼蛋玩家替身
	class GuanDanAvatar : public PokerAvatar
	{
	public:
		GuanDanAvatar(const std::shared_ptr<GuanDanRule>& rule, const std::string& playerId, int seat, bool robot);
		virtual ~GuanDanAvatar();

	public:
		// 清理
		virtual void clear() override;

	protected:
		// 清理当前分析数据
		virtual void clearAnalysis() override;

		// 分析组合时是否要忽略指定的牌
		virtual bool analyzeIgnore(const PokerCard& c) override;

		// 检索所有牌型的全部组合
		virtual void combineAllGenres() override;

		// 更新候选组合列表，首位出牌
		virtual void candidateCombinationsImpl(int situation = 0) override;

		// 更新候选组合列表，压牌出牌
		virtual void candidateCombinationsImpl(const PokerGenre& pg, int situation = 0) override;

	public:
		// 设置正在等待进贡
		void setWaitingPresentTribute(bool setting);

		// 是否正在等待进贡
		bool isWaitingPresentTribute() const;

		// 设置正在等待还贡
		void setWaitingRefundTribute(bool setting);

		// 是否正在等待还贡
		bool isWaitingRefundTribute() const;

		// 设置进贡目标玩家座位号
		void setPresentTributeSeat(int seat);

		// 获取进贡目标玩家座位号
		int getPresentTributeSeat() const;

		// 设置还贡目标玩家座位号
		void setRefundTributeSeat(int seat);

		// 获取还贡目标玩家座位号
		int getRefundTributeSeat() const;

		// 设置还贡提示文本
		void setRefundTributeText(const std::string& text);

		// 获取还贡提示文本
		const std::string& getRefundTributeText() const;

		// 设置进贡或者还贡给出去的牌
		void setCardOut(const PokerCard& card);

		// 返回进贡或者还贡给出去的牌
		const PokerCard& getCardOut() const;

		// 设置进贡或者还贡拿回来的牌
		void setCardIn(const PokerCard& card);

		// 返回进贡或者还贡拿回来的牌
		const PokerCard& getCardIn() const;

		// 获取同花顺牌id
		bool getStraightFlush(std::vector<int>& cardIds) const;

		// 手牌全是大牌，如果全是大牌，在压牌的时候可以无需顾忌
		bool isAllBig() const;

	private:
		// 检索除钢板和木板外的全部三带二
		void seachThreeWith2();

		// 分配检索牌型
		GuanDanSearch::Ptr allocateSearch();

		// 回收检索牌型
		void freeSearch(const GuanDanSearch::Ptr& search);

		// 分配检索牌型组合
		GuanDanSearchGroup::Ptr allocateSearchGroup();

		// 回收检索牌型组合
		void freeSearchGroup(const GuanDanSearchGroup::Ptr& group);

		// 计算牌值曲线图的无损系数及破坏系数
		void calcDamages(const std::unordered_map<int, int>& graph, int& damages, int& undamages) const;

		/**
		 * 计算检索牌型组合中包含的一个检索牌型的无损系数及破坏系数
		 * @param graph1 检索牌型的牌值曲线图
		 * @param graph2 检索牌型组合中的牌值曲线图
		 * @param damages 返回无损系数
		 * @param undamages 返回破坏系数 
		 */
		void calcDamages(const std::unordered_map<int, int>& graph1, const std::unordered_map<int, int>& graph2, int& damages, int& undamages) const;

		// 计算检索牌型的无损系数及破坏系数
		void calcDamages(const GuanDanSearch::Ptr& search) const;

		// 计算完整拆分炸弹的造成的破坏系数
		void calcBombDamages(const std::unordered_map<int, int>& graph, int& damages) const;

		// 将检索牌型插入到队列正确位置
		void insertSearch(const GuanDanSearch::Ptr& search);

		// 将检索牌型组合插入到队列正确位置
		void insertGroup(const GuanDanSearchGroup::Ptr& group);

		// 合并检索牌型
		void mergeSearch(const GuanDanSearch::Ptr& search);

		/**
		 * 尝试将检索牌型并入检索牌型组合
		 * @param group 检索牌型组合
		 * @param search 检索牌型
		 * @return true-并入成功，false-并入失败
		 */
		bool mergeSearch(const GuanDanSearchGroup::Ptr& group, const GuanDanSearch::Ptr& search);

		/**
		 * 尝试将两个检索牌型合并成一个新的检索牌型组合
		 * @param search1 检索牌型1
		 * @param search2 检索牌型2
		 * @return 检索牌型组合
		 */
		GuanDanSearchGroup::Ptr mergeSearch(const GuanDanSearch::Ptr& search1, const GuanDanSearch::Ptr& search2);

		//计算多倍顺子及同花顺的附加无损系数
		void straightUndamages(const GuanDanSearch::Ptr& search, const std::unordered_map<int, int>& graph, int& undamages) const;

		/**
		 * 将所有检索牌型组合进行合并
		 */
		void mergeSearchGroups();

		/**
		 * 尝试将两个检索牌型组合合并成一个新的检索牌型组合
		 * @param group1 检索牌型组合1
		 * @param group2 检索牌型组合2
		 * @return 检索牌型组合
		 */
		GuanDanSearchGroup::Ptr mergeSearchGroup(const GuanDanSearchGroup::Ptr& group1, const GuanDanSearchGroup::Ptr& group2);

		/**
		 * 检测两个检索数据是否冲突
		 * @param search1 检索数据1
		 * @param search2 检索数据2
		 * @return true-冲突，false-不冲突
		 */
		bool conflictTest(const GuanDanSearchData* data1, const GuanDanSearchData* data2) const;

		/**
		 * 检测两个检索牌型组合是否冲突
		 * @param group1 检索牌型组合1
		 * @param group2 检索牌型组合2
		 * @return true-冲突，false-不冲突
		 */
		bool conflictTest(const GuanDanSearchGroup::Ptr& group1, const GuanDanSearchGroup::Ptr& group2) const;
		/**
		 * 构建所有出牌组合
		 */
		bool makeCombinations();

		/**
		 * 构造指定检索牌型列表的出牌组合
		 */
		bool makeCombinations(const std::unordered_map<int, int>& graph, const std::list<GuanDanSearch::Ptr>& searches);

		/**
		 * 为出牌组合分配牌
		 * @param rule 掼蛋规则
		 * @param comb 出牌组合
		 * @param graph 牌值曲线图
		 * @param nums 每种牌值的抽取张数
		 * @param layers 层数
		 * @param variableCards 占用的逢人配张数
		 * @param officerPointOrder 主牌牌值顺序
		 * @param suit 指定花色，-1表示不指定
		 * @return 是否分配成功
		 */
		bool getCombinationCards(GuanDanRule* rule, const PokerCombination::Ptr& comb, const std::unordered_map<int, int>& graph, int nums, int layers, int variableCards, int officerPointOrder, int suit);

		/**
		 * 为出牌组合分配牌
		 * @param rule 掼蛋规则
		 * @param comb 出牌组合
		 * @param graph 牌值曲线图
		 * @param nums 每种牌值的抽取张数
		 * @param officerPointOrder 主牌牌值顺序
		 * @param suit 指定花色，-1表示不指定
		 * @return 是否分配成功
		 */
		bool getCombinationCards1(GuanDanRule* rule, const PokerCombination::Ptr& comb, const std::unordered_map<int, int>& graph, int nums, int officerPointOrder, int suit);

		/**
		 * 构建其他牌型(单张、对子、三张、炸弹)的数据表格，并与逢人配作配
		 * @param data 当前检索牌型(组合)数据
		 * @param table 数据表格，key-张数，value-牌值列表，按牌值大小(考虑级牌)从大到小排列
		 * @param pointOrder1 第一张逢人配匹配的牌值顺序
		 * @param pointOrder2 第二张逢人配匹配的牌值顺序
		 * @return 是否构建成功
		 */
		bool makeOtherTable(GuanDanSearchData* data, std::unordered_map<int, std::list<int> >& table, int& pointOrder1, int& pointOrder2);

		/**
		 * 构造其他牌型的出牌组合
		 * @param table 数据表格
		 * @param pointOrder1 第一张逢人配匹配的牌值顺序
		 * @param pointOrder2 第二张逢人配匹配的牌值顺序
		 */
		bool makeCombinations(std::unordered_map<int, std::list<int> >& table, int pointOrder1, int pointOrder2);

		/**
		 * 为出牌组合分配牌
		 * @param cardIds 牌id数组
		 * @param order 牌值顺序
		 * @param num 张数
		 * @param pointOrder1 第一张逢人配匹配的牌值顺序
		 * @param pointOrder2 第二张逢人配匹配的牌值顺序
		 * @param occupy 是否要占用
		 * @return 是否分配成功
		 */
		bool getCombinationCards(GuanDanRule* rule, std::vector<int>& cardIds, int order, int num, int& pointOrder1, int& pointOrder2, bool occupy = true);

		/**
		 * 从全部出牌组合表中找出满足条件的出牌组合
		 * @param genre 指定牌型
		 * @param officer 牌型主牌
		 * @param vec 返回满足条件的出牌组合
		 * @param 是否需要比较牌型主牌大小
		 */
		void getCombination(int genre, const PokerCard& officer, CombinationVec& vec, bool compareOfficer) const;

		/**
		 * 压牌出牌收集顺子、木板、钢板出牌组合
		 * @param rule 掼蛋规则
		 * @param genre 最新出牌牌型
		 * @param officerPoint 最新出牌牌型的主牌牌值
		 */
		void gatherStraight(GuanDanRule* rule, int genre, int officerPoint);

		/**
		 * 压牌出牌收集单张、对子、三张、三带二出牌组合
		 * @param rule 掼蛋规则
		 * @param genre 最新出牌牌型
		 * @param officer 最新出牌牌型的主牌
		 */
		void gatherOther(GuanDanRule* rule, int genre, const PokerCard& officer);

		/**
		 * 压牌出牌收集炸弹出牌组合
		 * @param rule 掼蛋规则
		 * @param genre 最新出牌牌型
		 * @param officerPoint 最新出牌牌型的主牌牌值
		 */
		void gatherBomb(GuanDanRule* rule, int genre, int officerPoint);

		/**
		 * 按顺序插入候选出牌组合
		 * @param comb 出牌组合
		 */
		void insertCandidate(const PokerCombination::Ptr& comb);

		/**
		 * 插入单张王或者对子王到候选出牌组合队列
		 * 实际上所做的就是插入到所有炸弹出牌组合前面
		 * @@param comb 出牌组合
		 */
		void insertCandidateJoker(const PokerCombination::Ptr& comb);

		/**
		 * 插入单张、对子、三张、三带二候选出牌组合到指定列表
		 * @param comb 出牌组合
		 * @param combList 指定出牌组合列表
		 * @param flag true:按主牌从小到大插入，false:按主牌从大到小插入
		 */
		void insertCandidateOther(const PokerCombination::Ptr& comb, CombinationList& combList, bool flag);

		/**
		 * 搜集所有同花顺，以便前端可以快速提示所有同花顺
		 */
		void collectStraightFlush();

	private:
		// 是否等待进贡
		bool _waitingPresentTribute;

		// 是否等待还贡
		bool _waitingRefundTribute;

		// 本局进贡目标玩家座位号
		int _presentTributeSeat;

		// 本局还贡目标玩家座位号
		int _refundTributeSeat;

		// 本局还贡提示文本
		std::string _refundTributeText;

		// 进贡或者还贡给出去的牌
		PokerCard _cardOut;

		// 收到进贡或者收到还贡拿回来的牌
		PokerCard _cardIn;

		// 逢人配张数
		int _variableCards;

		// 当前已抽取的逢人配张数
		int _occupiedVariableCards;

		// 2~A各牌值顺序的各花色数量表，0方块，1梅花，2红桃，3黑桃
		int _pointOrderSuitGraph[13][4];

		// 检索牌型Id
		int _searchId;

		// 检索牌型组合Id
		int _groupId;

		// 当前提示的同花顺索引
		mutable int _straightFlushIndex;

		// 检索牌型队列，按净值从大到小排列
		std::list<GuanDanSearch::Ptr> _searches;

		// 检索牌型组合队列，按净值从大到小排列
		std::list<GuanDanSearchGroup::Ptr> _groups;

		// 空闲检索牌型队列
		std::list<GuanDanSearch::Ptr> _freeSearches;

		// 空闲检索牌型组合队列
		std::list<GuanDanSearchGroup::Ptr> _freeGroups;

		// 当前已抽取的牌id集合
		std::unordered_set<int> _occupiedCardIds;

		// 所有同花顺的牌ID数组，每5个连续id组成一个同花顺，同一id可能会重复出现
		std::vector<int> _straightFlushCardIds;

		// 木板占用的牌值顺序表
		std::unordered_set<int> _woodOrders;

		// 钢板占用的牌值顺序表
		std::unordered_set<int> _steelOrders;
	};
}

#endif // !_NIU_MA_GUAN_DAN_AVATAR_H_