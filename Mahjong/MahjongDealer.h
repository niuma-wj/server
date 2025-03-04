// MahjongDealer.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.03

#ifndef _NIU_MA_MAHJONG_DEALER_H_
#define _NIU_MA_MAHJONG_DEALER_H_

#include "MahjongTile.h"

#include <map>

namespace NiuMa
{
	/**
	 * �齫��������ÿ��������һ��������
	 */
	class MahjongTileGenerator;
	class MahjongDealer
	{
	public:
		MahjongDealer(bool bFlower = false);
		virtual ~MahjongDealer();

	protected:
		/**
		 * �Ƿ������
		 */
		const bool _flower;

		/**
		 * һ�����������Ƶ�������(������136������144)
		 */
		const int _totalTileNums;

		/**
		 * �Ƴأ�����ʣ��δ���������
		 */
		int _tilePool[144];

		/**
		 * �Ƴ���ʣ���Ƶ���ʼλ��
		 */
		int _start;

		/**
		 * �Ƴ���ʣ���Ƶ���ֹλ��(ʣ������һ�������ڵ�λ��+1)
		 */
		int _end;

	private:
		/**
		 * ��������
		 */
		std::shared_ptr<MahjongTileGenerator> _generator;

	public:
		// �����Ƿ������
		bool hasFlower() const;

		// ����ϴ��
		void shuffle();

		// ����ϴ�ƣ��ڳ�ʼ��������ҵ�����֮����ϴ�Ƴ�(�����������ڲ���)
		void shuffle(const std::map<int, int>& initTiles);

		// �����Ƶ�������
		int getTotalTileNums() const;

		// ����ʣ���Ƶ�����
		int getTileLeft() const;

		// �����Ƴ��Ƿ��Ѿ�Ϊ��
		bool isEmpty() const;

		// ��ID�����
		bool getTile(MahjongTile& mt) const;

		// ���Ƴص���ʼλ�ô�ȡ��һ����
		bool fetchTile(MahjongTile& mt);

		// ���Ƴص���ֹλ�ô�ȡ��һ����
		bool fetchTile1(MahjongTile& mt);

		// ���Ƴ���ȡ��ָ����һ����
		bool fetchTile(MahjongTile& mt, const std::string& str);

		// ��ID�����
		static bool getTileById(MahjongTile& mt);

		// ���ƻ��ID(��һ��)
		static bool getIdByTile(MahjongTile& mt);
	};
}

#endif