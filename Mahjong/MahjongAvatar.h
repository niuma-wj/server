// MahjongAvatar.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.02

#ifndef _NIU_MA_MAHJONG_AVATAR_H_
#define _NIU_MA_MAHJONG_AVATAR_H_

#include "Game/GameAvatar.h"
#include "MahjongGenre.h"
#include "MahjongAction.h"
#include "MahjongChapter.h"

#include <map>

namespace NiuMa
{
	/**
	 * �齫�������
	 */
	class MahjongAvatar : public GameAvatar
	{
	public:
		MahjongAvatar(const std::string& playerId, int seat, bool bRobot);
		virtual ~MahjongAvatar();

	public:
		MahjongTileArray& getTiles();
		const MahjongTileArray& getTiles() const;
		void getTilesNoFetched(MahjongTileArray& tiles) const;
		unsigned int getTileNums() const;
		void backupDealedTiles();
		const MahjongTileArray& getDealedTiles() const;
		MahjongGenre::TingPaiArray& getTingTiles();
		const MahjongGenre::TingPaiArray& getTingTiles() const;
		bool getTile(MahjongTile& mt) const;	// �������У���ID������
		bool findTile(MahjongTile& mt) const;	// �������У����Ʋ���ID(��һ��)
		int getTileNums(const MahjongTile::Tile& t) const;
		void insertTile(const MahjongTile& mt);
		bool removeTile(int id);
		void sortTiles();
		bool getFetchedTile(MahjongTile& mt) const;
		int getFetchedTileId() const;
		void addPlayedTile(const MahjongTile& mt);
		unsigned int getPlayedTileNums() const;
		const MahjongTileArray& getPlayedTiles() const;
		void setPlayedTileAction(int id, MahjongAction::Type eType, int index);
		bool hasPlayedTilePeng(const MahjongTile::Tile& t) const;
		void getPlayedTilesNoAction(MahjongTileArray& lstTiles) const;
		const MahjongChapterArray& getChapters() const;
		void getChapterTiles(MahjongTileArray& chapterTiles) const;
		const MahjongTile::TileArray& getGangTiles() const;
		const std::vector<int>& getAllActionOptions() const;
		void addActionOption(int id);
		void removeActionOption(int id);
		bool hasActionOption() const;
		bool hasActionOption(int id) const;
		void clearActionOptions();

	public:
		// ����
		virtual void clear();

		// ����
		virtual void fetchTile(const MahjongTile& mt);

		// ����
		virtual bool playTile(int id);

		// ��ǰ�����ܳԵ���
		virtual bool canChi(const MahjongTile& mt, std::vector<std::pair<int, int> >& lstPairs) const;

		// ��ǰ�Ƿ�����ָ������
		// ����passed����֮ǰ��������
		virtual bool canPeng(const MahjongTile& mt, std::string& passed) const;

		// ��ǰ�Ƿ���ֱ��ָ������
		virtual bool canZhiGang(const MahjongTile& mt) const;

		// ��ǰ�����ܼӸܵ���
		virtual bool canJiaGang(std::vector<int>& lstTileIds) const;

		// ��ǰ�����ܰ��ܵ���
		virtual bool canAnGang(std::vector<int>& lstTileIds) const;

		// ��ǰ�Ƿ��ܺ�ָ������
		virtual bool canHu(const MahjongTile& mt) const;

		// ��ǰ�Ƿ���Է��ڣ������еĵط��涨����֮���ܷ���
		virtual bool canFangPao() const;

		// ��ǰ�Ƿ��ܵ���ָ�����ƣ������еĵط��涨���ѡ�����֮��ֱ��������´�����֮��
		// ���ܵ��ڣ����еĵط��涨����֮���ܵ���ֻ������
		// ����passed����֮ǰ��������
		virtual bool canDianPao(const MahjongTile& mt, std::string& passed) const;

		// ����(�����ڶ����ŵ��ô˺��������������ɵ��ô˺���)
		virtual void passDianPao(const MahjongTile& mt);

		// ����
		virtual void passPeng(const MahjongTile& mt);

		// ֪ͨ�Զ��������
		virtual void afterAutoAction();

		// ��
		virtual bool doChi(const MahjongTile& mt, int id1, int id2, int actionId, int player);

		// ��
		virtual bool doPeng(const MahjongTile& mt, int actionId, int player);

		// ֱ��
		virtual bool doZhiGang(const MahjongTile& mt, int actionId, int player);

		// �Ӹ�
		virtual bool doJiaGang(const MahjongTile& mt, int actionId);

		// ����
		virtual bool doAnGang(const MahjongTile& mt, int actionId);

		// �Ӹܱ������ѼӸ��±������
		virtual bool doQiangGang(int tileId, int actionId);

	public:
		// ������ĸ���(������֮���ڷ���������ϵ���)
		void vetoLastGangs(int nums);
		bool isHu() const;
		bool isZiMo() const;
		bool isDianPao() const;
		bool isMenQing() const;
		void addZiMo();
		void addJiePao();
		void addFangPao();
		void addHuTimes();
		void addHuWay(MahjongGenre::HuWay eWay);
		unsigned int getHuStyle() const;
		unsigned int getHuStyleEx() const;
		unsigned int getHuWay() const;
		void setNextTile(const std::string& str);
		const std::string& getNextTile() const;
		unsigned int getZiMo() const;
		unsigned int getJiePao() const;
		unsigned int getFangPao() const;
		void getTimes(int* arrTimes) const;
		void getGangs(int* arrGangs) const;
		void setScore(int s);
		int getScore() const;

	public:
		// �Զ����ƣ������ش˺���ʵ�ָ����ӵ�AI�Դ����ǰ���ʺϴ������
		virtual int autoPlayTile() const;

		// ��������ʽ(7С�ԡ���������)����ͬ������ܻ����Լ���չ����ʽ�����ظú���ʵ���Լ���չ���ж�
		virtual bool detectHuStyle(bool bZiMo, const MahjongTile& mt);

		// �ɺ���ʽ������ʽ�������
		virtual int calcHuScore() const = 0;

	protected:
		/**
		 * ����(����֮������������֮����Ϸ���̵��κ�ʱ�����ƶ�������õ�)
		 */
		MahjongTileArray _handTiles;

		/**
		 * ����֮��ĳ�ʼ���ƣ����ڱ���¼��
		 */
		MahjongTileArray _dealedTiles;

		/**
		 * �Ѵ������
		 */
		MahjongTileArray _playedTiles;

		/**
		 * ��������
		 */
		MahjongTileArray _passedHu;

		/**
		 * ��������
		 */
		MahjongTileArray _passedPeng;

		/**
		 * ���е���
		 */
		MahjongChapterArray _chapters;

		/**
		 * ���иܵ���
		 */
		MahjongTile::TileArray _gangTiles;

		/**
		 * ����
		 */
		MahjongGenre::TingPaiArray _tingTiles;

		/**
		 * ��ǰ�ȴ�ѡ��Ķ���ѡ��
		 */
		std::vector<int> _actionOptions;

		/**
		 * ��¼���ȥ���Ʊ�������ҳԡ�����ֱ��
		 */
		std::map<int, int> _playedTileAction;

		/**
		 * ��һ��Ҫ������ƣ��ڿͻ���ָ��(�����ڲ����齫����㷨����ȷ��)
		 */
		std::string _nextTile;

		/**
		 * ����������ID
		 */
		int _fetchedTileId;

		/**
		 * ������ʽ
		 */
		int _huStyle;

		/**
		 * ��չ������ʽ
		 */
		int _huStyleEx;

		/**
		 * ���Ʒ�ʽ
		 */
		int _huWay;

		/**
		 * �ۼ�������
		 */
		int _ziMoes;

		/**
		 * �ۼƽ�����
		 */
		int _jiePaoes;

		/**
		 * �ۼƷ�����
		 */
		int _fangPaoes;

		/**
		 * �ۼƵĺ������ܡ��Ӹܡ�ֱ�ܴ���
		 */
		int _huGangs[4];

		/**
		 * ���ֵİ��ܡ��Ӹܡ�ֱ�ܴ���
		 */
		int _gangs[3];

		/**
		 * �÷�
		 */
		int _score;
	};
}

#endif