// GuanDanCandidateOrder.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.08.21

#ifndef _NIU_MA_GUAN_DAN_CANDIDATE_ORDER_H_
#define _NIU_MA_GUAN_DAN_CANDIDATE_ORDER_H_

#include "Base/Singleton.h"

#include <unordered_map>

namespace NiuMa
{
	class GuanDanCandidateOrder : public Singleton<GuanDanCandidateOrder>
	{
	private:
		GuanDanCandidateOrder() {
			makeOrder1();
			makeOrder2();
		}

	public:
		virtual ~GuanDanCandidateOrder() = default;

		friend class Singleton<GuanDanCandidateOrder>;

	private:
		// 构建前期出牌顺序表
		void makeOrder1();

		// 构建末期出牌顺序表
		void makeOrder2();

		//
		int makeKey(int genre, int officerPoint, int gradePoint) const;

		//
		int getOrderImpl(int key, const std::unordered_map<int, int>& orderMap) const;

	public:
		/**
		 * 获取前期出牌组合候选顺序，顺序越小越先出
		 * @param genre 牌型
		 * @param officerPoint 主牌牌值
		 * @param gradePoint 当前级牌牌值，-1表示不考虑级牌大小
		 * @return 顺序
		 */
		int getOrder1(int genre, int officerPoint, int gradePoint = -1) const;

		/**
		 * 获取前期出牌组合候选顺序，顺序越小越先出
		 * @param genre 牌型
		 * @param officerPoint 主牌牌值
		 * @param gradePoint 当前级牌牌值，-1表示不考虑级牌大小
		 * @return 顺序
		 */
		int getOrder2(int genre, int officerPoint, int gradePoint) const;

	private:
		// 前期候选顺序表
		std::unordered_map<int, int> _orders1;

		// 后期候选顺序表
		std::unordered_map<int, int> _orders2;
	};
}

#endif // !_NIU_MA_GUAN_DAN_CANDIDATE_ORDER_H_