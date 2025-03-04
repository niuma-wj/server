// StandardMahjongRoom.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.03

#ifndef _NIU_MA_STANDARD_MAHJONG_ROOM_H_
#define _NIU_MA_STANDARD_MAHJONG_ROOM_H_

#include "MahjongRoom.h"
#include "../GameDefines.h"

namespace NiuMa
{
	/**
	 * 标准麻将游戏房
	 */
	class StandardMahjongRoom : public MahjongRoom
	{
	public:
		StandardMahjongRoom(const MahjongRule::Ptr& rule, const std::string& venueId, const std::string& number, int mode, int diZhu, int config);
		virtual ~StandardMahjongRoom();

	private:
		// 尺表
		static const int RULER_TABLE[6];

	public:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;
		virtual void onTimer() override;

		// 重写
	protected:
		virtual GameAvatar::Ptr createAvatar(const std::string& playerId, int seat, bool robot) const override;
		virtual bool checkEnter(const std::string& playerId, std::string& errMsg) const override;
		virtual int checkLeave(const std::string& playerId, std::string& errMsg) const override;
		virtual void getAvatarExtraInfo(const GameAvatar::Ptr& avatar, std::string& base64) const override;
		virtual void onAvatarLeaved(int seat, const std::string& playerId) override;
		virtual void clean() override;
		virtual double* getDistances() override;
		virtual void getDistances(std::vector<int>& distances) const override;
		virtual int getDistanceIndex(int seat1, int seat2) const override;
		virtual void calcHuScore() const override;
		virtual void doJieSuan() override;
		virtual void afterHu() override;

	private:
		/**
		 * 玩家请求同步游戏数据
		 */
		void onSyncMahjong(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家就绪
		 */
		void onPlayerReady(const NetMessage::Ptr& netMsg);

		/**
		 * 请求解散游戏
		 */
		void onDisbandRequest(const NetMessage::Ptr& netMsg);

		/**
		 * 玩家解散选择消息
		 */
		void onDisbandChoose(const NetMessage::Ptr& netMsg);

		/**
		 * 开始新一局
		 */
		void startRound();

		/**
		 * 每局开始扣除钻石
		 * @param diamonds 返回扣除后每个玩家当前的钻石数量
		 */
		void deductDiamond(int64_t diamonds[4]);

		/**
		 * 通知发起解散投票
		 * @param playerId 通知指定玩家，为空则通知全部
		 */
		void notifyDisbandVote(const std::string& playerId);

		/**
		 * 玩家做解散选择
		 * @param seat 玩家座位号
		 * @param choice 解散选择，1-同意、2-反对
		 */
		void doDisbandChoose(int seat, int choice);

		/**
		 * 解散游戏
		 */
		void disbandRoom();

		/**
		 * 取消解散
		 */
		void disbandObsolete();

		/**
		 * 保存一局游戏记录
		 */
		void saveRoundRecord();

	private:
		/**
		 * 房间编号，用于手动输入进入房间
		 */
		const std::string _number;

		/**
		 * 0-扣钻模式，1-抽水模式
		 */
		const int _mode;

		/**
		 * 底注索引
		 */
		const int _diZhu;

		/**
		 * 配置
		 */
		const int _config;

		/**
		 * 牌局状态
		 */
		StageState _roundState;

		/**
		 * 解散状态
		 */
		StageState _disbandState;

		/**
		 * 局号数，每局递增
		 */
		int _roundNo;

		/**
		 * 备份本局的庄家位置(在保存牌局记录时使用)
		 */
		int _backupBanker;

		/**
		 * 解散房间的玩家索引
		 */
		int _disbander;

		/**
		 * 房间进入投票解散状态的时间
		 */
		time_t _disbandTick;

		/**
		 * 解散投票，0-未选择、1-同意、2-反对
		 */
		int _disbandChoices[4];

		/**
		 * 玩家之间的距离，0-1、0-2、0-3、1-2、1-3、2-3
		 * 小于0表示无距离数值
		 */
		double _distances[6];

		/**
		 * 本局结束后被踢出房间的玩家
		 */
		bool _kicks[4];
	};
}

#endif // !_NIU_MA_STANDARD_MAHJONG_ROOM_H_
