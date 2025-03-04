// BiJiAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.25

#ifndef _NIU_MA_BI_JI_AVATAR_H_
#define _NIU_MA_BI_JI_AVATAR_H_

#include "Game/GameAvatar.h"
#include "PokerGenre.h"
#include "BiJiRule.h"

#include <unordered_set>
#include <unordered_map>

namespace NiuMa
{
	class BiJiRule;
	class BiJiAvatar : public GameAvatar
	{
	public:
		BiJiAvatar(const std::string& playerId, int seat, bool robot);
		virtual ~BiJiAvatar();

	public:
		// 清理
		void clear();

		// 
		void setJoinRound();

		// 
		bool isJoinRound() const;

		// 发牌
		void dealCards(const CardArray& cards, const std::shared_ptr<BiJiRule>& rule);

		// 返回牌数组
		const CardArray& getCards() const;

		// 
		void getOrderRaw(int arr[]) const;

		// 
		void getOrderSuit(int arr[]) const;

		//
		const PokerGenre& getDun(int dun) const;

		/**
		 * 配墩
		 * @param dun 墩号，0、1、2
		 * @param ids 该墩的三张牌id
		 * @param rule 规则
		 */
		bool makeDun(int dun, int ids[], const std::shared_ptr<BiJiRule>& rule);

		/**
		 * 撤销墩
		 * @param dun 墩号，0、1、2
		 */ 
		bool revocateDun(int dun);

		/**
		 * 查询墩是否已配置
		 * @param dun 墩号，0、1、2
		 */
		bool isDunOK(int dun) const;

		/**
		 * 查询指定墩的牌id
		 * @param dun 墩号，0、1、2
		 * @param ids 返回三张牌id
		 */
		bool getDunIds(int dun, int ids[]) const;

		// 
		void checkSupportGenre(const std::shared_ptr<BiJiRule>& rule);

		// 
		bool isSupportGenre(BiJiGenre genre) const;

		// 
		bool getGenreIds(BiJiGenre genre, int ids[], const std::shared_ptr<BiJiRule>& rule) const;

		/**
		 * 对三墩牌进行排序
		 * @param duns 返回三墩牌是否被调整位置
		 * @param rule 规则 
		 */
		bool sortDuns(bool duns[], const std::shared_ptr<BiJiRule>& rule);

		// 
		void setFixed();

		// 
		bool isFixed() const;

		// 
		void setQiPaiOrder(int order);

		// 
		int getQiPaiOrder() const;

		//
		bool isGiveUp() const;

		//
		void setDunScore(int d, int s);

		//
		int getDunScore(int d) const;

		void setWinGold(int64_t g);

		int64_t getWinGold() const;

		// 检测奖励类型(在比牌之后)
		void detectRewardType(const std::shared_ptr<BiJiRule>& rule);

		// 
		int getRewardType() const;

		//
		void setReward(int r);

		//
		int getReward() const;

		// 返回总分
		int getTotal() const;

		/**
		 * 添加离线游戏局数
		 */
		void addOfflines();

		/**
		 * 清除离线游戏局数
		 */
		void emptyOfflines();

		/**
		 * 获取离线游戏局数
		 */
		int getOfflines() const;

	private:
		// 自动创建最后一墩
		void finalDun(const std::shared_ptr<BiJiRule>& rule);

		// 获取最大的3条牌型的所有牌ID
		bool get3TiaoIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const;

		// 获取最大的顺子牌型的所有牌ID
		// 参数tongHua是否同花顺
		bool getShunZiIds(bool tongHua, int ids[], const std::shared_ptr<BiJiRule>& rule) const;

		// 获取最大的同花牌型的所有牌ID
		bool getTongHuaIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const;

		// 获取最大的对子牌型的所有牌ID
		bool getDuiZiIds(int ids[], const std::shared_ptr<BiJiRule>& rule) const;

		// 获取最大的乌龙牌型的所有牌ID
		bool getWuLongIds(int ids[]) const;

		//
		void calcGenreOrders(int orders[], const std::shared_ptr<BiJiRule>& rule) const;

	private:
		// 全部牌
		CardArray _cards;

		// 3墩牌
		PokerGenre _duns[3];

		// 已被使用的牌ID
		std::unordered_set<int> _occupiedIds;

		// 牌ID按原始顺序排列
		int _orderRaw[9];

		// 牌ID按花色排列
		int _orderSuit[9];

		// 是否参加本局游戏
		bool _joinRound;

		// 各墩是否已经配好
		bool _dunOKs[3];

		// 是否已经确认(固定)墩
		bool _fixed;

		// 弃牌顺序
		int _qiPaiOrder;

		// 3墩牌得分
		int _scores[3];

		// 一局中3墩牌总输赢金币
		int64_t _winGold;

		// 奖励类型
		int _rewardType;

		// 奖励分
		int _reward;

		// 离线后参加的游戏局数，超过3局自动踢出牌桌
		int _offlines;

		// 牌对应的ID
		std::unordered_map<int, int> _cardIds;

		int _points[14];

		bool _suits[14][4];

		// 支持的牌型，依次为3条、同花顺、同花、顺子、对子
		bool _supportedGenres[5];
	};
}

#endif