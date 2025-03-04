// BiJiMessages.cpp

#include "BiJiMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgBiJiSync::TYPE("MsgBiJiSync");

	const std::string MsgBiJiSyncResp::TYPE("MsgBiJiSyncResp");

	MsgBiJiSyncResp::MsgBiJiSyncResp()
		: mode(0)
		, diZhu(0)
		, seat(0)
		, commander(0)
		, gameState(0)
		, disbanding(false)
	{}

	MsgBiJiSyncResp::~MsgBiJiSyncResp()
	{}

	const std::string MsgBiJiCommander::TYPE("MsgBiJiCommander");

	MsgBiJiCommander::MsgBiJiCommander()
		: commander(0)
	{}

	MsgBiJiCommander::~MsgBiJiCommander()
	{}

	const std::string MsgBiJiGameState::TYPE("MsgBiJiGameState");

	MsgBiJiGameState::MsgBiJiGameState()
		: gameState(0)
		, elapsed(0)
	{}

	MsgBiJiGameState::~MsgBiJiGameState()
	{}

	const std::string MsgBiJiStartGame::TYPE("MsgBiJiStartGame");

	const std::string MsgBiJiStartGameResp::TYPE("MsgBiJiStartGameResp");

	const std::string MsgBiJiJoinRound::TYPE("MsgBiJiJoinRound");

	const std::string MsgBiJiDealCard::TYPE("MsgBiJiDealCard");

	MsgBiJiDealCard::MsgBiJiDealCard()
		: elapsed(0)
	{
		for (int i = 0; i < 9; i++) {
			orderRaw[i] = 0;
			orderSuit[i] = 0;
		}
	}

	MsgBiJiDealCard::~MsgBiJiDealCard()
	{}

	const std::string MsgBiJiMakeDun::TYPE("MsgBiJiMakeDun");

	MsgBiJiMakeDun::MsgBiJiMakeDun()
		: dun(0)
	{
		for (int i = 0; i < 3; i++)
			cardIds[i] = 0;
	}

	const std::string MsgBiJiMakeDunResp::TYPE("MsgBiJiMakeDunResp");

	MsgBiJiMakeDunResp::MsgBiJiMakeDunResp()
		: animate(false)
	{
		for (int i = 0; i < 3; i++) {
			duns[i] = false;
			dunIds1[i] = 0;
			dunIds2[i] = 0;
			dunIds3[i] = 0;
		}
	}

	MsgBiJiMakeDunResp::~MsgBiJiMakeDunResp() {}

	const std::string MsgBiJiSortDun::TYPE("MsgBiJiSortDun");

	MsgBiJiSortDun::MsgBiJiSortDun()
	{
		for (int i = 0; i < 3; i++) {
			duns[i] = false;
			dunIds1[i] = 0;
			dunIds2[i] = 0;
			dunIds3[i] = 0;
		}
	}

	MsgBiJiSortDun::~MsgBiJiSortDun() {}

	const std::string MsgBiJiRevocateDun::TYPE("MsgBiJiRevocateDun");

	MsgBiJiRevocateDun::MsgBiJiRevocateDun()
		: dun(0)
	{}

	const std::string MsgBiJiRevocateDunResp::TYPE("MsgBiJiRevocateDunResp");

	MsgBiJiRevocateDunResp::MsgBiJiRevocateDunResp()
		: dun(0)
	{
		for (int i = 0; i < 3; i++)
			cardIds[i] = 0;
	}

	const std::string MsgBiJiResetDun::TYPE("MsgBiJiResetDun");

	const std::string MsgBiJiResetDunResp::TYPE("MsgBiJiResetDunResp");

	MsgBiJiResetDunResp::MsgBiJiResetDunResp()
	{
		for (int i = 0; i < 9; i++)
			cardIds[i] = 0;
	}

	const std::string MsgBiJiFixDun::TYPE("MsgBiJiFixDun");

	const std::string MsgBiJiGiveUp::TYPE("MsgBiJiGiveUp");

	const std::string MsgBiJiFixDunResp::TYPE("MsgBiJiFixDunResp");

	MsgBiJiFixDunResp::MsgBiJiFixDunResp()
		: seat(0)
		, qiPai(false)
	{
		for (int i = 0; i < 9; i++)
			cardIds[i] = 0;
	}

	DunResult::DunResult()
		: seat(0)
		, genre(0)
		, score(0)
	{
		for (int i = 0; i < 3; i++)
			cards[i] = 0;
	}

	DunResult::DunResult(const DunResult& dr)
		: seat(dr.seat)
		, genre(dr.genre)
		, score(dr.score)
	{
		for (int i = 0; i < 3; i++)
			cards[i] = dr.cards[i];
	}

	DunResult& DunResult::operator=(const DunResult& dr) {
		seat = dr.seat;
		genre = dr.genre;
		score = dr.score;
		for (int i = 0; i < 3; i++)
			cards[i] = dr.cards[i];

		return *this;
	}

	const std::string MsgBiJiDunResult::TYPE("MsgBiJiDunResult");

	MsgBiJiDunResult::MsgBiJiDunResult()
		: dun(0)
		, animate(false)
	{}

	BiJiScore::BiJiScore()
		: seat(0)
		, reward(0)
		, total(0)
		, rewardType(0)
	{}

	const std::string MsgBiJiAggregate::TYPE("MsgBiJiAggregate");

	MsgBiJiAggregate::MsgBiJiAggregate()
		: animate(false)
	{}

	BiJiSettlement::BiJiSettlement()
		: seat(0)
		, totalScore(0)
		, winGold(0)
		, gold(0)
		, rewardType(0)
		, qiPai(false)
	{
		for (int i = 0; i < 3; i++) {
			dunScores[i] = 0;
			genres[i] = 0;
		}
		for (int i = 0; i < 9; i++)
			cards[i] = 0;
	}

	BiJiSettlement::BiJiSettlement(const BiJiSettlement& s)
		: seat(s.seat)
		, totalScore(s.totalScore)
		, winGold(s.winGold)
		, gold(s.gold)
		, rewardType(s.rewardType)
		, qiPai(s.qiPai)
	{
		for (int i = 0; i < 3; i++) {
			dunScores[i] = s.dunScores[i];
			genres[i] = s.genres[i];
		}
		for (int i = 0; i < 9; i++)
			cards[i] = s.cards[i];
	}

	BiJiSettlement& BiJiSettlement::operator=(const BiJiSettlement& s) {
		seat = s.seat;
		totalScore = s.totalScore;
		winGold = s.winGold;
		gold = s.gold;
		rewardType = s.rewardType;
		qiPai = s.qiPai;
		for (int i = 0; i < 3; i++) {
			dunScores[i] = s.dunScores[i];
			genres[i] = s.genres[i];
		}
		for (int i = 0; i < 9; i++)
			cards[i] = s.cards[i];
		return *this;
	}

	const std::string MsgBiJiSettlement::TYPE("MsgBiJiSettlement");

	MsgBiJiSettlement::MsgBiJiSettlement()
		: kick(false)
	{}

	void BiJiMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiSync>());
		MessageManager::getSingleton().registCreator(MsgBiJiSync::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiStartGame>());
		MessageManager::getSingleton().registCreator(MsgBiJiStartGame::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiMakeDun>());
		MessageManager::getSingleton().registCreator(MsgBiJiMakeDun::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiRevocateDun>());
		MessageManager::getSingleton().registCreator(MsgBiJiRevocateDun::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiResetDun>());
		MessageManager::getSingleton().registCreator(MsgBiJiResetDun::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiFixDun>());
		MessageManager::getSingleton().registCreator(MsgBiJiFixDun::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBiJiGiveUp>());
		MessageManager::getSingleton().registCreator(MsgBiJiGiveUp::TYPE, creator);
	}
}