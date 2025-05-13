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
	 * 麻将玩家替身
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
		bool getTile(MahjongTile& mt) const;	// 在手牌中，由ID查找牌
		bool findTile(MahjongTile& mt) const;	// 在手牌中，由牌查找ID(第一个)
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
		// 清理
		virtual void clear();

		// 摸牌
		virtual void fetchTile(const MahjongTile& mt);

		// 出牌
		virtual bool playTile(int id);

		// 当前所有能吃的牌
		virtual bool canChi(const MahjongTile& mt, std::vector<std::pair<int, int> >& lstPairs) const;

		// 当前是否能碰指定的牌
		// 参数passed返回之前过碰的牌
		virtual bool canPeng(const MahjongTile& mt, std::string& passed) const;

		// 当前是否能直杠指定的牌
		virtual bool canZhiGang(const MahjongTile& mt) const;

		// 当前所有能加杠的牌
		virtual bool canJiaGang(std::vector<int>& lstTileIds) const;

		// 当前所有能暗杠的牌
		virtual bool canAnGang(std::vector<int>& lstTileIds) const;

		// 当前是否能胡指定的牌
		virtual bool canHu(const MahjongTile& mt) const;

		// 当前是否可以放炮，例如有的地方规定暗杠之后不能放炮
		virtual bool canFangPao() const;

		// 当前是否能点炮指定的牌，例如有的地方规定玩家选择过胡之后，直到该玩家下次摸牌之后
		// 才能点炮，还有的地方规定暗杠之后不能点炮只能自摸
		// 参数passed返回之前过胡的牌
		virtual bool canDianPao(const MahjongTile& mt, std::string& passed) const;

		// 过胡(过点炮动作才调用此函数，过自摸不可调用此函数)
		virtual void passDianPao(const MahjongTile& mt);

		// 过碰
		virtual void passPeng(const MahjongTile& mt);

		// 通知自动操作完成
		virtual void afterAutoAction();

		// 吃
		virtual bool doChi(const MahjongTile& mt, int id1, int id2, int actionId, int player);

		// 碰
		virtual bool doPeng(const MahjongTile& mt, int actionId, int player);

		// 直杠
		virtual bool doZhiGang(const MahjongTile& mt, int actionId, int player);

		// 加杠
		virtual bool doJiaGang(const MahjongTile& mt, int actionId);

		// 暗杠
		virtual bool doAnGang(const MahjongTile& mt, int actionId);

		// 加杠被抢，把加杠章变回碰张
		virtual bool doQiangGang(int tileId, int actionId);

	public:
		// 否决最后的杠章(杠上炮之后在放炮玩家身上调用)
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
		// 自动出牌，可重载此函数实现更复杂的AI以打出当前最适合打出的牌
		virtual int autoPlayTile() const;

		// 检测胡牌样式(7小对、碰碰胡等)，不同规则可能会有自己扩展的样式，重载该函数实现自己扩展的判定
		virtual bool detectHuStyle(bool bZiMo, const MahjongTile& mt);

		// 由胡方式及胡样式计算分数
		virtual int calcHuScore() const = 0;

	protected:
		/**
		 * 手牌(发牌之后立刻排序，在之后游戏过程的任何时刻手牌都是排序好的)
		 */
		MahjongTileArray _handTiles;

		/**
		 * 发牌之后的初始手牌，用于保存录像
		 */
		MahjongTileArray _dealedTiles;

		/**
		 * 已打出的牌
		 */
		MahjongTileArray _playedTiles;

		/**
		 * 过胡的牌
		 */
		MahjongTileArray _passedHu;

		/**
		 * 过碰的牌
		 */
		MahjongTileArray _passedPeng;

		/**
		 * 所有的章
		 */
		MahjongChapterArray _chapters;

		/**
		 * 所有杠的牌
		 */
		MahjongTile::TileArray _gangTiles;

		/**
		 * 听牌
		 */
		MahjongGenre::TingPaiArray _tingTiles;

		/**
		 * 当前等待选择的动作选项
		 */
		std::vector<int> _actionOptions;

		/**
		 * 记录打出去的牌被其他玩家吃、碰、直杠
		 */
		std::map<int, int> _playedTileAction;

		/**
		 * 下一次要摸起的牌，在客户端指定(仅用于测试麻将相关算法的正确性)
		 */
		std::string _nextTile;

		/**
		 * 刚摸到的牌ID
		 */
		int _fetchedTileId;

		/**
		 * 胡牌样式
		 */
		int _huStyle;

		/**
		 * 扩展胡牌样式
		 */
		int _huStyleEx;

		/**
		 * 胡牌方式
		 */
		int _huWay;

		/**
		 * 累计自摸数
		 */
		int _ziMoes;

		/**
		 * 累计接炮数
		 */
		int _jiePaoes;

		/**
		 * 累计放炮数
		 */
		int _fangPaoes;

		/**
		 * 累计的胡、暗杠、加杠、直杠次数
		 */
		int _huGangs[4];

		/**
		 * 本局的暗杠、加杠、直杠次数
		 */
		int _gangs[3];

		/**
		 * 得分
		 */
		int _score;
	};
}

#endif