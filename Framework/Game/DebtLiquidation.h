// DebtLiquidation.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.05

#ifndef _NIU_MA_DEBT_LIQUIDATION_H_
#define _NIU_MA_DEBT_LIQUIDATION_H_

#include <string>
#include <unordered_map>

namespace NiuMa
{
	// 债务清算算法
	// 用于处理多边债务、环形债务等复杂情况，当债务网中节点不足以偿付时，按比例偿付。
	// 算法使用复式记账法记录节点的资产负债。

	/**
	 * 债务节点
	 */
	class DebtNode
	{
	public:
		DebtNode(int id, double capital);
		virtual ~DebtNode();

	public:
		// 返回ID
		int getId() const;

		// 返回现金总额
		double getCapital() const;

		// 设置现金总额
		void setCapital(double cap);

		// 增加现金
		void addCapital(double cap);

		// 记账
		void tally(DebtNode* node, double amount);

		// 从节点收回全部欠款
		bool receipt(DebtNode* node);

		// 偿还所有节点的全部欠款
		void repayAll();

		// 偿还指定节点的全部欠款
		bool repay(DebtNode* node);

		// 消除与指定节点间的债务关系
		void erase(DebtNode* node);

		// 消除与债务网内所有节点间的债务关系
		void erase(const std::unordered_map<int, DebtNode*>& debtNet);

		// 返回负债表
		std::unordered_map<DebtNode*, double>& getDebtSheet();

		// 返回负债表
		const std::unordered_map<DebtNode*, double>& getDebtSheet() const;

		// 返回净负债(应收账款总额减去欠款总额)
		double getDebt() const;

	private:
		// 节点ID
		const int _id;

		// 节点持有的现金总额(不算应收账款及欠款)
		double _capital;

		// 负债关系表，应收账款(小于0)欠款(大于0)
		std::unordered_map<DebtNode*, double> _debtSheet;
	};

	/**
	 * 债务清算
	 */
	class DebtLiquidation
	{
	public:
		DebtLiquidation();
		~DebtLiquidation();

		// 容差值
		static const double TOLERANCE;

	public:
		// 债务网清算
		bool operator()(std::unordered_map<int, DebtNode*>& debtNet);

		// 释放债务网
		static void releaseDebtNet(std::unordered_map<int, DebtNode*>& debtNet);

		// 打印债务网到字符串中，若清算结果不正确方便将原始债务网输出到日志中
		// 以便测试算法
		static void printDebtNet(const std::unordered_map<int, DebtNode*>& debtNet, std::string& text);
	};
}

#endif