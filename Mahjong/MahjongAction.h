// MahjongAction.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_ACTION_H_
#define _NIU_MA_MAHJONG_ACTION_H_

#include <vector>

#include "msgpack/msgpack.hpp"

namespace NiuMa
{
	/**
	 * �齫����
	 * �齫��Ϸ���߼����̼�״̬�仯�ɶ���������
	 */
	class MahjongAction
	{
	public:
		// ��������
		enum class Type : int
		{
			Invalid = 0,	// �Ƿ�����
			Fetch,			// ��ʣ������ȡ��һ����
			Play,			// ���һ����
			Chi,			// ��
			Peng,			// ��
			ZhiGang,		// ֱ��
			JiaGang,		// �Ӹ�
			AnGang,			// ����
			DianPao,		// ����(�еĵط��ƽ��ڣ����Ǻ�������Ҵ�����Ƶ���˼)
			ZiMo			// ����
		};

	public:
		MahjongAction();
		MahjongAction(const MahjongAction& ma);
		MahjongAction(Type type_, int slot_, int tile_);
		virtual ~MahjongAction();

	protected:
		/**
		 * ��������
		 */
		int type;

		/**
		 * �������б��е�λ������
		 */
		int slot;

		/**
		 * �˶�������ID
		 */
		int tile;

	public:
		MahjongAction& operator=(const MahjongAction& ma);

	public:
		Type getType() const;
		void setType(Type t);
		int getSlot() const;
		void setSlot(int s);
		int getTile() const;
		void setTile(int id);

		MSGPACK_DEFINE_MAP(type, slot, tile);
	};

	// �����б�
	typedef std::vector<MahjongAction> MahjongActionList;

	/**
	 * ������
	 * һ�������߻������������������磺��򵥵�������һ�����������ų���Ҳ��һ��������
	 * ����������Ķ�������һ������������ɣ������ӵ������ֱ����һ������(1)�����Ÿܺ���
	 * ����һ������(2)������������������ܸ�(����)������Ҳ��һ������(3)�����Ÿܺ�������һ��
	 * ����(4)�������ƻ���һ������(5)���ܹ�5������Ķ�������һ�����������
	 */
	class MahjongActor
	{
	public:
		MahjongActor();
		MahjongActor(const MahjongActor& ma);
		MahjongActor(int p, int s);
		virtual ~MahjongActor();

	public:
		MahjongActor& operator=(const MahjongActor& ma);

	public:
		int getPlayer() const;
		int getStart() const;

	protected:
		/**
		 * ����������ϵ�λ������
		 */
		int player;

		/**
		 * �ö����ߵĵ�һ�������ڶ����б��е�����
		 */
		int start;

	public:
		MSGPACK_DEFINE_MAP(player, start);
	};

	// �������б�
	typedef std::vector<MahjongActor> MahjongActorList;


	/**
	 * ����ѡ��
	 * �����������һ���ƺ����������ܼӸܡ��ܰ��ܣ�ע��ӸܺͰ��ܲ�һ���Ǹܸ�����������
	 * (������ͷ���ܼӸܻ��߰��ܶ���ƣ�ÿ���ܶ���һ��ѡ��)���������ж��ֶ���ѡ����⣬
	 * ����Ҵ��һ���ƺ���������ܺ�(����ͬʱ�ж����Һ�)����ֱ�ܡ��������¼��ܳ�(���
	 * �������ֳԷ���ÿ�ֳԷ�����һ��ѡ��)������Ҳ�кܶ��ֶ���ѡ��
	 */
	class MahjongActionOption
	{
	public:
		MahjongActionOption();
		MahjongActionOption(const MahjongActionOption& ao);
		virtual ~MahjongActionOption();

	public:
		MahjongActionOption& operator=(const MahjongActionOption& ma);

	public:
		int getId() const;
		void setId(int id_);
		MahjongAction::Type getType() const;
		void setType(MahjongAction::Type t);
		int getPlayer() const;
		void setPlayer(int p);
		int getTileId1() const;
		void setTileId1(int id);
		int getTileId2() const;
		void setTileId2(int id);

	protected:
		/**
		 * ѡ��ID
		 */
		int id;

		/**
		 * ��������(MahjongAction::Type)
		 */
		int type;

		/**
		 * ���λ������
		 */
		int player;

		/**
		 * ��ID1
		 */
		int tile1;

		/**
		 * ��ID2(��ѡ�����������)
		 */
		int tile2;

	public:
		MSGPACK_DEFINE_MAP(id, type, player, tile1, tile2);
	};
}

#endif