// LackeyAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.31

#ifndef _NIU_MA_LACKEY_AVATAR_H_
#define _NIU_MA_LACKEY_AVATAR_H_

#include "PokerGenre.h"
#include "PokerAvatar.h"

namespace NiuMa
{
	class LackeyRule;
	// 逮狗腿玩家替身
	class LackeyAvatar : public PokerAvatar
	{
	public:
		LackeyAvatar(const std::shared_ptr<LackeyRule>& rule, const std::string& playerId, int seat, bool robot);
		virtual ~LackeyAvatar();

	public:
		// 清理
		virtual void clear() override;

	protected:
		// 检索所有牌型的全部组合
		virtual void combineAllGenres() override;

		// 更新候选组合列表
		virtual void candidateCombinationsImpl(int situation = 0) override;

		// 更新候选组合列表
		virtual void candidateCombinationsImpl(const PokerGenre& pg, int situation = 0) override;

	public:
		// 设置地主1打4时的狗腿牌ID
		void setLackeyCard(int id);

		// 返回狗腿炸ID
		int getLackeyCard() const;

		// 增加喜钱
		void addXiQian(int q);

		// 返回喜钱
		int getXiQian() const;

		// 计算剩余手牌的喜钱
		void calcLeftXiQian();

	protected:
		// 检索指定牌型的全部组合
		void combineGenre(int genre);

		// 检索蝴蝶牌型组合
		void combineButterfly(int genre);

		// 检索3带2牌型组合
		void combineButterfly();

		// 完成蝴蝶牌型组合
		void completeButterfly();

		// 检索三顺牌型组合
		void combineTriple(int genre);

		// 检索连对牌型组合
		void combinePair(int genre);

		// 检索对子牌型组合
		void combinePair();

		// 检索单张牌型组合
		void combineSingle();

		// 检索炸弹牌型组合
		void combineBomb(int genre);

		// 检索王炸牌型组合
		void combineBombJoker(int genre);

		// 检查狗腿炸牌型组合
		void combineBombLackey();

		// 添加候选同牌型组合
		void addCandidate(const PokerGenre& pg);

		// 添加候选同牌型组合
		void addCandidateSort(const PokerGenre& pg);

		// 添加候选炸弹组合
		void addCandidateBomb(int order);

	protected:
		// 地主1打4时的狗腿牌ID否则为-1(即便是狗腿玩家也不设置该值)
		int _lackeyCard;

		// 玩家当前获得的喜钱数量
		int _xiQian;
	};
}

#endif // !_NIU_MA_LACKEY_AVATAR_H_