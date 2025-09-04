// VenueOuterHandler.cpp

#include "Base/Log.h"
#include "VenueOuterHandler.h"
#include "VenueManager.h"
#include "VenueMessages.h"
#include "Constant/RedisKeys.h"
#include "Constant/ErrorDefines.h"
#include "Redis/RedisPool.h"
#include "Message/MsgDisconnect.h"
#include "Player/PlayerManager.h"
#include "Player/PlayerMessages.h"

#include <boost/locale.hpp>

namespace NiuMa
{
	VenueOuterHandler::VenueOuterHandler(const MessageQueue::Ptr& queue)
		: PlayerSignatureHandler(queue)
	{
		// 添加接收的消息类型
		addMessage(MsgPlayerConnect::TYPE);
		addMessage(MsgDisconnect::TYPE);
		addMessage(MsgEnterVenue::TYPE);
		addMessage(MsgLeaveVenue::TYPE);
	}

	VenueOuterHandler::~VenueOuterHandler() {}

	bool VenueOuterHandler::onMessage(const NetMessage::Ptr& netMsg) {
		bool ret = true;
		const std::string& msgType = netMsg->getType();
		if (MsgPlayerConnect::TYPE == msgType)
			onPlayerConnect(netMsg);
		else if (MsgDisconnect::TYPE == msgType)
			onDisconnect(netMsg);
		else if (MsgEnterVenue::TYPE == msgType)
			onEnterVenue(netMsg);
		else if (MsgLeaveVenue::TYPE == netMsg->getType())
			onLeaveVenue(netMsg);
		else
			ret = false;
		return ret;
	}

	void VenueOuterHandler::onPlayerConnect(const NetMessage::Ptr& netMsg) {
		MsgPlayerConnect* msg = dynamic_cast<MsgPlayerConnect*>(netMsg->getMessage().get());
		if (msg == nullptr)
			return;
		Session::Ptr session = netMsg->getSession();
		if (!session)
			return;
		std::string playerId = msg->getPlayerId();
		Player::Ptr player = PlayerManager::getSingleton().getPlayer(playerId);
		if (!player)
			return;
		// 先将旧的sessionId与playerId撤销关联
		std::string sessionId;
		if (player->getSessionId(sessionId))
			PlayerManager::getSingleton().removeSessionId(sessionId);
		session->getId(sessionId);
		player->setSession(session);
		PlayerManager::getSingleton().setSessionPlayerId(sessionId, playerId);
		MsgPlayerConnectResp resp;
		resp.send(session);
		InfoS << "Player(id:" << playerId << ") connected, session id: " << sessionId;
		std::string venueId;
		player->getVenueId(venueId);
		if (venueId.empty())
			return;
		Venue::Ptr venue = VenueManager::getSingleton().getVenue(venueId);
		if (!venue)
			return;
		std::shared_ptr<VenueInnerHandler> handler = venue->getHandler();
		ThreadWorker::Ptr dispThread;
		if (handler)
			dispThread = handler->getWorker();
		if (!dispThread)
			return;
		std::weak_ptr<Venue> weakVenue = venue;
		dispThread->dispatch([weakVenue, playerId]() {
			Venue::Ptr strong = weakVenue.lock();
			if (strong)
				strong->onConnect(playerId);
			});
	}

	void VenueOuterHandler::onDisconnect(const NetMessage::Ptr& netMsg) {
		const MsgBase::Ptr& msg = netMsg->getMessage();
		if (!msg)
			return;
		MsgDisconnect* ptr = dynamic_cast<MsgDisconnect*>(msg.get());
		if (ptr == nullptr)
			return;
		const std::string& sessionId = ptr->getSessionId();
		Player::Ptr player = PlayerManager::getSingleton().getPlayerBySessionId(sessionId);
		if (!player)
			return;
		PlayerManager::getSingleton().removeSessionId(sessionId);
		player->setOffline();
		PlayerManager::getSingleton().addOfflinePlayer(player->getId());
		std::string venueId;
		player->getVenueId(venueId);
		if (venueId.empty())
			return;
		Venue::Ptr venue = VenueManager::getSingleton().getVenue(venueId);
		if (!venue)
			return;
		std::shared_ptr<VenueInnerHandler> handler = venue->getHandler();
		ThreadWorker::Ptr dispThread;
		if (handler)
			dispThread = handler->getWorker();
		if (!dispThread)
			return;
		std::string playerId = player->getId();
		std::weak_ptr<Venue> weakVenue = venue;
		dispThread->dispatch([weakVenue, playerId]() {
			Venue::Ptr strong = weakVenue.lock();
			if (strong)
				strong->onDisconnect(playerId);
			});
	}

	void VenueOuterHandler::onEnterVenue(const NetMessage::Ptr& netMsg) {
		MsgEnterVenue* msg = dynamic_cast<MsgEnterVenue*>(netMsg->getMessage().get());
		if (msg == nullptr)
			return;
		Session::Ptr session = netMsg->getSession();
		if (!session)
			return;
		std::string playerId = msg->getPlayerId();
		MsgEnterVenueResp resp;
		resp.venueId = msg->venueId;
		std::string authVenueId;
		std::string redisKey = RedisKeys::PLAYER_AUTHORIZED_VENUE + playerId;
		RedisPool::getSingleton().get(redisKey, authVenueId);
		if (authVenueId != msg->venueId) {
			resp.code = static_cast<int>(EnterVenueResult::LOAD_FAILED);
			resp.errMsg = "Haven't been authorized to enter the specified venue";
			resp.send(session);
			ErrorS << "Player(id: " << playerId << ") enter venue(id: " << msg->venueId << ", type: " << msg->gameType << ") failed, not authorized";
			return;
		}
		int error = 0;
		Venue::Ptr venue = VenueManager::getSingleton().loadVenue(msg->venueId, msg->gameType, error);
		if (!venue) {
			resp.code = error;
			if (error == static_cast<int>(EnterVenueResult::GAME_TYPE_ERROR))
				resp.errMsg = "Game type error";
			else if (error == static_cast<int>(EnterVenueResult::STATUS_ERROR))
				resp.errMsg = "Venue status error";
			else if (error == static_cast<int>(EnterVenueResult::LOAD_FAILED))
				resp.errMsg = "Load venue failed";
			else if (error == static_cast<int>(EnterVenueResult::DISTRIBUTE_FAILED))
				resp.errMsg = "Distribute venue failed";
			else if (error == static_cast<int>(EnterVenueResult::HANDLER_ERROR))
				resp.errMsg = "Message handler error";
			resp.send(session);
			ErrorS << "Player(id: " << playerId << ") enter venue(id: " << msg->venueId << ", type: " << msg->gameType << ") error: " << resp.errMsg;
			return;
		}
		std::shared_ptr<VenueInnerHandler> handler = venue->getHandler();
		ThreadWorker::Ptr dispThread;
		if (handler)
			dispThread = handler->getWorker();
		if (!dispThread) {
			resp.code = static_cast<int>(EnterVenueResult::UNKNOWN);
			resp.errMsg = "Internal server error";
			resp.send(session);
			ErrorS << "Player(id: " << playerId << ") enter venue(id: " << msg->venueId << ", type: " << msg->gameType << ") error: " << resp.errMsg;
			return;
		}
		std::weak_ptr<Venue> weakVenue = venue;
		std::string base64 = msg->base64;
		dispThread->dispatch([weakVenue, playerId, session, base64] {
			Venue::Ptr strong = weakVenue.lock();
			if (!strong)
				return;
			std::string errMsg;
			bool ret = strong->onEnter(playerId, base64, errMsg);
			if (!ret) {
				ErrorS << "Player(id: " << playerId << ") enter venue(id: " << strong->getId() << ", type: " << strong->getGameType() << ") error: " << errMsg;
			}
#ifdef _MSC_VER
			// VC环境下gb2312编码转utf8
			errMsg = boost::locale::conv::to_utf<char>(errMsg, std::string("gb2312"));
#endif
			MsgEnterVenueResp resp;
			resp.venueId = strong->getId();
			resp.code = ret ? 0 : static_cast<int>(EnterVenueResult::ENTER_FAILED);
			resp.errMsg = errMsg;
			resp.send(session);
		});
	}

	void VenueOuterHandler::onLeaveVenue(const NetMessage::Ptr& netMsg) {
		MsgLeaveVenue* inst = dynamic_cast<MsgLeaveVenue*>(netMsg->getMessage().get());
		if (inst == nullptr)
			return;
		ThreadWorker::Ptr dispThread;
		Venue::Ptr venue = VenueManager::getSingleton().getVenue(inst->getVenueId());
		if (venue) {
			std::shared_ptr<VenueInnerHandler> handler = venue->getHandler();
			if (handler)
				dispThread = handler->getWorker();
		}
		if (!dispThread) {
			// 场地不存在，返回直接返回离开成功
			MsgLeaveVenueResp resp;
			resp.venueId = inst->getVenueId();
			resp.result = 0;
			resp.send(netMsg->getSession());
			return;
		}
		NetMessage::Ptr tmpMsg = netMsg;
		std::weak_ptr<Venue> weakVenue = venue;
		dispThread->dispatch([weakVenue, tmpMsg] {
			Venue::Ptr strong = weakVenue.lock();
			if (strong)
				strong->onLeaveVenue(tmpMsg);
		});
	}
}