// StandardMahjongMessages.cpp

#include "StandardMahjongMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgMahjongSync::TYPE("MsgMahjongSync");

	const std::string MsgMahjongSyncResp::TYPE("MsgMahjongSyncResp");

	MsgMahjongSyncResp::MsgMahjongSyncResp()
		: gold(0)
		, diamond(0)
		, mode(0)
		, diZhu(0)
		, chi(false)
		, dianPao(false)
		, hasFetch(false)
		, seat(0)
		, roundState(0)
		, disbandState(0)
		, banker(0)
		, leftTiles(0)
	{
		for (int i = 0; i < 4; i++)
			handTileNums[i] = 0;
	}

	MsgMahjongSyncResp::~MsgMahjongSyncResp() {}

	const std::string MsgMahjongStartRound::TYPE("MsgMahjongStartRound");

	MsgMahjongStartRound::MsgMahjongStartRound()
		: banker(0)
		, diamond(0)
	{}

	MsgMahjongStartRound::~MsgMahjongStartRound() {}

	const std::string MsgMahjongSettlement::TYPE("MsgMahjongSettlement");

	MsgMahjongSettlement::MsgMahjongSettlement()
		: kick(false)
	{
		for (int i = 0; i < 4; i++) {
			golds[i] = 0LL;
			winGolds[i] = 0LL;
		}
	}

	MsgMahjongSettlement::~MsgMahjongSettlement() {}

	const std::string MsgMahjongDisbandVote::TYPE("MsgMahjongDisbandVote");

	MsgMahjongDisbandVote::MsgMahjongDisbandVote()
		: disbander(0)
		, elapsed(0)
	{
		for (int i = 0; i < 4; i++)
			choices[i] = 0;
	}

	MsgMahjongDisbandVote::~MsgMahjongDisbandVote() {}

	void StandardMahjongMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgMahjongSync>());
		MessageManager::getSingleton().registCreator(MsgMahjongSync::TYPE, creator);
	}
}