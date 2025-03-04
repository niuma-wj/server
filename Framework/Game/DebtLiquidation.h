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
	// ծ�������㷨
	// ���ڴ�����ծ�񡢻���ծ��ȸ����������ծ�����нڵ㲻���Գ���ʱ��������������
	// �㷨ʹ�ø�ʽ���˷���¼�ڵ���ʲ���ծ��

	/**
	 * ծ��ڵ�
	 */
	class DebtNode
	{
	public:
		DebtNode(int id, double capital);
		virtual ~DebtNode();

	public:
		// ����ID
		int getId() const;

		// �����ֽ��ܶ�
		double getCapital() const;

		// �����ֽ��ܶ�
		void setCapital(double cap);

		// �����ֽ�
		void addCapital(double cap);

		// ����
		void tally(DebtNode* node, double amount);

		// �ӽڵ��ջ�ȫ��Ƿ��
		bool receipt(DebtNode* node);

		// �������нڵ��ȫ��Ƿ��
		void repayAll();

		// ����ָ���ڵ��ȫ��Ƿ��
		bool repay(DebtNode* node);

		// ������ָ���ڵ���ծ���ϵ
		void erase(DebtNode* node);

		// ������ծ���������нڵ���ծ���ϵ
		void erase(const std::unordered_map<int, DebtNode*>& debtNet);

		// ���ظ�ծ��
		std::unordered_map<DebtNode*, double>& getDebtSheet();

		// ���ظ�ծ��
		const std::unordered_map<DebtNode*, double>& getDebtSheet() const;

		// ���ؾ���ծ(Ӧ���˿��ܶ��ȥǷ���ܶ�)
		double getDebt() const;

	private:
		// �ڵ�ID
		const int _id;

		// �ڵ���е��ֽ��ܶ�(����Ӧ���˿Ƿ��)
		double _capital;

		// ��ծ��ϵ��Ӧ���˿�(С��0)Ƿ��(����0)
		std::unordered_map<DebtNode*, double> _debtSheet;
	};

	/**
	 * ծ������
	 */
	class DebtLiquidation
	{
	public:
		DebtLiquidation();
		~DebtLiquidation();

		// �ݲ�ֵ
		static const double TOLERANCE;

	public:
		// ծ��������
		bool operator()(std::unordered_map<int, DebtNode*>& debtNet);

		// �ͷ�ծ����
		static void releaseDebtNet(std::unordered_map<int, DebtNode*>& debtNet);

		// ��ӡծ�������ַ����У�������������ȷ���㽫ԭʼծ�����������־��
		// �Ա�����㷨
		static void printDebtNet(const std::unordered_map<int, DebtNode*>& debtNet, std::string& text);
	};
}

#endif