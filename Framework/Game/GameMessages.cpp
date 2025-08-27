// GameMessages.cpp

#include "GameMessages.h"
#include "Message/MessageManager.h"

namespace NiuMa
{
	const std::string MsgJoinGame::TYPE("MsgJoinGame");
	const std::string MsgJoinGameResp::TYPE("MsgJoinGameResp");
	const std::string MsgBecomeSpectator::TYPE("MsgBecomeSpectator");
	const std::string MsgBecomeSpectatorResp::TYPE("MsgBecomeSpectatorResp");
	const std::string MsgAddAvatar::TYPE("MsgAddAvatar");
	const std::string MsgRemoveAvatar::TYPE("MsgRemoveAvatar");
	const std::string MsgAddSpectator::TYPE("MsgAddSpectator");
	const std::string MsgRemoveSpectator::TYPE("MsgRemoveSpectator");
	const std::string MsgAvatarConnect::TYPE("MsgAvatarConnect");
	const std::string MsgGetAvatars::TYPE("MsgGetAvatars");
	const std::string MsgGetSpectators::TYPE("MsgGetSpectators");
	const std::string MsgPlayerDiamonds::TYPE("MsgPlayerDiamonds");
	const std::string MsgPlayerReady::TYPE("MsgPlayerReady");
	const std::string MsgPlayerReadyResp::TYPE("MsgPlayerReadyResp");

	MsgJoinGame::MsgJoinGame()
		: seat(-1)
	{}

	MsgJoinGameResp::MsgJoinGameResp()
		: seat(-1)
		, success(false)
	{}

	MsgBecomeSpectatorResp::MsgBecomeSpectatorResp()
		: result(0)
	{}

	MsgPlayerReadyResp::MsgPlayerReadyResp()
		: seat(-1)
	{}

	const std::string MsgPlayerAuthorize::TYPE("MsgPlayerAuthorize");

	const std::string MsgPlayerAuthorizeResp::TYPE("MsgPlayerAuthorizeResp");

	MsgPlayerAuthorizeResp::MsgPlayerAuthorizeResp()
		: seat(0)
		, authorize(false)
	{}

	const std::string MsgPlayerGeolocation::TYPE("MsgPlayerGeolocation");

	MsgPlayerGeolocation::MsgPlayerGeolocation()
		: latitude(0.0)
		, longitude(0.0)
		, altitude(0.0)
	{}

	const std::string MsgGetDistances::TYPE("MsgGetDistances");

	const std::string MsgGetDistancesResp::TYPE("MsgGetDistancesResp");

	const std::string MsgDisbandRequest::TYPE("MsgDisbandRequest");

	const std::string MsgDisbandChoose::TYPE("MsgDisbandChoose");

	MsgDisbandChoose::MsgDisbandChoose()
		: choice(0)
	{}

	MsgDisbandChoose::~MsgDisbandChoose() {}

	const std::string MsgDisbandChoice::TYPE("MsgDisbandChoice");

	MsgDisbandChoice::MsgDisbandChoice()
		: seat(0)
		, choice(0)
	{}

	MsgDisbandChoice::~MsgDisbandChoice() {}

	const std::string MsgDisband::TYPE("MsgDisband");
	const std::string MsgDisbandObsolete::TYPE("MsgDisbandObsolete");

	const std::string MsgChatClient::TYPE("MsgChatClient");

	MsgChatClient::MsgChatClient()
		: type(0)
		, index(0)
	{}

	MsgChatClient::~MsgChatClient() {}

	const std::string MsgChatServer::TYPE("MsgChatServer");

	MsgChatServer::MsgChatServer()
		: seat(0)
		, type(0)
		, index(0)
	{}

	MsgChatServer::~MsgChatServer() {}

	const std::string MsgEffectClient::TYPE("MsgEffectClient");

	MsgEffectClient::MsgEffectClient()
		: index(0)
	{}

	MsgEffectClient::~MsgEffectClient() {}

	const std::string MsgEffectServer::TYPE("MsgEffectServer");
	
	MsgEffectServer::MsgEffectServer()
		: index(0)
		, srcSeat(0)
		, dstSeat(0)
	{}

	MsgEffectServer::~MsgEffectServer() {}

	const std::string MsgVoiceClient::TYPE("MsgVoiceClient");

	const std::string MsgVoiceServer::TYPE("MsgVoiceServer");

	MsgVoiceServer::MsgVoiceServer()
		: seat(0)
	{}

	const std::string MsgTipText::TYPE("MsgTipText");

	void GameMessages::registMessages() {
		IMsgCreator::Ptr creator = IMsgCreator::Ptr(new MsgCreator<MsgJoinGame>());
		MessageManager::getSingleton().registCreator(MsgJoinGame::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgBecomeSpectator>());
		MessageManager::getSingleton().registCreator(MsgBecomeSpectator::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGetAvatars>());
		MessageManager::getSingleton().registCreator(MsgGetAvatars::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGetSpectators>());
		MessageManager::getSingleton().registCreator(MsgGetSpectators::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgPlayerReady>());
		MessageManager::getSingleton().registCreator(MsgPlayerReady::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgPlayerAuthorize>());
		MessageManager::getSingleton().registCreator(MsgPlayerAuthorize::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgPlayerGeolocation>());
		MessageManager::getSingleton().registCreator(MsgPlayerGeolocation::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgGetDistances>());
		MessageManager::getSingleton().registCreator(MsgGetDistances::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgDisbandRequest>());
		MessageManager::getSingleton().registCreator(MsgDisbandRequest::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgDisbandChoose>());
		MessageManager::getSingleton().registCreator(MsgDisbandChoose::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgChatClient>());
		MessageManager::getSingleton().registCreator(MsgChatClient::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgEffectClient>());
		MessageManager::getSingleton().registCreator(MsgEffectClient::TYPE, creator);
		creator = IMsgCreator::Ptr(new MsgCreator<MsgVoiceClient>());
		MessageManager::getSingleton().registCreator(MsgVoiceClient::TYPE, creator);
	}
}