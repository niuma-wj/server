// MahjongAvatar.cpp 

#include "Base/Log.h"
#include "MahjongAvatar.h"
#include "MahjongRule.h"
#include "MahjongDealer.h"

#include <sstream>
#include <algorithm>

namespace NiuMa
{
	MahjongAvatar::MahjongAvatar(const std::string& playerId, int seat, bool bRobot)
		: GameAvatar(playerId, bRobot)
		, _fetchedTileId(MahjongTile::INVALID_ID)
		, _huStyle(0)
		, _huStyleEx(0)
		, _huWay(0)
		, _ziMoes(0)
		, _jiePaoes(0)
		, _fangPaoes(0)
		, _huGangs{ 0, 0, 0, 0 }
		, _gangs{ 0, 0, 0 }
		, _score(0)
	{
		setSeat(seat);
		_handTiles.reserve(14);
		_dealedTiles.reserve(13);
	}

	MahjongAvatar::~MahjongAvatar()
	{}

	void MahjongAvatar::clear() {
		_handTiles.clear();
		_dealedTiles.clear();
		_playedTiles.clear();
		_passedHu.clear();
		_passedPeng.clear();
		_chapters.clear();
		_gangTiles.clear();
		_tingTiles.clear();
		_actionOptions.clear();
		_playedTileAction.clear();
		_fetchedTileId = MahjongTile::INVALID_ID;
		_huStyle = 0;
		_huStyleEx = 0;
		_huWay = 0;
		_gangs[0] = 0;
		_gangs[1] = 0;
		_gangs[2] = 0;
		_score = 0;
	}

	MahjongTileArray& MahjongAvatar::getTiles() {
		return _handTiles;
	}

	const MahjongTileArray& MahjongAvatar::getTiles() const {
		return _handTiles;
	}

	void MahjongAvatar::getTilesNoFetched(MahjongTileArray& tiles) const {
		tiles.clear();
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->getId() != _fetchedTileId)
				tiles.push_back(*it);
			++it;
		}
	}

	unsigned int MahjongAvatar::getTileNums() const {
		return static_cast<unsigned int>(_handTiles.size());
	}

	void MahjongAvatar::backupDealedTiles() {
		_dealedTiles.clear();
		_dealedTiles = _handTiles;
	}

	const MahjongTileArray& MahjongAvatar::getDealedTiles() const {
		return _dealedTiles;
	}

	MahjongGenre::TingPaiArray& MahjongAvatar::getTingTiles() {
		return _tingTiles;
	}

	const MahjongGenre::TingPaiArray& MahjongAvatar::getTingTiles() const {
		return _tingTiles;
	}

	bool MahjongAvatar::getTile(MahjongTile& mt) const {
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->getId() == mt.getId()) {
				mt.setTile(it->getTile());
				return true;
			}
			else if (it->getId() > mt.getId())
				return false;

			++it;
		}
		return false;
	}

	bool MahjongAvatar::findTile(MahjongTile& mt) const {
		bool bTest = false;
		bool bFound = false;
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->getPattern() != mt.getPattern()) {
				if (bTest)
					return false;
			} else {
				bTest = true;
				if (it->getNumber() == mt.getNumber()) {
					bFound = true;
					mt.setId(it->getId());
					break;
				}
			}
			++it;
		}
		return bFound;
	}

	int MahjongAvatar::getTileNums(const MahjongTile::Tile& t) const {
		int nums = 0;
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->getTile() == t)
				++nums;
			else if (nums > 0)
				break;

			++it;
		}
		return nums;
	}

	void MahjongAvatar::insertTile(const MahjongTile& mt) {
		bool bInserted = false;
		MahjongTileArray::iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (*it > mt) {
				_handTiles.insert(it, mt);
				bInserted = true;
				break;
			}
			++it;
		}
		if (!bInserted)
			_handTiles.push_back(mt);
	}

	bool MahjongAvatar::removeTile(int id) {
		// 删除指定的牌
		bool bFound = false;
		MahjongTileArray::iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->getId() == id) {
				bFound = true;
				_handTiles.erase(it);
				break;
			}
			else if (it->getId() > id)
				return false;

			++it;
		}
		return bFound;
	}

	void MahjongAvatar::sortTiles() {
		std::sort(_handTiles.begin(), _handTiles.end());
	}

	bool MahjongAvatar::getFetchedTile(MahjongTile& mt) const {
		mt.setId(_fetchedTileId);
		return MahjongDealer::getTileById(mt);
	}

	int MahjongAvatar::getFetchedTileId() const {
		return _fetchedTileId;
	}

	void MahjongAvatar::addPlayedTile(const MahjongTile& mt) {
		_playedTiles.push_back(mt);
	}

	unsigned int MahjongAvatar::getPlayedTileNums() const {
		return static_cast<unsigned int>(_playedTiles.size());
	}

	const MahjongTileArray& MahjongAvatar::getPlayedTiles() const {
		return _playedTiles;
	}

	void MahjongAvatar::setPlayedTileAction(int id, MahjongAction::Type eType, int index) {
		int val = index;
		int tmp = static_cast<int>(eType);
		val <<= 8;
		val |= tmp;

		_playedTileAction.insert(std::make_pair(id, val));
	}

	bool MahjongAvatar::hasPlayedTilePeng(const MahjongTile::Tile& t) const {
		// 判断是否打出过指定的牌并且被别的玩家碰了
		MahjongTile mt;
		int action = 0;
		std::map<int, int>::const_iterator it = _playedTileAction.begin();
		while (it != _playedTileAction.end()) {
			mt.setId(it->first);
			MahjongDealer::getTileById(mt);
			if (!mt.isSame(t)) {
				++it;
				continue;
			}
			action = it->second;
			action &= 0x000000ff;
			if (action == static_cast<int>(MahjongAction::Type::Peng))
				return true;

			++it;
		}
		return false;
	}

	void MahjongAvatar::getPlayedTilesNoAction(MahjongTileArray& lstTiles) const {
		MahjongTileArray::const_iterator it = _playedTiles.begin();
		std::map<int, int>::const_iterator it_;
		while (it != _playedTiles.end()) {
			it_ = _playedTileAction.find(it->getId());
			if (it_ == _playedTileAction.end())
				lstTiles.push_back(*it);

			++it;
		}
	}

	const MahjongChapterArray& MahjongAvatar::getChapters() const {
		return _chapters;
	}

	void MahjongAvatar::getChapterTiles(MahjongTileArray& chapterTiles) const {
		MahjongChapterArray::const_iterator it = _chapters.begin();
		while (it != _chapters.end()) {
			const MahjongTileArray& lstTiles = it->getAllTiles();
			chapterTiles.insert(chapterTiles.end(), lstTiles.begin(), lstTiles.end());

			++it;
		}
	}

	const MahjongTile::TileArray& MahjongAvatar::getGangTiles() const {
		return _gangTiles;
	}

	const std::vector<int>& MahjongAvatar::getAllActionOptions() const {
		return _actionOptions;
	}

	void MahjongAvatar::addActionOption(int id) {
		_actionOptions.push_back(id);
	}

	void MahjongAvatar::removeActionOption(int id) {
		std::vector<int>::const_iterator it = _actionOptions.begin();
		while (it != _actionOptions.end()) {
			if (*it == id) {
				_actionOptions.erase(it);
				break;
			}
			++it;
		}
	}

	bool MahjongAvatar::hasActionOption() const {
		return !_actionOptions.empty();
	}

	bool MahjongAvatar::hasActionOption(int id) const {
		std::vector<int>::const_iterator it = _actionOptions.begin();
		while (it != _actionOptions.end()) {
			if (*it == id)
				return true;

			++it;
		}
		return false;
	}

	void MahjongAvatar::clearActionOptions() {
		_actionOptions.clear();
	}

	void MahjongAvatar::fetchTile(const MahjongTile& mt) {
		insertTile(mt);

		_fetchedTileId = mt.getId();

		_passedHu.clear();
		_passedPeng.clear();
	}

	bool MahjongAvatar::playTile(int id) {
		return removeTile(id);
	}

	bool MahjongAvatar::canChi(const MahjongTile& mt, std::vector<std::pair<int, int> >& lstPairs) const {
		lstPairs.clear();
		if (mt.getPattern() > MahjongTile::Pattern::Wan)
			return false;

		MahjongTile tmp1;
		MahjongTile tmp2;
		int num = static_cast<int>(mt.getNumber());
		if (num > static_cast<int>(MahjongTile::Number::Er)) {
			tmp1.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num - 2)));
			tmp2.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num - 1)));
			if (findTile(tmp1) && findTile(tmp2))
				lstPairs.push_back(std::make_pair(tmp1.getId(), tmp2.getId()));
		}
		if (num < static_cast<int>(MahjongTile::Number::Jiu) && num > static_cast<int>(MahjongTile::Number::Yi)) {
			tmp1.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num - 1)));
			tmp2.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num + 1)));
			if (findTile(tmp1) && findTile(tmp2))
				lstPairs.push_back(std::make_pair(tmp1.getId(), tmp2.getId()));
		}
		if (num < static_cast<int>(MahjongTile::Number::Ba)) {
			tmp1.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num + 1)));
			tmp2.setTile(MahjongTile::Tile(mt.getPattern(), static_cast<MahjongTile::Number>(num + 2)));
			if (findTile(tmp1) && findTile(tmp2))
				lstPairs.push_back(std::make_pair(tmp1.getId(), tmp2.getId()));
		}
		return !lstPairs.empty();
	}

	bool MahjongAvatar::canPeng(const MahjongTile& mt, std::string& passed) const {
		passed.clear();
		MahjongTileArray::const_iterator it = _passedPeng.begin();
		while (it != _passedPeng.end()) {
			if (mt.isSame(*it)) {
				mt.getTile().toString(passed);
				std::stringstream ss;
				ss << "玩家(Id: " << getPlayerId() << ")此前过碰" << passed << "，在下次摸牌前不能再碰该牌。";
				LOG_INFO(ss.str());
				return false;
			}
			++it;
		}
		return (getTileNums(mt.getTile()) > 1);
	}

	bool MahjongAvatar::canZhiGang(const MahjongTile& mt) const {
		return (getTileNums(mt.getTile()) == 3);
	}

	bool MahjongAvatar::canJiaGang(std::vector<int>& lstTileIds) const {
		lstTileIds.clear();

		MahjongTile mt;
		MahjongChapterArray::const_iterator it = _chapters.begin();
		while (it != _chapters.end()) {
			if (it->getType() != MahjongChapter::Type::Peng) {
				++it;
				continue;
			}
			const MahjongTileArray& lstTiles = it->getAllTiles();
			if (!lstTiles.empty()) {
				mt.setTile(lstTiles[0].getTile());
				if (findTile(mt))
					lstTileIds.push_back(mt.getId());
			}
			++it;
		}
		return !lstTileIds.empty();
	}

	bool MahjongAvatar::canAnGang(std::vector<int>& lstTileIds) const {
		lstTileIds.clear();
		if (_handTiles.size() < 4)
			return false;

		int nums = 1;
		MahjongTileArray::const_iterator it1 = _handTiles.begin();
		MahjongTileArray::const_iterator it2 = it1;
		it2++;
		while (it2 != _handTiles.end()) {
			if (it1->getTile() == it2->getTile())
				nums++;
			else {
				if (nums > 3)
					lstTileIds.push_back(it1->getId());
				nums = 1;
				it1 = it2;
			}
			++it2;
		}
		if (nums > 3)
			lstTileIds.push_back(it1->getId());

		return !lstTileIds.empty();
	}

	bool MahjongAvatar::canHu(const MahjongTile& mt) const {
		MahjongGenre::TingPaiArray::const_iterator it = _tingTiles.begin();
		while (it != _tingTiles.end()) {
			if (mt.isSame(it->tile))
				return true;
			++it;
		}
		return false;
	}

	bool MahjongAvatar::canFangPao() const {
		return true;
	}

	bool MahjongAvatar::canDianPao(const MahjongTile& mt, std::string& passed) const {
		passed.clear();
		if (!_passedHu.empty()) {
			_passedHu[0].getTile().toString(passed);
			std::stringstream ss;
			ss << "玩家(Id: " << getPlayerId() << "此前选择过胡" << passed << "，在下次摸牌之前不能再点炮该牌。";
			LOG_INFO(ss.str());
			return false;
		}
		return true;
	}

	void MahjongAvatar::passDianPao(const MahjongTile& mt) {
		_passedHu.push_back(mt);
	}

	void MahjongAvatar::passPeng(const MahjongTile& mt) {
		_passedPeng.push_back(mt);
	}

	void MahjongAvatar::afterAutoAction() {}

	bool MahjongAvatar::doChi(const MahjongTile& mt, int id1, int id2, int actionId, int player) {
		MahjongTile mt1;
		MahjongTile mt2;
		mt1.setId(id1);
		mt2.setId(id2);
		if (!getTile(mt1) || !getTile(mt2))
			return false;

		MahjongTileArray lstTemp;
		lstTemp.push_back(mt);
		lstTemp.push_back(mt1);
		lstTemp.push_back(mt2);
		std::sort(lstTemp.begin(), lstTemp.end());
		int tmp = static_cast<int>(lstTemp[1].getNumber());
		tmp -= static_cast<int>(lstTemp[0].getNumber());
		if (tmp != 1)
			return false;
		tmp = static_cast<int>(lstTemp[2].getNumber());
		tmp -= static_cast<int>(lstTemp[1].getNumber());
		if (tmp != 1)
			return false;

		removeTile(id1);
		removeTile(id2);

		MahjongChapter mc;
		mc.addType(MahjongChapter::Type::Chi, actionId);
		mc.setTargetTile(mt.getId());
		mc.setTargetPlayer(player);
		mc.setAllTiles(lstTemp);
		_chapters.push_back(mc);

		return true;
	}

	bool MahjongAvatar::doPeng(const MahjongTile& mt, int actionId, int player) {
		if (getTileNums(mt.getTile()) < 2)
			return false;

		MahjongTileArray lstTemp;
		lstTemp.reserve(3);
		int nums = 0;
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->isSame(mt)) {
				lstTemp.push_back(*it);
				it = _handTiles.erase(it);
				nums++;
				if (nums > 1)
					break;
			}
			else
				++it;
		}
		lstTemp.push_back(mt);
		std::sort(lstTemp.begin(), lstTemp.end());

		MahjongChapter mc;
		mc.addType(MahjongChapter::Type::Peng, actionId);
		mc.setTargetTile(mt.getId());
		mc.setTargetPlayer(player);
		mc.setAllTiles(lstTemp);
		_chapters.push_back(mc);

		return true;
	}

	bool MahjongAvatar::doZhiGang(const MahjongTile& mt, int actionId, int player) {
		if (getTileNums(mt.getTile()) != 3)
			return false;

		MahjongTileArray lstTemp;
		lstTemp.reserve(4);
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->isSame(mt)) {
				lstTemp.push_back(*it);
				it = _handTiles.erase(it);
			}
			else
				++it;
		}
		lstTemp.push_back(mt);
		std::sort(lstTemp.begin(), lstTemp.end());

		MahjongChapter mc;
		mc.addType(MahjongChapter::Type::ZhiGang, actionId);
		mc.setTargetTile(mt.getId());
		mc.setTargetPlayer(player);
		mc.setAllTiles(lstTemp);
		_chapters.push_back(mc);

		_gangTiles.push_back(mt.getTile());

		_huGangs[3]++;
		_gangs[2]++;

		return true;
	}

	bool MahjongAvatar::doJiaGang(const MahjongTile& mt, int actionId) {
		bool bFound = false;
		unsigned int uiPos = 0;
		MahjongChapterArray::iterator it = _chapters.begin();
		while (it != _chapters.end()) {
			if (it->getType() != MahjongChapter::Type::Peng) {
				++uiPos;
				++it;
				continue;
			}
			const MahjongTileArray& lstTiles = it->getAllTiles();
			if (!lstTiles.empty() && lstTiles[0].isSame(mt)) {
				bFound = true;
				break;
			}
			++uiPos;
			++it;
		}
		if (!bFound)
			return false;
		if (!removeTile(mt.getId()))
			return false;

		it->addType(MahjongChapter::Type::JiaGang, actionId);
		MahjongTileArray& lstTiles = it->getAllTiles();
		lstTiles.push_back(mt);
		if (uiPos != (_chapters.size() - 1)) {
			// 把加杠的章挪到列表最后面，作为最新的章
			MahjongChapter mc = *it;
			_chapters.erase(it);
			_chapters.push_back(mc);
		}
		_gangTiles.push_back(mt.getTile());

		_huGangs[2]++;
		_gangs[1]++;

		return true;
	}

	bool MahjongAvatar::doAnGang(const MahjongTile& mt, int actionId) {
		if (getTileNums(mt.getTile()) != 4)
			return false;

		MahjongTileArray lstTemp;
		lstTemp.reserve(4);
		MahjongTileArray::const_iterator it = _handTiles.begin();
		while (it != _handTiles.end()) {
			if (it->isSame(mt)) {
				lstTemp.push_back(*it);
				it = _handTiles.erase(it);
			}
			else
				++it;
		}
		MahjongChapter mc;
		mc.addType(MahjongChapter::Type::AnGang, actionId);
		mc.setAllTiles(lstTemp);
		_chapters.push_back(mc);

		_gangTiles.push_back(mt.getTile());

		_huGangs[1]++;
		_gangs[0]++;

		return true;
	}

	bool MahjongAvatar::doQiangGang(int tileId, int actionId) {
		MahjongTile mt;
		mt.setId(tileId);
		if (!MahjongDealer::getTileById(mt))
			return false;

		bool bFound = false;
		MahjongChapterArray::iterator it = _chapters.begin();
		while (it != _chapters.end()) {
			MahjongChapter& mc = *it;
			if (mc.getType() != MahjongChapter::Type::JiaGang) {
				++it;
				continue;
			}
			MahjongTileArray& lstTiles = mc.getAllTiles();
			if (!lstTiles.empty() && lstTiles[0].isSame(mt)) {
				bFound = true;
				mc.addType(MahjongChapter::Type::Peng, actionId);
				_gangTiles.resize(_gangTiles.size() - 1);
				_huGangs[2]--;
				break;
			}
			++it;
		}
		return bFound;
	}

	void MahjongAvatar::vetoLastGangs(int nums) {
		if (nums < 1)
			return;
		int n = 0;
		MahjongChapter::Type eType = MahjongChapter::Type::Invalid;
		MahjongChapterArray::reverse_iterator it = _chapters.rbegin();
		while (it != _chapters.rend()) {
			MahjongChapter& mc = *it;
			eType = mc.getType();
			if (eType == MahjongChapter::Type::ZhiGang) {
				_huGangs[3]--;
				_gangs[2]--;
			}
			else if (eType == MahjongChapter::Type::JiaGang) {
				_huGangs[2]--;
				_gangs[1]--;
			}
			else if (eType != MahjongChapter::Type::AnGang) {
				_huGangs[1]--;
				_gangs[0]--;
			}
			else
				break;	// 正确情况下不会执行到这里，注意调试！！！
			mc.setVetoed();
			n++;
			if (n == nums)
				break;

			++it;
		}
	}

	bool MahjongAvatar::isHu() const {
		if (isZiMo() || isDianPao())
			return true;

		return false;
	}

	bool MahjongAvatar::isZiMo() const {
		int ziMo = static_cast<int>(MahjongGenre::HuWay::ZiMo);
		if ((_huWay & ziMo) == ziMo)
			return true;

		return false;
	}

	bool MahjongAvatar::isDianPao() const {
		int dianPao = static_cast<int>(MahjongGenre::HuWay::DianPao);
		if ((_huWay & dianPao) == dianPao)
			return true;

		return false;
	}

	bool MahjongAvatar::isMenQing() const {
		MahjongChapter::Type eType = MahjongChapter::Type::Invalid;
		MahjongChapterArray::const_iterator it = _chapters.begin();
		while (it != _chapters.end()) {
			eType = it->getType();
			if (eType == MahjongChapter::Type::Chi ||
				eType == MahjongChapter::Type::Peng ||
				eType == MahjongChapter::Type::ZhiGang ||
				eType == MahjongChapter::Type::JiaGang)
				return false;

			++it;
		}
		return true;
	}

	void MahjongAvatar::addZiMo() {
		_ziMoes++;
	}

	void MahjongAvatar::addJiePao() {
		_jiePaoes++;
	}

	void MahjongAvatar::addFangPao() {
		_fangPaoes++;
	}

	void MahjongAvatar::addHuTimes() {
		_huGangs[0]++;
	}

	void MahjongAvatar::addHuWay(MahjongGenre::HuWay eWay) {
		_huWay |= static_cast<int>(eWay);
	}

	unsigned int MahjongAvatar::getHuStyle() const {
		return _huStyle;
	}

	unsigned int MahjongAvatar::getHuStyleEx() const {
		return _huStyleEx;
	}

	unsigned int MahjongAvatar::getHuWay() const {
		return _huWay;
	}

	void MahjongAvatar::setNextTile(const std::string& str) {
		_nextTile = str;
	}

	const std::string& MahjongAvatar::getNextTile() const {
		return _nextTile;
	}

	unsigned int MahjongAvatar::getZiMo() const {
		return _ziMoes;
	}

	unsigned int MahjongAvatar::getJiePao() const {
		return _jiePaoes;
	}

	unsigned int MahjongAvatar::getFangPao() const {
		return _fangPaoes;
	}

	void MahjongAvatar::getTimes(int* arrTimes) const {
		if (arrTimes == nullptr)
			return;
		for (int i = 0; i < 4; i++)
			arrTimes[i] = _huGangs[i];
	}

	void MahjongAvatar::getGangs(int* arrGangs) const {
		if (arrGangs == nullptr)
			return;
		for (int i = 0; i < 3; i++)
			arrGangs[i] = _gangs[i];
	}

	void MahjongAvatar::setScore(int s) {
		_score = s;
	}

	int MahjongAvatar::getScore() const {
		return _score;
	}

	int MahjongAvatar::autoPlayTile() const {
		if (_handTiles.empty())
			return 0;

		bool bTest = true;
		MahjongTile::Tile t;
		int nCount = 0;
		MahjongTileArray arrTiles;
		std::vector<int> arrCount;
		arrTiles.reserve(_handTiles.size());
		arrCount.reserve(_handTiles.size());
		MahjongTileArray::const_reverse_iterator it = _handTiles.rbegin();
		while (it != _handTiles.rend()) {
			const MahjongTile& mt = *it;
			if (!arrTiles.empty()) {
				if (mt.isSame(t)) {
					bTest = false;
					nCount++;
				}
				else if (nCount == 1) {
					// 若遇到单个字牌，直接返回
					if (t.getPattern() > MahjongTile::Pattern::Wan) {
						const MahjongTile& mt1 = arrTiles.back();
						return mt1.getId();
					}
				}
			}
			if (bTest) {
				// 与上一个牌不相同
				if (!arrTiles.empty())
					arrCount.push_back(nCount);
				arrTiles.push_back(mt);
				t = mt.getTile();
				nCount = 1;
			}
			else
				bTest = true;
			++it;
		}
		arrCount.push_back(nCount);
		if (arrTiles.size() != arrCount.size()) {
			// 在正确情况下不会执行到这里，注意调试!!!
			const MahjongTile& mt = _handTiles.back();
			return mt.getId();
		}
		std::reverse(arrTiles.begin(), arrTiles.end());
		std::reverse(arrCount.begin(), arrCount.end());
		// 牌的聚集程度(gather degree)
		// 不聚集
		const int GD_NULL = 0x00;
		// 相邻小2张，例如对于牌7万，若手牌中有牌5万，则牌7万可加上此标识
		const int GD_LEFT2 = 0x01;
		// 相邻大2张，例如对于牌7万，若手牌中有牌9万，则牌7万可加上此标识
		const int GD_RIGHT2 = 0x02;
		// 相邻小1张
		const int GD_LEFT1 = 0x04;
		// 相邻大1张
		const int GD_RIGHT1 = 0x08;
		// 成对
		const int GD_PAIR = 0x10;
		// 三张
		const int GD_TRIPLE = 0x20;
		// 四张
		const int GD_QUADRUPLE = 0x40;
		//
		const unsigned int COUNT = static_cast<unsigned int>(arrTiles.size());
		int* gathers = new int[COUNT];
		int* weights = new int[COUNT];
		unsigned int i = 0;
		for (; i < COUNT; i++) {
			gathers[i] = 0;
			weights[i] = 0;
		}
		i = COUNT;
		do {
			i--;
			const MahjongTile& mt = arrTiles[i];
			if (arrCount[i] == 2)
				gathers[i] |= GD_PAIR;
			else if (arrCount[i] == 3)
				gathers[i] |= GD_TRIPLE;
			else if (arrCount[i] == 4)
				gathers[i] |= GD_QUADRUPLE;
			if (mt.getPattern() > MahjongTile::Pattern::Wan)
				continue;
			bTest = false;
			if (i > 0) {
				const MahjongTile& mt1 = arrTiles[i - 1];
				if (mt.isAdjacent(mt1.getTile(), false)) {
					gathers[i] |= GD_LEFT1;
					gathers[i - 1] |= GD_RIGHT1;
					bTest = true;
				}
				else if (mt.isAdjacent(mt1.getTile(), true)) {
					gathers[i] |= GD_LEFT2;
					gathers[i - 1] |= GD_RIGHT2;
				}
			}
			if (bTest && (i > 1)) {
				const MahjongTile& mt2 = arrTiles[i - 2];
				if (mt.isAdjacent(mt2.getTile(), true)) {
					gathers[i] |= GD_LEFT2;
					gathers[i - 2] |= GD_RIGHT2;
				}
			}
		} while (i > 0);
		const int GD_LEFT = GD_RIGHT1 | GD_RIGHT2;
		const int GD_RIGHT = GD_LEFT1 | GD_LEFT2;
		const int GD_MIDDLE = GD_LEFT1 | GD_RIGHT1;
		/** 权值表：
			GD_QUADRUPLE	- 100
			GD_TRIPLE		- 90
			GD_MIDDLE		- 80
			GD_LEFT			- 70
			GD_RIGHT		- 70
			GD_PAIR			- 60或80
			GD_LEFT1		- 50
			GD_RIGHT1		- 50
			GD_LEFT2		- 40
			GD_RIGHT2		- 40
		*/
		int pairs = 0;
		for (i = 0; i < COUNT; i++) {
			if ((gathers[i] & GD_PAIR) == GD_PAIR)
				pairs++;
		}
		for (i = 0; i < COUNT; i++) {
			if ((gathers[i] & GD_QUADRUPLE) == GD_QUADRUPLE)
				weights[i] = 100;
			else if ((gathers[i] & GD_TRIPLE) == GD_TRIPLE)
				weights[i] = 90;
			else if ((gathers[i] & GD_MIDDLE) == GD_MIDDLE)
				weights[i] = 80;
			else if (((gathers[i] & GD_LEFT) == GD_LEFT) || ((gathers[i] & GD_RIGHT) == GD_RIGHT))
				weights[i] = 70;
			else if ((gathers[i] & GD_PAIR) == GD_PAIR) {
				if (pairs > 1)
					weights[i] = 60;
				else
					weights[i] = 80;
			}
			else if (((gathers[i] & GD_LEFT1) == GD_LEFT1) || ((gathers[i] & GD_RIGHT1) == GD_RIGHT1))
				weights[i] = 50;
			else if (((gathers[i] & GD_LEFT2) == GD_LEFT2) || ((gathers[i] & GD_RIGHT2) == GD_RIGHT2))
				weights[i] = 40;
			const MahjongTile& mt = arrTiles[i];
			if (mt.getPattern() < MahjongTile::Pattern::Dong) {
				int n = static_cast<int>(mt.getNumber());
				weights[i] += (5 - abs(n - 5));
			}
		}
		int nMin = 1000;
		unsigned int index = 0;
		for (i = 0; i < COUNT; i++) {
			if (nMin > weights[i]) {
				nMin = weights[i];
				index = i;
			}
		}
		delete[] gathers;
		delete[] weights;
		const MahjongTile& mt = arrTiles[index];
		return mt.getId();
	}

	bool MahjongAvatar::detectHuStyle(bool bZiMo, const MahjongTile& mt) {
		bool bFound = false;
		bool bQiXiaoDui = false;
		_huStyle = 0;
		MahjongGenre::TingPaiArray::const_iterator it = _tingTiles.begin();
		while (it != _tingTiles.end()) {
			if (mt.isSame(it->tile)) {
				_huStyle = it->style;
				bFound = true;
				break;
			}
			++it;
		}
		if (!bFound)
			return false;

		MahjongTileArray lstTemp;
		lstTemp.reserve(_handTiles.size() + _chapters.size() * 4);
		lstTemp = _handTiles;
		if (!bZiMo)
			lstTemp.push_back(mt);
		MahjongRule rule;
		if (_chapters.empty()) {
			// 门清并且没有暗杠才能是七小对
			std::sort(lstTemp.begin(), lstTemp.end());
			if (rule.checkQiXiaoDui(lstTemp)) {
				bQiXiaoDui = true;
				_huStyle |= static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui);
				int ret = rule.checkHaoHuaQiXiaoDui(lstTemp);
				if (ret == 1)
					_huStyle |= static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui1);
				else if (ret == 2)
					_huStyle |= static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui2);
				else if (ret == 3)
					_huStyle |= static_cast<int>(MahjongGenre::HuStyle::QiXiaoDui3);
			}
		} else {
			MahjongChapterArray::const_iterator it1 = _chapters.begin();
			while (it1 != _chapters.end()) {
				const MahjongTileArray& lstTiles = it1->getAllTiles();
				lstTemp.insert(lstTemp.end(), lstTiles.begin(), lstTiles.end());
				++it1;
			}
			std::sort(lstTemp.begin(), lstTemp.end());
		}
		if (rule.checkQingYiSe(lstTemp))
			_huStyle |= static_cast<int>(MahjongGenre::HuStyle::QingYiSe);
		else if (rule.checkZiYiSe(lstTemp))
			_huStyle |= static_cast<int>(MahjongGenre::HuStyle::ZiYiSe);
		if (!bQiXiaoDui && rule.checkPengPengHu(lstTemp))
			_huStyle |= static_cast<int>(MahjongGenre::HuStyle::PengPengHu);

		return true;
	}
}