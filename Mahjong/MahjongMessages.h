// MahjongMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.12.02

#ifndef _NIU_MA_MAHJONG_MESSAGES_H_
#define _NIU_MA_MAHJONG_MESSAGES_H_

#include "MahjongAction.h"
#include "MahjongChapter.h"

#include "Game/GameMessages.h"

namespace NiuMa
{
	class MahjongMessages
	{
	private:
		MahjongMessages() {}

	public:
		virtual ~MahjongMessages() {}

		static void registMessages();
	};

	/**
	 * ������Ϣ
	 * ������->�ͻ���
	 */
	class MsgMahjongTiles : public MsgBase
	{
	public:
		MsgMahjongTiles();
		virtual ~MsgMahjongTiles();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// ����
		MahjongTileArray tiles;

		MSGPACK_DEFINE_MAP(tiles);
	};

	/**
	 * ֪ͨ��ǰ��Ҹ�����Ϣ
	 * ������->�ͻ���
	 */
	class MsgActorUpdated : public MsgBase
	{
	public:
		MsgActorUpdated();
		virtual ~MsgActorUpdated();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;

		MSGPACK_DEFINE_MAP(actor);
	};

	/**
	 * ֪ͨ�ȴ����ִ�ж���
	 * ������->�ͻ���
	 */
	class MsgWaitAction : public MsgBase {
	public:
		MsgWaitAction();
		virtual ~MsgWaitAction();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// �ȴ�(true)��ȡ���ȴ�(false)
		bool waitting;

		// ��֪ͨ������Ƿ����ڱ��ȴ�
		bool beingHeld;

		// �ѵȴ�ʱ��(��)
		int second;

		MSGPACK_DEFINE_MAP(waitting, beingHeld, second);
	};

	/**
	 * ֪ͨ������Ϣ
	 * ������->�ͻ���
	 */
	class MsgFetchTile : public MsgBase
	{
	public:
		MsgFetchTile();
		virtual ~MsgFetchTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// �����Ƶ����
		int player;
		// ʣ���������
		int nums;
		// �Ƿ�Ӻ�������(�ܺ���)
		bool back;
		//
		MahjongTile tile;

		MSGPACK_DEFINE_MAP(player, nums, back, tile);
	};

	/**
	 * ֪ͨ����ѡ����Ϣ
	 * ������->�ͻ���
	 */
	class MsgActionOption : public MsgBase
	{
	public:
		MsgActionOption();
		virtual ~MsgActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<MahjongActionOption> actionOptions;

		MSGPACK_DEFINE_MAP(actionOptions);
	};

	/**
	 * ִ�ж���ѡ��
	 * �ͻ���->������
	 */
	class MsgDoActionOption : public MsgVenueInner {
	public:
		MsgDoActionOption();
		virtual ~MsgDoActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// ����id
		int actionId;

		// ��id
		int tileId;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, actionId, tileId);
	};

	/**
	 * ִ�ж���ѡ��㡰�����������������ܡ�������
	 * �ͻ���->������
	 */
	class MsgPassActionOption : public MsgVenueInner {
	public:
		MsgPassActionOption();
		virtual ~MsgPassActionOption();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * ֪ͨ����ѡ���Ѿ����
	 * ������->�ͻ���
	 */
	class MsgActionOptionFinish : public MsgBase {
	public:
		MsgActionOptionFinish();
		virtual ~MsgActionOptionFinish();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// ռλ����ûɶ��
		int placeholder;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * ֪ͨ��ҳ�����Ϣ
	 * ������->�ͻ���
	 */
	class MsgPlayTile : public MsgBase
	{
	public:
		MsgPlayTile();
		virtual ~MsgPlayTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;							// ���Ƶ����
		MahjongTile tile;					// �������
		MahjongTileArray handTiles;			// ����֮�������

		MSGPACK_DEFINE_MAP(actor, tile, handTiles);
	};

	/**
	 * ֪ͨ��Ҹ�����Ϣ
	 * ������->�ͻ���
	 */
	class MsgGangTile : public MsgBase
	{
	public:
		MsgGangTile();
		virtual ~MsgGangTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int actor;						// �������
		int player;						// �����Ƶ����
		int tileNums;					// ʣ����������
		int chapter;					// ��������(ֱ�ܡ��Ӹܡ���)
		MahjongTileArray handTiles;		// �ܡ���������֮�������
		MahjongChapterArray chapters;	// ����

		MSGPACK_DEFINE_MAP(actor, player, tileNums, chapter, handTiles, chapters);
	};

	/**
	 * ֪ͨ�������������Ϣ
	 * ������->�ͻ���
	 */
	class MsgPengChiTile : public MsgBase
	{
	public:
		MsgPengChiTile();
		virtual ~MsgPengChiTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// �����������
		int actor;

		// ���������Ƶ����
		int player;

		// ��-true, ��-false
		bool pengOrChi;

		//
		MahjongTileArray tiles;

		// ��������֮�������
		MahjongTileArray handTiles;		

		MSGPACK_DEFINE_MAP(actor, player, pengOrChi, tiles, handTiles);
	};

	/**
	 * ֪ͨ���������Ϣ
	 * ������->�ͻ���
	 */
	class MsgTingTile : public MsgBase {
	public:
		MsgTingTile();
		virtual ~MsgTingTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// �����б�
		MahjongTile::TileArray tiles;

		MSGPACK_DEFINE_MAP(tiles);
	};

	/**
	 * ֪ͨ��Һ���Ϣ
	 * ������->�ͻ���
	 */
	class MsgHuTile : public MsgBase
	{
	public:
		MsgHuTile();
		virtual ~MsgHuTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		int players[3];					// �������λ��
		int actor;						// �������(���������)
		bool ziMo;						// �Ƿ�����
		MahjongTile tile;				// ��������

		MSGPACK_DEFINE_MAP(players, actor, ziMo, tile);
	};

	/**
	 * ��ʾ������ҵ�������Ϣ(������һ�ֽ���ʱ��������ҵ��Ƶ���)
	 * ������->�ͻ���
	 */
	class MsgShowTiles : public MsgBase
	{
	public:
		MsgShowTiles();
		virtual ~MsgShowTiles();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		MahjongTileArray handTiles[4];	// ������ҵ�����

		MSGPACK_DEFINE_MAP(handTiles);
	};

	/**
	 * ��ʾ������������������������
	 * ������->�ͻ���
	 */
	class MsgPassTip : public MsgBase {
	public:
		MsgPassTip();
		virtual ~MsgPassTip();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 0-������1-����
		int action;

		// �������������
		std::string tile;

		MSGPACK_DEFINE_MAP(action, tile);
	};

	/**
	 * ָ����һ���������(�����ڲ�����Ϸ����㷨����ȷ��)
	 * �ͻ���->������
	 */
	class MsgNextTile : public MsgVenueInner {
	public:
		MsgNextTile();
		virtual ~MsgNextTile();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// ��id
		int tileId;

		// ������
		std::string tileName;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, tileId, tileName);
	};
}

#endif