// MahjongTile.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_TILE_H_
#define _NIU_MA_MAHJONG_TILE_H_

#include <vector>

#include "msgpack/msgpack.hpp"

namespace NiuMa
{
	/**
	 * �齫��
	 */
	class MahjongTile
	{
	public:
		// ������:Ͳ���������֡����������Ƶ�ID�����������ͺ�����ֵ������
		enum class Pattern : int
		{
			Invalid = 0,// ��Ч
			Tong,		// Ͳ��
			Tiao,		// ����
			Wan,		// ����
			Dong,		// ����
			Nan,		// �Ϸ�
			Xi,			// ����
			Bei,		// ����
			Zhong,		// ����
			Fa,			// ����
			Bai,		// �װ�
			Chun,		// ��
			Xia,		// ��
			Qiu,		// ��
			Winter,		// ��
			Mei,		// ÷
			Lan,		// ��
			Ju,			// ��
			Zhu			// ��
		};

		// �Ƶ���
		enum class Number : int
		{
			Invalid = 0,	// ��Ч
			Yi,				// 1
			Er,				// 2
			San,			// 3
			Si,				// 4
			Wu,				// 5
			Liu,			// 6
			Qi,				// 7
			Ba,				// 8
			Jiu				// 9
		};

		class Tile
		{
		public:
			Tile();
			Tile(const Tile& t);
			Tile(Pattern p, Number n = Number::Invalid);

		public:
			Tile& operator=(const Tile& t);
			bool operator==(const Tile& t) const;
			bool operator!=(const Tile& t) const;
			bool operator>(const Tile& t) const;
			bool operator>=(const Tile& t) const;
			bool operator<(const Tile& t) const;
			bool operator<=(const Tile& t) const;

			// ����������
			Pattern getPattern() const;

			// ��������ֵ
			Number getNumber() const;

			// �ж����Ƿ�Ϊ��Ч��
			bool isValid() const;

			// ����Ϊ��Ч��
			void setInvalid();

			// �Ƿ�Ϊ����ֵ���ơ����ƺͻ��Ʋ�����ֵ
			bool isNumbered() const;

			// �ж��������Ƿ����ڣ���1���2��4Ͳ��5Ͳ������astrideΪtrueʱ�ж��Ƿ��һ�������ڣ���1���3��4Ͳ��6Ͳ
			bool isAdjacent(const Tile& t, bool astride) const;

			// ת��Ϊ�ַ���
			void toString(std::string& text) const;

			// ���ַ�����������
			static Tile fromString(const std::string& text);

		private:
			int pattern;		// ������
			int number;			// �Ƶ���
		
		public:
			MSGPACK_DEFINE_MAP(pattern, number);
		};

		typedef std::vector<Tile> TileArray;

		static const int INVALID_ID;
		static const char* PATTERN_NAMES[18];
		static const char* NUMBER_NAMES[9];

	public:
		MahjongTile();
		MahjongTile(const MahjongTile& mt);
		MahjongTile(int id, Pattern p, Number n);
		virtual ~MahjongTile();

	public:
		MahjongTile& operator=(const MahjongTile& mt);
		// ���ڰ������еıȽϺ����������ıȽ������Ƶ�ID
		bool operator==(const MahjongTile& mt) const;
		bool operator!=(const MahjongTile& mt) const;
		bool operator>(const MahjongTile& mt) const;
		bool operator>=(const MahjongTile& mt) const;
		bool operator<(const MahjongTile& mt) const;
		bool operator<=(const MahjongTile& mt) const;

	public:
		// �Ƿ�Ϊ��Ч��
		bool isValid() const;

		// ������Ч��
		void setInvalid();

		// ������
		const Tile& getTile() const;

		//
		void setTile(const Tile& t);

		// ����������
		Pattern getPattern() const;

		// ��������ֵ
		Number getNumber() const;

		// ������ID
		int getId() const;

		// ������ID
		void setId(int id_);

		// �ж��������ǲ���ͬ����ͬ��ֵ
		bool isSame(const MahjongTile& mt) const;

		// �ж��������ǲ���ͬ����ͬ��ֵ
		bool isSame(const MahjongTile::Tile& t) const;

		// �ж��������Ƿ����ڣ���1���2��4Ͳ��5Ͳ������astrideΪtrueʱ�ж��Ƿ��һ�������ڣ���1���3��4Ͳ��6Ͳ
		bool isAdjacent(const MahjongTile::Tile& t, bool astride) const;

	private:
		/**
		 * ��ID��ÿ���ƶ���ΨһID��������ͬ����ͬ����
		 */
		int id;

		/**
		 * ��
		 */
		Tile tile;

	public:
		MSGPACK_DEFINE_MAP(id, tile);
	};

	typedef std::vector<MahjongTile> MahjongTileArray;
}

#endif