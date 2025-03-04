// DouDiZhuAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_DOU_DI_ZHU_AVATAR_H_
#define _NIU_MA_DOU_DI_ZHU_AVATAR_H_

#include "Game/GameAvatar.h"
#include "DouDiZhuRule.h"
#include "DouDiZhuCombination.h"

namespace NiuMa
{
	/**
	 * 斗地主类游戏玩家替身
	 */
	class DouDiZhuAvatar : public GameAvatar
	{
	public:
		DouDiZhuAvatar(const PokerRule::Ptr& rule, const std::string& playerId, int seat, bool robot);
		virtual ~DouDiZhuAvatar();

		typedef std::vector<DouDiZhuCombination::Ptr> CombinationVec;
		typedef std::unordered_map<int, CombinationVec> CombinationMap;

	protected:
		// 规则
		const PokerRule::Ptr _rule;

		// 手牌
		CardArray _cards;

		// 玩家最新打出的牌型
		PokerGenre _outtedGenre;

		// 全部组合
		CombinationMap _combinations;

		// 所有牌值的牌ID
		std::vector<int> _pointCards[14];

		// 小大王牌ID，有些斗地主玩法需要区分大小王
		std::vector<int> _jokerCards[2];

		// 候选组合列表
		CombinationVec _candidates;

		// 各牌值牌数量
		int _pointNums[14];

		// 候选组合列表的索引
		int _candidatePos;

		// 手牌是否发生变化，如新发牌或出牌，每次分析组合之后该值设置为false
		bool _cardsUpdated;

		// 当局内是否打出过牌(包括"不要")
		bool _played;

	private:
		int _combId;
		CombinationVec _allCombs;
		CombinationVec _freeCombs;

	protected:
		// 分配组合
		DouDiZhuCombination::Ptr allocateCombination();

		// 回收组合
		void freeCombination(const DouDiZhuCombination::Ptr& comb);

		// 清理当前分析数据
		void clearAnalysis();

		// 清空候选列表
		void clearCandidates();

		// 添加牌型组合
		void addCombination(int genre, const DouDiZhuCombination::Ptr& comb);

		/**
		 * 获取nums张指定牌值的牌ID
		 * @param order 指定牌值在顺序表中的位置索引
		 * @param nums 张数
		 * @param ids 返回牌id列表
		 * @return 是否成功获取
		 */
		bool getCardIds(int order, int nums, std::vector<int>& ids) const;

	protected:
		// 检索所有牌型的全部组合
		virtual void combineAllGenres() = 0;

		// 更新候选组合列表
		virtual void candidateCombinationsImpl() = 0;

		// 更新候选组合列表
		virtual void candidateCombinationsImpl(const PokerGenre& pg) = 0;

	public:
		// 设置手牌
		void setCards(const CardArray& cards);

		// 获得手牌
		const CardArray& getCards() const;

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

		// 删除指定id的牌
		void removeCardsByIds(const std::vector<int>& ids);

		// 返回牌ID数组，按相同牌值的张数大到小排序
		void cardIdsSortedByPoints(std::vector<int>& ids) const;

		// 分析全部合法组合
		void analyzeCombinations();

		// 更新候选组合列表
		void candidateCombinations();

		// 更新候选组合列表
		void candidateCombinations(const PokerGenre& pg);

		// 是否有可出的候选组合
		bool hasCandidate() const;

		// 获得候选组合的牌
		bool getCandidateCards(std::vector<int>& ids);

		// 获得第一个候选组合的牌型及牌ID
		DouDiZhuCombination::Ptr getFirstCandidate() const;

		// 是否出过牌
		bool isPlayed() const;

	public:
		// 清理
		virtual void clear();
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

#endif