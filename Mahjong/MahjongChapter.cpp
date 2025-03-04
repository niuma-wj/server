// MahjongChapter.cpp

#include "MahjongChapter.h"

namespace NiuMa
{
	MahjongChapter::MahjongChapter()
		: _vetoed(false)
		, targetPlayer(0)
		, targetTile(-1)
	{
		for (int i = 0; i < 3; i++) {
			types[i] = static_cast<int>(Type::Invalid);
			actionIds[i] = 0;
		}
	}

	MahjongChapter::MahjongChapter(const MahjongChapter& mc)
		: _vetoed(mc.isVetoed())
		, targetPlayer(mc.getTargetPlayer())
		, targetTile(mc.getTargetTile())
		, tiles(mc.getAllTiles())
	{
		mc.getTypes(types, actionIds);
	}

	MahjongChapter::~MahjongChapter()
	{}

	MahjongChapter& MahjongChapter::operator=(const MahjongChapter& mc) {
		_vetoed = mc.isVetoed();
		mc.getTypes(types, actionIds);
		targetPlayer = mc.getTargetPlayer();
		targetTile = mc.getTargetTile();
		tiles = mc.getAllTiles();
		return *this;
	}

	bool MahjongChapter::isVetoed() const {
		return _vetoed;
	}

	void MahjongChapter::setVetoed() {
		_vetoed = true;
	}

	MahjongChapter::Type MahjongChapter::getType() const {
		Type eType = Type::Invalid;
		for (unsigned int i = 0; i < 3; i++) {
			if (types[i] == static_cast<int>(Type::Invalid))
				break;
			eType = static_cast<Type>(types[i]);
		}
		return eType;
	}

	MahjongChapter::Type MahjongChapter::getType(int actionId) const {
		Type eType = Type::Invalid;
		for (unsigned int i = 0; i < 3; i++) {
			if (actionId < actionIds[i])
				continue;
			eType = static_cast<Type>(types[i]);
			break;
		}
		return eType;
	}

	void MahjongChapter::getTypes(int types_[], int actionIds_[]) const {
		for (unsigned int i = 0; i < 3; i++) {
			types_[i] = types[i];
			actionIds_[i] = actionIds[i];
		}
	}

	void MahjongChapter::addType(Type t, int actionId) {
		for (unsigned int i = 0; i < 3; i++) {
			if (static_cast<int>(Type::Invalid) == types[i]) {
				types[i] = static_cast<int>(t);
				actionIds[i] = actionId;
				break;
			}
		}
	}

	bool MahjongChapter::hasActionId(int actionId) const {
		bool bRet = false;
		for (unsigned int i = 0; i < 3; i++) {
			if (actionIds[i] == actionId) {
				bRet = (types[i] != static_cast<int>(Type::Invalid));
				break;
			}
		}
		return bRet;
	}

	bool MahjongChapter::isGang() const {
		Type type = getType();
		if (type == Type::ZhiGang ||
			type == Type::JiaGang ||
			type == Type::AnGang)
			return true;
		return false;
	}

	int MahjongChapter::getTargetPlayer() const {
		return targetPlayer;
	}

	void MahjongChapter::setTargetPlayer(int p) {
		targetPlayer = p;
	}

	int MahjongChapter::getTargetTile() const {
		return targetTile;
	}

	void MahjongChapter::setTargetTile(int id) {
		targetTile = id;
	}

	MahjongTileArray& MahjongChapter::getAllTiles() {
		return tiles;
	}

	const MahjongTileArray& MahjongChapter::getAllTiles() const {
		return tiles;
	}

	void MahjongChapter::setAllTiles(const MahjongTileArray& tiles_) {
		tiles = tiles_;
	}

	void MahjongChapter::hideAnGangTiles() {
		if (getType() != Type::AnGang)
			return;
		tiles.clear();
	}
}