// PokerUtilities.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.24

#ifndef _NIU_MA_POKER_UTILITIES_H_
#define _NIU_MA_POKER_UTILITIES_H_

#include "PokerGenre.h"

#include <unordered_set>

namespace NiuMa
{
	/**
	 * 扑克实用工具函数
	 */
	class PokerUtilities
	{
	private:
		PokerUtilities();
		virtual ~PokerUtilities();

	public:
		/**
		 * 计算牌数组中包含多少张指定牌值的牌
		 * @param cards 牌数组
		 * @param point 牌值
		 * @return 牌数组中指定牌值的牌数量
		 */
		static int getPointNums(const CardArray& cards, int point);

		/**
		 * 计算牌数组中包含多少张指定花色的牌
		 * @param cards 牌数组
		 * @param suit 花色
		 * @return 牌数组中指定花色的牌数量
		 */
		static int getSuitNums(const CardArray& cards, int suit);

		/**
		 * 计算牌数组中包含多少张指定的牌
		 * @param cards 牌数组
		 * @param point 牌值
		 * @param suit 花色
		 * @return 牌数组中指定的牌数量
		 */
		static int getCardNums(const CardArray& cards, int point, int suit);

		/**
		 * 计算牌数组中有多少种不同类型的牌值，有时候需要忽略掉特定的牌、牌值或花色，
		 * 例如当某个牌是万变牌的时候
		 * @param cards 牌数组(已从小到大排序)
		 * @param ignorePoint 忽略的牌值，小于等于0表示不忽略任何牌值
		 * @param ignoreSuit 忽略的花色，小于等于0表示不忽略任何花色
		 * @return 牌数组中牌值的种类数量
		 */
		static int getPointTypes(const CardArray& cards, int ignorePoint = 0, int ignoreSuit = 0);

		/**
		 * 从牌数组中统计出牌值的曲线图
		 * @param cards 牌数组(已从小到大排序)
		 * @param graph 返回曲线图，first-牌值，second-对应的牌数量
		 * @param ignorePoint 忽略的牌值，小于等于0表示不忽略任何牌值
		 * @param ignoreSuit 忽略的花色，小于等于0表示不忽略任何花色
		 * @param comp 牌值比较器
		 */
		static void getPointGraph(const CardArray& cards, std::vector<std::pair<int, int> >& graph,
			int ignorePoint = 0, int ignoreSuit = 0, const PointComparator::Ptr& comp = PointComparator::Ptr());

		/**
		 * 牌数组是否为统一牌值
		 * @param cards 牌数组
		 * @param ignorePoint 忽略的牌值，小于等于0表示不忽略任何牌值
		 * @param ignoreSuit 忽略的花色，小于等于0表示不忽略任何花色
		 * @return 是-true，否-false
		 */
		static bool samePoint(const CardArray& cards, int ignorePoint = 0, int ignoreSuit = 0);

		/**
		 * 牌数组是否为统一花色
		 * @param cards 牌数组
		 * @param ignorePoint 忽略的牌值，小于等于0表示不忽略任何牌值
		 * @param ignoreSuit 忽略的花色，小于等于0表示不忽略任何花色
		 * @return 是-true，否-false
		 */
		static bool sameSuit(const CardArray& cards, int ignorePoint = 0, int ignoreSuit = 0);

		/**
		 * 从牌数组中查找首次遇到的连续N个相同牌值的牌，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param c 返回数组中首次遇到的连续N个相同牌值的牌
		 * @param N_ 数量N
		 * @return 是否找到
		 */
		static bool findSamePointN(const CardArray& cards, PokerCard& c, int N_);
		/**
		 * 从牌数组中反向查找首次遇到的连续N个相同牌值的牌，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param c 返回数组中首次遇到的连续N个相同牌值的牌
		 * @param N_ 数量N
		 * @return 是否找到
		 */
		static bool rfindSamePointN(const CardArray& cards, PokerCard& c, int N_);

		/**
		 * 从牌值数组中抽出首次遇到的连续N个相同的牌值，牌值数组已经排好序(从小到大)
		 * 注意该方法并不从cardPoints中删除元素，而是将抽出的元素的位置置0
		 * @param cardPoints 牌值数组(已排序)
		 * @param point 返回数组中首次遇到的连续N个相同的牌值
		 * @param N_ 数量N
		 * @return 是否找到
		 */
		static bool extractSamePointN(std::vector<int>& cardPoints, int& point, int N_);

		/**
		 * 计算牌数组中与指定牌相同(牌值和花色相同)的牌的数量，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param c 指定的牌
		 * @return 数量
		 */
		static int countSameCard(const CardArray& cards, const PokerCard& c);

		/**
		 * 判定牌数组中是否包含指定牌值的牌
		 * @param cards 牌数组
		 * @param point 牌值
		 * @return 是否包含
		 */
		static bool hasPoint(const CardArray& cards, int point);

		/**
		 * 判定牌数组中是否包含指定花色的牌
		 * @param cards 牌数组
		 * @param point 花色
		 * @return 是否包含
		 */
		static bool hasSuit(const CardArray& cards, int suit);

		/**
		 * 判定牌数组中是否包含指定ID的牌
		 * @param cards 牌数组
		 * @param id 牌id
		 * @return 是否包含
		 */
		static bool hasCard(const CardArray& cards, int id);

		/**
		 * 判定牌数组中是否有指定牌值及花色的牌
		 * @param cards 牌数组
		 * @param point 牌值
		 * @param suit 花色
		 * @return true-有，false-无
		 */
		static bool hasCard(const CardArray& cards, int point, int suit);

		/**
		 * 从牌数组中查找第一个牌值为指定牌值的牌，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param c 返回牌数组中第一个牌值为指定牌值的牌
		 * @param point 指定牌值
		 * @return 是否找到
		 */
		static bool getFirstCardOfPoint(const CardArray& cards, PokerCard& c, int point);

		/**
		 * 从牌数组中查找最后一个牌值为指定牌值的牌，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param c 返回牌数组中第一个牌值为指定牌值的牌
		 * @param point 指定牌值
		 * @return 是否找到
		 */
		static bool getLastCardOfPoint(const CardArray& cards, PokerCard& c, int point);

		/**
		 * 从传入牌数组中获得指定id的牌
		 * @param cardsIn 传入牌数组
		 * @param cardsOut 返回获取到的牌
		 * @param ids 牌ID数组
		 * @return 是否获取到全部指定id的牌
		 */ 
		static bool getCardsByIds(const CardArray& cardsIn, CardArray& cardsOut, const std::vector<int>& ids);

		/**
		 * 从牌数组中获得指定id的牌
		 * @param cards 牌数组
		 * @param id 指定的牌ID
		 * @param c 返回指定的牌
		 * @return 是否找到
		 */
		static bool getCardById(const CardArray& cards, int id, PokerCard& c);

		/**
		 * 从牌数组获取指定牌值及花色的所有牌
		 * @param cards 牌数组
		 * @param results 返回所有指定牌值及花色的牌
		 * @param point 指定牌值，-1表示不指定牌值
		 * @param suit 指定花色，-1表示不指定花色
		 */
		static void getCards(const CardArray& cards, CardArray& results, int point, int suit = -1);

		/**
		 * 从牌数组获取指定牌值及花色的所有牌的id
		 * @param cards 牌数组
		 * @param ids 返回所有指定牌的id
		 * @param point 指定牌值，-1表示不指定牌值
		 * @param suit 指定花色，-1表示不指定花色
		 */
		static void getCardIds(const CardArray& cards, std::vector<int>& ids, int point, int suit = -1);

		/**
		 * 从牌数组获取一张指定牌值及花色的牌的id
		 * @param cards 牌数组
		 * @param excludedIds 排除的id集合，例如这些id的牌已被抽取
		 * @param point 指定牌值，-1表示不指定牌值
		 * @param suit 指定花色，-1表示不指定花色
		 * @return 牌id，-1表示无法抽取指定的牌
		 */
		static int getCardId(const CardArray& cards, const std::unordered_set<int>& excludedIds, int point, int suit = -1);

		/**
		 * 从牌数组查找顺子、连对、三顺等牌型，牌数组已经排好序(从小到大)
		 * @param cards 牌数组(已排序)
		 * @param results 返回获取的牌
		 * @param point 起始牌值
		 * @param straight 连续级数
		 * @param N_ 每级的牌数量，例如顺子该参数为1，连对为2，三顺则为3等
		 * @param rule 规则
		 * @return 是否找到指定牌型
		 */
		static bool getStraightCards(const CardArray& cards, CardArray& results, int point, int straight, int N_, const std::shared_ptr<PokerRule>& rule);

		/**
		 * 将指定牌值的牌移动到牌数组的前面(后面)
		 * @param cards 牌数组
		 * @param point 指定牌值
		 * @param front 移动到前面(true)，后面(false)
		 */
		static void moveCardsOfPoint2FrontBack(CardArray& cards, int point, bool front);

		// 牌数组1减去牌数组2
		static void cardArraySubtract(CardArray& cards1, const CardArray& cards2);

		// 牌数组转到字符串
		static void cardArray2String(const CardArray& cards, std::string& str);

		// 计算牌数组(已排序)中大小王的数量
		static void getJokerSuitNums(const CardArray& cards, int& big, int& little);

		// 根据牌值数量对牌数组(已按牌大小排序)进行重新排序
		static void sortCardsByPoints(const CardArray& cardsIn, CardArray& cardsOut);

		// 计算牌数组里面黑色及红色的牌数量(小王为黑色大王为红色)
		static void getBlackRedNums(const CardArray& cards, int& black, int& red);

		/**
		 * 从牌数组中获取除忽略的牌外最大的牌
		 * @param cards 牌数组，从小到大排序
		 * @param c 返回最大的牌
		 * @param ignorePoint 忽略的牌值，小于等于0表示不忽略任何牌值
		 * @param ignoreSuit 忽略的花色，小于等于0表示不忽略任何花色
		 * @return 找到-true，否则-false
		 */
		static bool getBiggestCard(const CardArray& cards, PokerCard& c, int ignorePoint = 0, int ignoreSuit = 0);
	};
}

#endif