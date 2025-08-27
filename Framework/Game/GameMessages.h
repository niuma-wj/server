// GameMessages.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.26

#ifndef _NIU_MA_GAME_MESSAGES_H_
#define _NIU_MA_GAME_MESSAGES_H_

#include "Venue/VenueMessages.h"

namespace NiuMa
{
	class GameMessages
	{
	private:
		GameMessages() {}

	public:
		virtual ~GameMessages() {}

		static void registMessages();
	};

	/**
	 * 加入游戏消息，由观众变为玩家
	 * 客户端->服务器
	 */
	class MsgJoinGame : public MsgVenueInner
	{
	public:
		MsgJoinGame();
		virtual ~MsgJoinGame() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 座位号
		int seat;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, seat);
	};

	/**
	 * 响应加入游戏消息
	 * 服务器->客户端
	 */
	class MsgJoinGameResp : public MsgBase
	{
	public:
		MsgJoinGameResp();
		virtual ~MsgJoinGameResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 座位号
		int seat;

		// 是否成功加入
		bool success;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(seat, success, errMsg);
	};

	/**
	 * 退出游戏消息，由玩家变为观众
	 * 客户端->服务器
	 */
	class MsgBecomeSpectator : public MsgVenueInner
	{
	public:
		MsgBecomeSpectator() {}
		virtual ~MsgBecomeSpectator() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应退出游戏消息
	 * 服务器->客户端
	 */
	class MsgBecomeSpectatorResp : public MsgBase
	{
	public:
		MsgBecomeSpectatorResp();
		virtual ~MsgBecomeSpectatorResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 结果，0-成功，其他失败原因
		int result;

		// 错误消息
		std::string errMsg;

		MSGPACK_DEFINE_MAP(result, errMsg);
	};

	/**
	 * 玩家信息
	 */
	typedef struct _AvatarInfo {
		// 玩家id
		std::string playerId;

		// 昵称
		std::string nickname;

		// 头像图片链接
		std::string headUrl;

		// 座位号
		int seat = -1;

		// 性别
		int sex = 0;

		// 就绪
		bool ready = false;

		// 离线
		bool offline = false;

		// 额外信息(json打包成base64)
		std::string base64;

		MSGPACK_DEFINE_MAP(playerId, nickname, headUrl, seat, sex, ready, offline, base64);
	} AvatarInfo;

	/**
	 * 观众信息
	 */
	typedef struct _SpectatorInfo {
		// 玩家id
		std::string playerId;

		// 昵称
		std::string nickname;

		// 头像图片链接
		std::string headUrl;

		// 额外信息(json打包成base64)
		std::string base64;

		MSGPACK_DEFINE_MAP(playerId, nickname, headUrl, base64);
	} SpectatorInfo;

	/**
	 * 添加玩家消息
	 * 服务器->客户端
	 */
	class MsgAddAvatar : public MsgBase {
	public:
		MsgAddAvatar() {}
		virtual ~MsgAddAvatar() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<AvatarInfo> avatars;

		MSGPACK_DEFINE_MAP(avatars);
	};

	/**
	 * 删除玩家消息
	 * 服务器->客户端
	 */
	class MsgRemoveAvatar : public MsgBase {
	public:
		MsgRemoveAvatar()
			: seat(0)
		{}

		virtual ~MsgRemoveAvatar() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::string playerId;

		//
		int seat;

		MSGPACK_DEFINE_MAP(playerId, seat);
	};

	/**
	 * 添加观众消息
	 * 服务器->客户端
	 */
	class MsgAddSpectator : public MsgBase {
	public:
		MsgAddSpectator() {}
		virtual ~MsgAddSpectator() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::vector<SpectatorInfo> spectators;

		MSGPACK_DEFINE_MAP(spectators);
	};

	/**
	 * 删除观众消息
	 * 服务器->客户端
	 */
	class MsgRemoveSpectator : public MsgBase {
	public:
		MsgRemoveSpectator() {}
		virtual ~MsgRemoveSpectator() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		std::string playerId;

		MSGPACK_DEFINE_MAP(playerId);
	};

	/**
	 * 玩家上下线消息
	 * 服务器->客户端
	 */
	class MsgAvatarConnect : public MsgBase {
	public:
		MsgAvatarConnect() {}
		virtual ~MsgAvatarConnect() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::string playerId;

		//
		std::string ip;

		//
		int seat = -1;

		//
		bool offline = false;

		MSGPACK_DEFINE_MAP(playerId, ip, seat, offline);
	};

	/**
	 * 获取全部玩家信息
	 * 客户端->服务器
	 */
	class MsgGetAvatars : public MsgVenueInner {
	public:
		MsgGetAvatars() {}
		virtual ~MsgGetAvatars() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 获取全部观众信息
	 * 客户端->服务器
	 */
	class MsgGetSpectators : public MsgVenueInner {
	public:
		MsgGetSpectators() {}
		virtual ~MsgGetSpectators() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 通知房间内各玩家的钻石数量消息
	 * 服务器->客户端
	 */
	class MsgPlayerDiamonds : public MsgBase {
	public:
		MsgPlayerDiamonds() {}
		virtual ~MsgPlayerDiamonds() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::vector<std::string> playerIds;

		//
		std::vector<int> seats;

		//
		std::vector<int64_t> diamonds;

		MSGPACK_DEFINE_MAP(playerIds, seats, diamonds);
	};

	/**
	 * 玩家就绪
	 * 客户端->服务器
	 */
	class MsgPlayerReady : public MsgVenueInner {
	public:
		MsgPlayerReady() {}
		virtual ~MsgPlayerReady() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应玩家就绪消息
	 * 服务器->客户端
	 */
	class MsgPlayerReadyResp : public MsgBase {
	public:
		MsgPlayerReadyResp();
	   virtual ~MsgPlayerReadyResp() {}

	   static const std::string TYPE;

	   virtual const std::string& getType() const {
		   return TYPE;
	   }

	   MSG_PACK_IMPL

   public:
	   //
	   std::string playerId;

	   //
	   int seat;

	   MSGPACK_DEFINE_MAP(playerId, seat);
	};

	/**
	 * 玩家请求托管(由系统自动出牌)消息
	 * 客户端->服务器
	 */
	class MsgPlayerAuthorize : public MsgVenueInner
	{
	public:
		MsgPlayerAuthorize() {}
		virtual ~MsgPlayerAuthorize() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应玩家托管消息
	 * 服务器->客户端
	 */
	class MsgPlayerAuthorizeResp : public MsgBase
	{
	public:
		MsgPlayerAuthorizeResp();
		virtual ~MsgPlayerAuthorizeResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 座位号
		int seat;

		// 玩家id
		std::string playerId;

		// 是否托管
		bool authorize;

		MSGPACK_DEFINE_MAP(seat, playerId, authorize);
	};

	/**
	 * 玩家定位消息
	 * 客户端->服务器
	 */
	class MsgPlayerGeolocation : public MsgVenueInner {
	public:
		MsgPlayerGeolocation();
		virtual ~MsgPlayerGeolocation() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 纬度
		double latitude;

		// 经度
		double longitude;

		// 海拔
		double altitude;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, latitude, longitude, altitude);
	};

	/**
	 * 查询所有玩家间的距离
	 * 客户端->服务器
	 */
	class MsgGetDistances : public MsgVenueInner {
	public:
		MsgGetDistances() {}
		virtual ~MsgGetDistances() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 响应所有玩家间的距离
	 * 服务器->客户端
	 */
	class MsgGetDistancesResp : public MsgBase {
	public:
		MsgGetDistancesResp() {}
		virtual ~MsgGetDistancesResp() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		//
		std::vector<int> distances;

		MSGPACK_DEFINE_MAP(distances);
	};

	/**
	 * 玩家请求解散房间消息
	 * 客户端->服务器
	 */
	class MsgDisbandRequest : public MsgVenueInner
	{
	public:
		MsgDisbandRequest() {}
		virtual ~MsgDisbandRequest() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId);
	};

	/**
	 * 解散选择消息
	 * 客户端->服务器
	 */
	class MsgDisbandChoose : public MsgVenueInner
	{
	public:
		MsgDisbandChoose();
		virtual ~MsgDisbandChoose();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 选择，1-同意、2-反对
		int choice;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, choice);
	};

	/**
	 * 通知玩家做的解散选择
	 * 服务器->客户端
	 */
	class MsgDisbandChoice : public MsgBase
	{
	public:
		MsgDisbandChoice();
		virtual ~MsgDisbandChoice();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 玩家id
		std::string playerId;

		// 玩家座位号
		int seat;

		// 选择
		int choice;

		MSGPACK_DEFINE_MAP(playerId, seat, choice);
	};

	/**
	 * 通知游戏已解散
	 * 服务器->客户端
	 */
	class MsgDisband : public MsgBase
	{
	public:
		MsgDisband() {}
		virtual ~MsgDisband() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符
		int placeholder = 0;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * 通知取消解散
	 * 服务器->客户端
	 */
	class MsgDisbandObsolete : public MsgBase
	{
	public:
		MsgDisbandObsolete() {}
		virtual ~MsgDisbandObsolete() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 占位符
		int placeholder = 0;

		MSGPACK_DEFINE_MAP(placeholder);
	};

	/**
	 * 玩家发送聊天消息
	 * 客户端->服务器
	 */
	class MsgChatClient : public MsgVenueInner
	{
	public:
		MsgChatClient();
		virtual ~MsgChatClient();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 消息类型，1-表情，2-常用语，3-普通文本，4-meme(静态搞笑图)
		int type;

		// 索引，当type为1、2、4时有效
		int index;

		// 文本
		std::string text;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, type, index, text);
	};

	/**
	 * 聊天消息
	 * 服务器->客户端
	 */
	class MsgChatServer : public MsgBase
	{
	public:
		MsgChatServer();
		virtual ~MsgChatServer();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 发聊天消息的玩家座位号
		int seat;

		// 消息类型，1-表情，2-常用语，3-普通文本
		int type;

		// 消息索引
		int index;

		// 消息文本
		std::string text;

		// 发聊天消息的玩家Id
		std::string playerId;

		MSGPACK_DEFINE_MAP(seat, type, index, text, playerId);
	};

	/**
	 * 投掷特效(如鸡蛋、鲜花等)消息
	 * 客户端->服务器
	 */
	class MsgEffectClient : public MsgVenueInner
	{
	public:
		MsgEffectClient();
		virtual ~MsgEffectClient();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 特效索引
		int index;

		// 被投掷(目标)玩家id
		std::string targetId;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, index, targetId);
	};

	/**
	 * 投掷特效(如鸡蛋、鲜花等)消息
	 * 服务器->客户端
	 */
	class MsgEffectServer : public MsgBase
	{
	public:
		MsgEffectServer();
		virtual ~MsgEffectServer();

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 特效索引
		int index;

		// 投掷玩家座位号
		int srcSeat;

		// 被投掷(目标)玩家座位号
		int dstSeat;

		MSGPACK_DEFINE_MAP(index, srcSeat, dstSeat);
	};

	/**
	 * 语音消息
	 * 客户端->服务器
	 */
	class MsgVoiceClient : public MsgVenueInner
	{
	public:
		MsgVoiceClient() {}
		virtual ~MsgVoiceClient() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

	public:
		// 将语音mp3文件打包成base64
		std::string base64;

		MSGPACK_DEFINE_MAP(playerId, timestamp, nonce, signature, venueId, base64);
	};

	/**
	 * 语音消息
	 * 服务器->客户端
	 */
	class MsgVoiceServer : public MsgBase
	{
	public:
		MsgVoiceServer();
		virtual ~MsgVoiceServer() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 语音聊天玩家座位号
		int seat;

		// 语音聊天玩家id
		std::string playerId;

		// 将语音mp3文件打包成base64
		std::string base64;

		MSGPACK_DEFINE_MAP(seat, playerId, base64);
	};

	/**
	 * 提示文本消息
	 * 服务器->客户端
	 */
	class MsgTipText : public MsgBase
	{
	public:
		MsgTipText() {}
		virtual ~MsgTipText() {}

		static const std::string TYPE;

		virtual const std::string& getType() const {
			return TYPE;
		}

		MSG_PACK_IMPL

	public:
		// 提示文本
		std::string tip;

		MSGPACK_DEFINE_MAP(tip);
	};
}

#endif // !_NIU_MA_GAME_MESSAGES_H_