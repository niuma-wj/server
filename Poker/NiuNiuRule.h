// NiuNiuRule.h
// Author wujian
// Email 393817707@qq.com
// Date 2025.02.13

#ifndef _NIU_MA_NIU_NIU_RULE_H_
#define _NIU_MA_NIU_NIU_RULE_H_

#include "PokerRule.h"

namespace NiuMa
{
	// ����
	enum class NiuNiuGenre : int
	{
		Invalid,				// ��Ч����
		Niu0,					// ûţ
		Niu1,					// ţһ
		Niu2,					// ţ��
		Niu3,					// ţ��
		Niu4,					// ţ��
		Niu5,					// ţ��
		Niu6,					// ţ��
		Niu7,					// ţ��
		Niu8,					// ţ��
		Niu9,					// ţ��
		NiuNiu = 0x0010,		// ţţ
		ShunZi = 0x0020,		// ˳��ţ
		WuHua = 0x0040,			// �廨ţ
		TongHua = 0x0060,		// ͬ��ţ
		HuLu = 0x0080,			// ��«ţ
		ZhaDan = 0x00A0,		// ը��ţ
		WuXiao = 0x00C0,		// ��Сţ
		KaiXin = 0x00E0			// ����ţ
	};

	// ţţ��Ϸ����(֧������ţţ�淨������ţţ������ţţ)
	// ����ţţ�����ţţ���������ڣ�����ţţ֧��˳��ţ���廨ţ��ͬ��ţ����«ţ��
	// ը��ţ����Сţ������ţ��������ţţû�д�С���ƣ��ҽ���֧���廨ţ��ը��ţ
	class NiuNiuRule : public PokerRule
	{
	public:
		NiuNiuRule(bool niu100);
		virtual ~NiuNiuRule();

	protected:
		const bool _niu100;			// �Ƿ�Ϊ����ţţ
		static const int GENRE_ORDER_NIU100[13];		// ����ţţ���ʹ�С˳���(��С����)
		
	public:
		// �ж�����
		virtual int predicateCardGenre(PokerGenre& pcg) const override;

		// �ж���������pcg1�Ƿ����pcg2������0��������Ȼ����޷��Ƚϣ�����1��ǰ�ߴ��ں��ߣ�����2�����ߴ���ǰ��
		virtual int compareGenre(const PokerGenre& pcg1, const PokerGenre& pcg2) const override;

		// ˳�������ų�����(����һ��˳�Ӷ����ܴ�����)
		virtual bool straightExcluded(const PokerCard& c) const override;

	private:
		// �ж�����ţţ����
		int predicateCardGenre1(PokerGenre& pcg) const;

		// �ж�����ţţ����
		int predicateCardGenre2(PokerGenre& pcg) const;

		// ����1-9��Ӧ�ķ�ţ����
		int getNotNiuGenre(int nRemainder) const;

		// ���������ڰ���ţţ˳����е�λ��
		static int getGenreOrderNiu100(int genre);
	};
}

#endif // _NIU_MA_NIU_NIU_RULE_H_