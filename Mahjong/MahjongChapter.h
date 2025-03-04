// MahjongChapter.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_CHAPTER_H_
#define _NIU_MA_MAHJONG_CHAPTER_H_

#include "MahjongTile.h"

namespace NiuMa
{
	/**
	 * �齫�£���ҳԡ�������֮����������ϵ�3(4)���Ƴ�Ϊһ��
	 */
	class MahjongChapter
	{
	public:
		MahjongChapter();
		MahjongChapter(const MahjongChapter& mc);
		virtual ~MahjongChapter();

	public:
		enum class Type : int {
			Invalid,	// �Ƿ�
			Chi,		// ��
			Peng,		// ��
			ZhiGang,	// ֱ��
			JiaGang,	// �Ӹ�
			AnGang		// ����
		};

	public:
		MahjongChapter& operator=(const MahjongChapter& mc);

	public:
		// �����Ƿ񱻷��
		bool isVetoed() const;
		// �����·��
		void setVetoed();
		// ���յ���������
		Type getType() const;
		// ��ָ������ִ��֮�����µ�����
		Type getType(int actionId) const;
		//
		void getTypes(int types_[], int actionIds_[]) const;
		void addType(Type t, int actionId);
		bool hasActionId(int actionId) const;
		bool isGang() const;
		int getTargetPlayer() const;
		void setTargetPlayer(int p);
		int getTargetTile() const;
		void setTargetTile(int t);
		MahjongTileArray& getAllTiles();
		const MahjongTileArray& getAllTiles() const;
		void setAllTiles(const MahjongTileArray& tiles_);
		void hideAnGangTiles();

	private:
		/**
		 * �����Ƿ񱻷���������֮���ܼ����������������еĵط��涨�����ڲ�����ܷ֣�
		 * ��ʱ���Խ�����һ���ܷ��
		 */
		bool _vetoed;

		/**
		 * �������飬����������ʱ��ֻ�����ض���һ�����ͣ�����ΪʲôҪ�����������أ���Ϊ������״
		 * ̬��Ǩ:��->�Ӹ�->��(������֮��)��������Ҫ��¼״̬��Ǩ����ʱ��(��Ӧ�����б��е�����)��
		 * �Ա������Խ�����Ϸ����¼��ط�
		 */
		int types[3];

		/**
		 * ��������
		 */
		int actionIds[3];

		/**
		 * Ŀ�����λ�ã����A����Ʊ�������ô�ó�Ա��ֵ�������A��λ��������
		 * ����ǰ�����ó�Աû������
		 */
		int targetPlayer;

		/**
		 * Ŀ����ID������ǰ�����ó�Աû������
		 */
		int targetTile;

		/**
		 * �������ȫ����
		 */
		MahjongTileArray tiles;

	public:
		MSGPACK_DEFINE_MAP(types, actionIds, targetPlayer, targetTile, tiles);
	};

	typedef std::vector<MahjongChapter> MahjongChapterArray;
}

#endif // !_NIU_MA_MAHJONG_CHAPTER_H_
