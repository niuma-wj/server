// PokerAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_POKER_AVATAR_H_
#define _NIU_MA_POKER_AVATAR_H_

#include "Game/GameAvatar.h"
#include "PokerRule.h"
#include "PokerCombination.h"

#include <list>

namespace NiuMa
{
	/**
	 * 扑克类游戏玩家替身
	 */
	class PokerAvatar : public GameAvatar
	{
	public:
		PokerAvatar(const PokerRule::Ptr& rule, const std::string& playerId, int seat, bool robot);
		virtual ~PokerAvatar();

		typedef std::list<PokerCombination::Ptr> CombinationList;
		typedef std::vector<PokerCombination::Ptr> CombinationVec;
		typedef std::unordered_map<int, CombinationList> CombinationMap;

	public:
		// 设置手牌
		void setCards(const CardArray& cards);

		// 获得手牌
		const CardArray& getCards() const;

		// 排序手牌
		void sortCards();

		// 获得手牌数量
		int getCardNums() const;

		// 设置最新打出牌型
		void setOuttedGenre(const PokerGenre& g);

		// 返回最新打出牌型
		const PokerGenre& getOuttedGenre() const;

		// 手牌数组中是否指定ID的牌
		bool hasCard(int id) const;

		// 获得指定id的牌
		bool getCardsByIds(const std::vector<int>& ids, CardArray& cards) const;

		// 获得指定id的牌
		bool getCardById(int id, PokerCard& c) const;

		// 添加牌
		void addCard(const PokerCard& c);

		// 删除指定id的牌
		void removeCardById(int id);

		// 删除指定id的牌
		void removeCardsByIds(const std::vector<int>& ids);

		// 返回牌ID数组，按相同牌值的张数大到小排序
		void cardIdsSortedByPoints(std::vector<int>& ids) const;

		// 手牌是否已更新
		bool isCardsUpdated() const;

		// 分析全部合法出牌组合
		void analyzeCombinations();

		// 返回出牌组合数量
		int getCombinationNums() const;

		/**
		 * 更新候选组合列表，首位出牌
		 * @param situation 当前情形，例如是否已经打到后期需要抗牌
		 */
		void candidateCombinations(int situation = 0);

		/**
		 * 更新候选组合列表，压牌出牌
		 * @param pg 牌桌上最新打出的出牌组合
		 * @param situation 当前情形，例如是否已经打到后期需要抗牌
		 */
		void candidateCombinations(const PokerGenre& pg, int situation = 0);

		// 是否有可出的候选组合
		bool hasCandidate() const;

		// 获得候选组合的牌
		bool getCandidateCards(std::vector<int>& ids);

		// 获得第一个候选组合的牌型及牌ID
		PokerCombination::Ptr getFirstCandidate() const;

		// 从头开始提示
		void resetCandidatePos();

		// 是否出过牌
		bool isPlayed() const;

		// 重置出牌标志
		void resetPlayed();

	public:
		// 清理
		virtual void clear();

	protected:
		// 分配组合
		PokerCombination::Ptr allocateCombination();

		// 回收组合
		void freeCombination(const PokerCombination::Ptr& comb);

		// 清空候选列表
		void clearCandidates();

		/**
		 * 插入出牌组合
		 * @param comb 出牌组合
		 * @param orderByPoint 是否按主牌牌值顺序(由小到大)排序，true-是，false-按牌大小(由小到大)排序
		 */
		void insertCombination(const PokerCombination::Ptr& comb, bool orderByPoint = false);

		/**
		 * 将出牌组合插入到列表中正确位置，插入后按净值由大到小、主牌由小到大排序
		 * @param combList 出牌组合列表
		 * @param comb 出牌组合
		 * @param orderByPoint 是否按主牌牌值顺序(由小到大)排序，true-是，false-按牌大小(由小到大)排序
		 * @return 是否插入成功
		 */
		bool insertCombination(CombinationList& combList, const PokerCombination::Ptr& comb, bool orderByPoint = false) const;

		/**
		 * 获取nums张指牌值顺序的牌ID
		 * @param order 指定牌值在顺序表中的位置索引
		 * @param nums 张数
		 * @param ids 返回牌id列表
		 * @return 是否成功获取
		 */
		bool getCardIds(int order, int nums, std::vector<int>& ids) const;

		/**
		 * 从抽取一张指定牌值的牌的id
		 * @param excludedIds 排除的id集合，例如这些id的牌已被抽取
		 * @param order 指定牌值在顺序表中的位置索引
		 * @return 牌id，-1表示无法抽取指定的牌
		 */
		int getCardId(const std::unordered_set<int>& excludedIds, int order);

	protected:
		// 清理当前分析数据
		virtual void clearAnalysis();

		/**
		 * 分析组合时是否要忽略指定的牌
		 * @param c 指定的牌
		 * @return true-忽略，false-不忽略
		 */
		virtual bool analyzeIgnore(const PokerCard& c);

		// 检索所有牌型的全部组合
		virtual void combineAllGenres() = 0;

		/**
		 * 更新候选组合列表，首位出牌
		 * @param situation 当前情形，例如是否已经打到后期需要抗牌
		 */
		virtual void candidateCombinationsImpl(int situation = 0) = 0;

		/**
		 * 更新候选组合列表，压牌出牌
		 * @param pg 牌桌上最新打出的出牌组合
		 * @param situation 当前情形，例如是否已经打到后期需要抗牌
		 */
		virtual void candidateCombinationsImpl(const PokerGenre& pg, int situation = 0) = 0;

	protected:
		// 规则
		const PokerRule::Ptr _rule;

		// 手牌
		CardArray _cards;

		// 玩家最新打出的牌型
		PokerGenre _outtedGenre;

		// 全部出牌组合表
		CombinationMap _combinations;

		// 所有牌值顺序的牌ID
		std::vector<int> _pointOrderCards[14];

		// 小大王牌ID，有些斗地主玩法需要区分大小王
		// 0-小王，1-大王
		std::vector<int> _jokerCards[2];

		// 候选组合列表
		CombinationVec _candidates;

		// 各牌值顺序数量
		int _pointOrderNums[14];

		// 候选组合列表的索引
		int _candidatePos;

		// 手牌是否发生变化，如新发牌或出牌，每次分析组合之后该值设置为false
		bool _cardsUpdated;

		// 当局内是否打出过牌(包括"不要")
		bool _played;

	private:
		int _combId;
		CombinationVec _freeCombs;
	};

	class SortByCombines
	{
	public:
		SortByCombines(const std::unordered_map<int, int>& map);
		virtual ~SortByCombines();

	private:
		const std::unordered_map<int, int>& _cardCombines;

	public:
		bool operator()(const int& id1, const int& id2) const;
	};
}

#endif // !_NIU_MA_POKER_AVATAR_H_