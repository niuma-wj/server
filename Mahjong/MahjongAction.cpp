// MahjongAction.cpp 

#include "MahjongAction.h"
#include "MahjongTile.h"

namespace NiuMa
{
	MahjongAction::MahjongAction()
		: type(static_cast<int>(Type::Invalid))
		, slot(-1)
		, tile(MahjongTile::INVALID_ID)
	{}

	MahjongAction::MahjongAction(const MahjongAction& ma)
		: type(static_cast<int>(ma.getType()))
		, slot(ma.getSlot())
		, tile(ma.getTile())
	{}

	MahjongAction::MahjongAction(MahjongAction::Type type_, int slot_, int tile_)
		: type(static_cast<int>(type_))
		, slot(slot_)
		, tile(tile_)
	{}

	MahjongAction::~MahjongAction()
	{}

	MahjongAction& MahjongAction::operator=(const MahjongAction& ma) {
		type = static_cast<int>(ma.getType());
		slot = ma.getSlot();
		tile = ma.getTile();
		return *this;
	}

	MahjongAction::Type MahjongAction::getType() const {
		return static_cast<MahjongAction::Type>(type);
	}

	void MahjongAction::setType(MahjongAction::Type t) {
		type = static_cast<int>(t);
	}

	int MahjongAction::getSlot() const
	{
		return slot;
	}

	void MahjongAction::setSlot(int s) {
		slot = s;
	}

	int MahjongAction::getTile() const {
		return tile;
	}

	void MahjongAction::setTile(int id) {
		tile = id;
	}

	MahjongActor::MahjongActor()
		: player(-1)
		, start(-1)
	{}

	MahjongActor::MahjongActor(const MahjongActor& ma)
		: player(ma.getPlayer())
		, start(ma.getStart())
	{}

	MahjongActor::MahjongActor(int p, int s)
		: player(p)
		, start(s)
	{}

	MahjongActor::~MahjongActor()
	{}

	MahjongActor& MahjongActor::operator=(const MahjongActor& ma) {
		player = ma.getPlayer();
		start = ma.getStart();
		return *this;
	}

	int MahjongActor::getPlayer() const {
		return player;
	}

	int MahjongActor::getStart() const {
		return start;
	}

	//
	MahjongActionOption::MahjongActionOption()
		: id(0)
		, type(static_cast<int>(MahjongAction::Type::Invalid))
		, player(-1)
		, tile1(MahjongTile::INVALID_ID)
		, tile2(MahjongTile::INVALID_ID)
	{}

	MahjongActionOption::MahjongActionOption(const MahjongActionOption& ao)
		: id(ao.getId())
		, type(static_cast<int>(ao.getType()))
		, player(ao.getPlayer())
		, tile1(ao.getTileId1())
		, tile2(ao.getTileId2())
	{}

	MahjongActionOption::~MahjongActionOption()
	{}

	MahjongAction::Type MahjongActionOption::getType() const {
		return static_cast<MahjongAction::Type>(type);
	}

	void MahjongActionOption::setType(MahjongAction::Type t) {
		type = static_cast<int>(t);
	}

	int MahjongActionOption::getId() const {
		return id;
	}

	void MahjongActionOption::setId(int id_) {
		id = id_;
	}

	int MahjongActionOption::getPlayer() const {
		return player;
	}

	void MahjongActionOption::setPlayer(int p) {
		player = p;
	}

	int MahjongActionOption::getTileId1() const {
		return tile1;
	}

	void MahjongActionOption::setTileId1(int id) {
		tile1 = id;
	}

	int MahjongActionOption::getTileId2() const {
		return tile2;
	}

	void MahjongActionOption::setTileId2(int id) {
		tile2 = id;
	}

	MahjongActionOption& MahjongActionOption::operator=(const MahjongActionOption& ma) {
		id = ma.getId();
		type = static_cast<int>(ma.getType());
		player = ma.getPlayer();
		tile1 = ma.getTileId1();
		tile2 = ma.getTileId2();
		return *this;
	}
}