// LackeyMessages.cpp

#include "LackeyMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgLackeySync::TYPE("MsgLackeySync");

	const std::string MsgLackeySyncResp::TYPE("MsgLackeySyncResp");

	MsgLackeySyncResp::MsgLackeySyncResp()
		: level(0)
		, mode(0)
		, diZhu(0)
		, seat(0)
		, landlord(0)
		, gameState(0)
	{}

	const std::string MsgLackeyWinLose::TYPE("MsgLackeyWinLose");

	MsgLackeyWinLose::MsgLackeyWinLose()
		: seat(0)
		, win(0)
		, lose(0)
		, draw(0)
	{}

	const std::string MsgLackeyDealCard::TYPE("MsgLackeyDealCard");

	MsgLackeyDealCard::MsgLackeyDealCard()
		: landlord(0)
	{}

	const std::string MsgLackeyHandCard::TYPE("MsgLackeyHandCard");

	const std::string MsgLackeyWaitOption::TYPE("MsgLackeyWaitOption");

	MsgLackeyWaitOption::MsgLackeyWaitOption()
		: seat(0)
		, elapsed(0)
		, duration(0)
	{}

	const std::string MsgWaitCallLackey::TYPE("MsgWaitCallLackey");

	MsgWaitCallLackey::MsgWaitCallLackey()
		: elapsed(0)
	{}

	const std::string MsgDoCallLackey::TYPE("MsgDoCallLackey");

	MsgDoCallLackey::MsgDoCallLackey()
		: yes(false)
	{}

	const std::string MsgCallLackeyDone::TYPE("MsgCallLackeyDone");

	MsgCallLackeyDone::MsgCallLackeyDone()
		: placeholder(0)
	{}

	const std::string MsgLackeyCard::TYPE("MsgLackeyCard");

	const std::string MsgLackeySeat::TYPE("MsgLackeySeat");

	MsgLackeySeat::MsgLackeySeat()
		: seat(0)
		, playSound(0)
	{}

	const std::string MsgLackeyWaitShowCard::TYPE("MsgLackeyWaitShowCard");

	MsgLackeyWaitShowCard::MsgLackeyWaitShowCard()
		: elapsed(0)
		, duration(0)
	{}

	const std::string MsgLackeyDoShowCard::TYPE("MsgLackeyDoShowCard");

	MsgLackeyDoShowCard::MsgLackeyDoShowCard()
		: yes(false)
	{}

	const std::string MsgLackeyShowCard::TYPE("MsgLackeyShowCard");

	MsgLackeyShowCard::MsgLackeyShowCard()
		: seat(0)
		, role(0)
	{}

	const std::string MsgLackeyShowCardDone::TYPE("MsgLackeyShowCardDone");

	MsgLackeyShowCardDone::MsgLackeyShowCardDone()
		: show(false)
	{}

	const std::string MsgLackeyWaitPlayCard::TYPE("MsgLackeyWaitPlayCard");

	MsgLackeyWaitPlayCard::MsgLackeyWaitPlayCard()
		: firstPlay(false)
		, canPlay(false)
		, elapsed(0)
	{}

	const std::string MsgLackeyDoPlayCard::TYPE("MsgLackeyDoPlayCard");

	MsgLackeyDoPlayCard::MsgLackeyDoPlayCard()
		: pass(false)
	{}

	const std::string MsgLackeyPlayCardFailed::TYPE("MsgLackeyPlayCardFailed");

	MsgLackeyPlayCardFailed::MsgLackeyPlayCardFailed()
		: reason(0)
	{}

	const std::string MsgLackeyPlayCard::TYPE("MsgLackeyPlayCard");

	MsgLackeyPlayCard::MsgLackeyPlayCard()
		: seat(0)
		, xiQian(0)
		, genre(0)
		, pass(false)
		, realTime(true)
	{}

	const std::string MsgLackeyXiQian::TYPE("MsgLackeyXiQian");

	MsgLackeyXiQian::MsgLackeyXiQian()
		: seat(0)
		, xiQian(0)
		, realTime(true)
	{}

	const std::string MsgLackeyCardNums::TYPE("MsgLackeyCardNums");

	MsgLackeyCardNums::MsgLackeyCardNums()
		: seat(0)
		, cardNums(0)
	{}

	const std::string MsgLackeyCardAlert::TYPE("MsgLackeyCardAlert");

	MsgLackeyCardAlert::MsgLackeyCardAlert()
		: seat(0)
	{}

	const std::string MsgLackeyHintCard::TYPE("MsgLackeyHintCard");

	const std::string MsgLackeyHintCardResp::TYPE("MsgLackeyHintCardResp");

	const std::string MsgLackeyLeftCards::TYPE("MsgLackeyLeftCards");

	LackeyResult::LackeyResult()
		: score(0.0f)
		, xiQian(0)
		, winGold(0)
		, gold(0LL)
		, showCard(false)
	{}

	const std::string MsgLackeyResult::TYPE("MsgLackeyResult");

	MsgLackeyResult::MsgLackeyResult()
		: beiLv(0)
		, first(0)
		, kick(false)
	{}

	const std::string MsgLackeyDisbandVote::TYPE("MsgLackeyDisbandVote");

	MsgLackeyDisbandVote::MsgLackeyDisbandVote()
		: disbander(0)
		, elapsed(0)
	{
		for (int i = 0; i < 5; i++)
			choices[i] = 0;
	}

	void LackeyMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgLackeySync>());
		MessageManager::getSingleton().registCreator(MsgLackeySync::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgDoCallLackey>());
		MessageManager::getSingleton().registCreator(MsgDoCallLackey::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgLackeyDoShowCard>());
		MessageManager::getSingleton().registCreator(MsgLackeyDoShowCard::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgLackeyDoPlayCard>());
		MessageManager::getSingleton().registCreator(MsgLackeyDoPlayCard::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgLackeyHintCard>());
		MessageManager::getSingleton().registCreator(MsgLackeyHintCard::TYPE, creator);
	}
}