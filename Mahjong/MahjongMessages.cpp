// MahjongMessages.cpp

#include "MahjongMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgMahjongTiles::TYPE("MsgMahjongTiles");

	MsgMahjongTiles::MsgMahjongTiles()
	{}

	MsgMahjongTiles::~MsgMahjongTiles()
	{}

	const std::string MsgActorUpdated::TYPE("MsgActorUpdated");

	MsgActorUpdated::MsgActorUpdated()
		: actor(0)
	{}

	MsgActorUpdated::~MsgActorUpdated() {}

	const std::string MsgWaitAction::TYPE("MsgWaitAction");

	MsgWaitAction::MsgWaitAction()
		: waitting(false)
		, second(0)
		, beingHeld(false)
	{}

	MsgWaitAction::~MsgWaitAction() {}

	const std::string MsgFetchTile::TYPE("MsgFetchTile");

	MsgFetchTile::MsgFetchTile()
		: player(0)
		, nums(0)
		, back(false)
	{}

	MsgFetchTile::~MsgFetchTile() {}

	const std::string MsgActionOption::TYPE("MsgActionOption");

	MsgActionOption::MsgActionOption() {}

	MsgActionOption::~MsgActionOption() {}

	const std::string MsgDoActionOption::TYPE("MsgDoActionOption");

	MsgDoActionOption::MsgDoActionOption()
		: actionId(0)
		, tileId(0)
	{}

	MsgDoActionOption::~MsgDoActionOption() {}

	const std::string MsgPassActionOption::TYPE("MsgPassActionOption");

	MsgPassActionOption::MsgPassActionOption() {}

	MsgPassActionOption::~MsgPassActionOption() {}

	const std::string MsgActionOptionFinish::TYPE("MsgActionOptionFinish");

	MsgActionOptionFinish::MsgActionOptionFinish()
		: placeholder(0)
	{}

	MsgActionOptionFinish::~MsgActionOptionFinish() {}

	const std::string MsgPlayTile::TYPE("MsgPlayTile");

	MsgPlayTile::MsgPlayTile()
		: actor(0)
	{}

	MsgPlayTile::~MsgPlayTile()
	{}

	const std::string MsgGangTile::TYPE("MsgGangTile");

	MsgGangTile::MsgGangTile()
		: actor(0)
		, player(0)
		, tileNums(0)
		, chapter(0)
	{}

	MsgGangTile::~MsgGangTile() {}

	const std::string MsgPengChiTile::TYPE("MsgPengChiTile");

	MsgPengChiTile::MsgPengChiTile()
		: actor(0)
		, player(0)
		, pengOrChi(true)
	{}

	MsgPengChiTile::~MsgPengChiTile()
	{}

	const std::string MsgTingTile::TYPE("MsgTingTile");

	MsgTingTile::MsgTingTile() {}

	MsgTingTile::~MsgTingTile() {}

	const std::string MsgHuTile::TYPE("MsgHuTile");

	MsgHuTile::MsgHuTile()
		: actor(0)
		, ziMo(false)
	{
		for (int i = 0; i < 3; i++)
			players[i] = -1;
	}

	MsgHuTile::~MsgHuTile() {}

	const std::string MsgShowTiles::TYPE("MsgShowTiles");

	MsgShowTiles::MsgShowTiles() {}

	MsgShowTiles::~MsgShowTiles() {}

	const std::string MsgPassTip::TYPE("MsgPassTip");

	MsgPassTip::MsgPassTip()
		: action(0)
	{}

	MsgPassTip::~MsgPassTip() {}

	const std::string MsgNextTile::TYPE("MsgNextTile");

	MsgNextTile::MsgNextTile()
		: tileId(0)
	{}

	MsgNextTile::~MsgNextTile() {}

	void MahjongMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgDoActionOption>());
		MessageManager::getSingleton().registCreator(MsgDoActionOption::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgPassActionOption>());
		MessageManager::getSingleton().registCreator(MsgPassActionOption::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgNextTile>());
		MessageManager::getSingleton().registCreator(MsgNextTile::TYPE, creator);
	}
}