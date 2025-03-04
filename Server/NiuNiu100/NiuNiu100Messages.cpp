// NiuNiu100Messages.cpp

#include "NiuNiu100Messages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgNiu100Sync::TYPE("MsgNiu100Sync");

	const std::string MsgNiu100GameState::TYPE("MsgNiu100GameState");

	MsgNiu100GameState::MsgNiu100GameState()
		: gameState(0)
		, elapsed(0)
	{}

	const std::string MsgNiu100Compare::TYPE("MsgNiu100Compare");

	MsgNiu100Compare::MsgNiu100Compare()
		: elapsed(0)
		, gold(0)
		, deposit(0)
	{
		for (int i = 0; i < 4; i++) {
			multiples[i] = 0;
			scores[i] = 0;
			bankerScores[i] = 0;
		}
		for (int i = 0; i < 4; i++)
			rankScores1[i] = 0;
		for (int i = 0; i < 4; i++)
			rankScores2[i] = 0;
		for (int i = 0; i < 4; i++)
			rankScores3[i] = 0;
		for (int i = 0; i < 4; i++)
			rankScores4[i] = 0;
		for (int i = 0; i < 4; i++)
			rankScores5[i] = 0;
		for (int i = 0; i < 4; i++)
			rankScores6[i] = 0;
		for (int i = 0; i < 6; i++)
			rankGolds[i] = 0;
	}

	const std::string MsgNiu100SyncResp::TYPE("MsgNiu100SyncResp");

	MsgNiu100SyncResp::MsgNiu100SyncResp()
		: demo(false)
		, gameState(0)
		, deposit(0)
		, gold(0)
	{
		for (int i = 0; i < 4; i++) {
			betTotals[i] = 0;
			myBetAmounts[i] = 0;
		}
	}

	const std::string MsgNiu100Bet::TYPE("MsgNiu100Bet");

	MsgNiu100Bet::MsgNiu100Bet()
		: zone(0)
		, chip(0)
	{}

	const std::string MsgNiu100BetFailed::TYPE("MsgNiu100BetFailed");

	const std::string MsgNiu100BetSucess::TYPE("MsgNiu100BetSucess");

	MsgNiu100BetSucess::MsgNiu100BetSucess()
		: zone(0)
		, chip(0)
	{}

	const std::string MsgNiu100Settlement::TYPE("MsgNiu100Settlement");

	MsgNiu100Settlement::MsgNiu100Settlement()
		: score(0)
		, bankerScore(0)
	{
		for (int i = 0; i < 4; i++)
			winnerScores[i] = 0;
	}

	const std::string MsgNiu100Rank::TYPE("MsgNiu100Rank");

	MsgNiu100Rank::MsgNiu100Rank()
	{
		for (int i = 0; i < 6; i++)
			golds[i] = 0LL;
	}

	Niu100RankItem::Niu100RankItem()
		: gold(0LL)
		, accWins20(0)
		, accBets20(0LL)
	{}

	const std::string MsgNiu100GetRankList::TYPE("MsgNiu100GetRankList");

	const std::string MsgNiu100RankList::TYPE("MsgNiu100RankList");

	const std::string MsgNiu100GetTrend::TYPE("MsgNiu100GetTrend");

	const std::string MsgNiu100Trend::TYPE("MsgNiu100Trend");

	const std::string MsgNiu100BankerDisband::TYPE("MsgNiu100BankerDisband");

	void NiuNiu100Messages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgNiu100Sync>());
		MessageManager::getSingleton().registCreator(MsgNiu100Sync::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgNiu100Bet>());
		MessageManager::getSingleton().registCreator(MsgNiu100Bet::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgNiu100GetRankList>());
		MessageManager::getSingleton().registCreator(MsgNiu100GetRankList::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgNiu100GetTrend>());
		MessageManager::getSingleton().registCreator(MsgNiu100GetTrend::TYPE, creator);
	}
}