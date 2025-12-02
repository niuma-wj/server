// GuanDanMessages.cpp

#include "GuanDanMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgGuanDanSync::TYPE("MsgGuanDanSync");

	const std::string MsgGuanDanSyncResp::TYPE("MsgGuanDanSyncResp");

	MsgGuanDanSyncResp::MsgGuanDanSyncResp()
		: level(-1)
		, ownerSeat(-1)
		, seat(-1)
		, gameState(0)
	{}

	const std::string MsgGuanDanSitting::TYPE("MsgGuanDanSitting");

	MsgGuanDanSitting::MsgGuanDanSitting()
		: gameState(0)
	{}

	const std::string MsgGuanDanOwnerSeat::TYPE("MsgGuanDanOwnerSeat");
	MsgGuanDanOwnerSeat::MsgGuanDanOwnerSeat()
		: ownerSeat(-1)
	{}

	const std::string MsgGuanDanStartGame::TYPE("MsgGuanDanStartGame");

	const std::string MsgGuanDanStartGameResp::TYPE("MsgGuanDanStartGameResp");

	MsgGuanDanStartGameResp::MsgGuanDanStartGameResp()
		: result(0)
	{}

	const std::string MsgGuanDanGradePoint::TYPE("MsgGuanDanGradePoint");
	MsgGuanDanGradePoint::MsgGuanDanGradePoint()
		: gradePointRed(2)
		, gradePointBlue(2)
		, banker(0)
		, realTime(false)
	{}

	const std::string MsgGuanDanDealCard::TYPE("MsgGuanDanDealCard");

	const std::string MsgGuanDanHandCard::TYPE("MsgGuanDanHandCard");

	MsgGuanDanHandCard::MsgGuanDanHandCard()
		: gradePoint(2)
		, contributeId(-1)
	{}

	const std::string MsgResistTribute::TYPE("MsgResistTribute");
	MsgResistTribute::MsgResistTribute()
		: seat1(-1)
		, seat2(-1)
	{}

	const std::string MsgWaitPresentTribute::TYPE("MsgWaitPresentTribute");

	MsgWaitPresentTribute::MsgWaitPresentTribute()
		: seat1(-1)
		, seat2(-1)
		, elapsed(0)
	{}

	const std::string MsgPresentTribute::TYPE("MsgPresentTribute");

	MsgPresentTribute::MsgPresentTribute()
		: cardId(-1)
	{}

	const std::string MsgPresentTributeResult::TYPE("MsgPresentTributeResult");
	
	MsgPresentTributeResult::MsgPresentTributeResult()
		: success(false)
		, cardId(-1)
	{}

	const std::string MsgWaitRefundTribute::TYPE("MsgWaitRefundTribute");

	MsgWaitRefundTribute::MsgWaitRefundTribute()
		: seat1(-1)
		, seat2(-1)
		, elapsed(0)
	{}

	const std::string MsgRefundTribute::TYPE("MsgRefundTribute");

	MsgRefundTribute::MsgRefundTribute()
		: cardId(-1)
	{}

	const std::string MsgRefundTributeResult::TYPE("MsgRefundTributeResult");

	MsgRefundTributeResult::MsgRefundTributeResult()
		: success(false)
	{}

	const std::string MsgTributeComplete::TYPE("MsgTributeComplete");

	const std::string MsgGuanDanWaitPlayCard::TYPE("MsgGuanDanWaitPlayCard");
	MsgGuanDanWaitPlayCard::MsgGuanDanWaitPlayCard()
		: elapsed(0)
		, seat(0)
		, firstPlay(false)
		, canPlay(false)
	{}

	const std::string MsgGuanDanDoPlayCard::TYPE("MsgGuanDanDoPlayCard");
	MsgGuanDanDoPlayCard::MsgGuanDanDoPlayCard()
		: pass(false)
	{}

	const std::string MsgGuanDanPlayCardFailed::TYPE("MsgGuanDanPlayCardFailed");
	MsgGuanDanPlayCardFailed::MsgGuanDanPlayCardFailed()
		: reason(0)
	{}

	const std::string MsgGuanDanPlayCard::TYPE("MsgGuanDanPlayCard");
	MsgGuanDanPlayCard::MsgGuanDanPlayCard()
		: seat(0)
		, genre(0)
		, pass(false)
		, firstPlay(false)
		, realTime(false)
	{}

	const std::string MsgGuanDanCardNums::TYPE("MsgGuanDanCardNums");
	MsgGuanDanCardNums::MsgGuanDanCardNums()
		: seat(0)
		, nums(0)
	{}

	const std::string MsgGuanDanCardAlert::TYPE("MsgGuanDanCardAlert");
	MsgGuanDanCardAlert::MsgGuanDanCardAlert()
		: seat(0)
	{}

	const std::string MsgGuanDanHintCard::TYPE("MsgGuanDanHintCard");

	const std::string MsgGuanDanResetHintCard::TYPE("MsgGuanDanResetHintCard");

	const std::string MsgGuanDanHintCardResp::TYPE("MsgGuanDanHintCardResp");

	const std::string MsgGuanDanHintStraightFlush::TYPE("MsgGuanDanHintStraightFlush");

	const std::string MsgGuanDanClearPlayedOut::TYPE("MsgGuanDanClearPlayedOut");
	MsgGuanDanClearPlayedOut::MsgGuanDanClearPlayedOut()
		: seat(0)
	{}

	const std::string MsgGuanDanFinished::TYPE("MsgGuanDanFinished");
	MsgGuanDanFinished::MsgGuanDanFinished()
		: touYou(-1)
		, erYou(-1)
	{}

	const std::string MsgGuanDanJieFeng::TYPE("MsgGuanDanJieFeng");
	MsgGuanDanJieFeng::MsgGuanDanJieFeng()
		: seat(0)
	{}

	const std::string MsgGuanDanResult::TYPE("MsgGuanDanResult");
	MsgGuanDanResult::MsgGuanDanResult()
		: gradePointNext(2)
	{
		for (int i = 0; i < 4; i++) {
			finishedSeats[i] = -1;
			kicks[i] = false;
		}
	}

	const std::string MsgGuanDanDisbandVote::TYPE("MsgGuanDanDisbandVote");
	MsgGuanDanDisbandVote::MsgGuanDanDisbandVote()
		: disbander(0)
		, elapsed(0)
	{
		for (int i = 0; i < 4; i++)
			choices[i] = 0;
	}

	void GuanDanMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanSync>());
		MessageManager::getSingleton().registCreator(MsgGuanDanSync::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanStartGame>());
		MessageManager::getSingleton().registCreator(MsgGuanDanStartGame::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgPresentTribute>());
		MessageManager::getSingleton().registCreator(MsgPresentTribute::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgRefundTribute>());
		MessageManager::getSingleton().registCreator(MsgRefundTribute::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanDoPlayCard>());
		MessageManager::getSingleton().registCreator(MsgGuanDanDoPlayCard::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanHintCard>());
		MessageManager::getSingleton().registCreator(MsgGuanDanHintCard::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanResetHintCard>());
		MessageManager::getSingleton().registCreator(MsgGuanDanResetHintCard::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGuanDanHintStraightFlush>());
		MessageManager::getSingleton().registCreator(MsgGuanDanHintStraightFlush::TYPE, creator);
	}
}