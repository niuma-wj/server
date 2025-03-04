// MahjongTable.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.11.30

#ifndef _NIU_MA_MAHJONG_TABLE_H_
#define _NIU_MA_MAHJONG_TABLE_H_

#include "Game/GameRoom.h"
#include "MahjongAvatar.h"
#include "MahjongAction.h"
#include "MahjongRule.h"
#include "MahjongDealer.h"
#include "MahjongSettlement.h"
#include "MahjongPlayback.h"

#include "Base/IdentityAllocator.h"

// ����ѡ���صĴ�С��Ԥ��20���϶�����
#define ACTION_OPTION_POOL_SIZE	20

namespace NiuMa
{
	class MahjongSettlement;
	/**
	 * �齫��Ϸ����
	 */
	class MahjongRoom : public GameRoom
	{
	public:
		MahjongRoom(const MahjongRule::Ptr& rule, const std::string& venueId, int gameType, int maxPlayerNums = 4);
		virtual ~MahjongRoom();

	protected:
		/**
		 * ���齫������Ϊһ��״̬����������״̬��һ���������ƺ��״̬A��һ���ǵȴ�ѡ����
		 * ѡ���״̬B����һ���ǵȴ����Ƶ�״̬C���������һ����֮�����A����ʱ���ܼӸܡ���
		 * �ܻ���������ʱ�ͽ���B������ʱ���ܽ���B�����̽���C�����һ����֮��������ҿ��ܽ�
		 * ��B��Ҳ��������һλ��ҽ���A������BҲ�����ٴν���B������Ӹ�֮��ȴ����������
		 * �ܡ�״̬�仯��˷���ֱ�����ƻ������֣�����״̬ͼ�ɸ����������״̬�ı�Ǩ�ƽ�
		 * �ƾֵ��ݻ������״ֹ̬ͣ�仯�����ƾ�Ҳ��ֹͣ��ǰ��
		 */
		enum class StateMachine : int
		{
			Null,
			Fetched,		// ȡ�ƺ�״̬A
			Action,			// �ȴ�����ѡ��״̬B
			Play,			// �ȴ�����״̬C
			End				// ����״̬D
		};

	public:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		// ����
		virtual void clean() override;

	private:
		/**
		 * ����ִ�ж���ѡ����Ϣ
		 * @param netMsg ������Ϣ
		 */
		void onActionOption(const NetMessage::Ptr& netMsg);

		/**
		 * ���������ѡ����Ϣ
		 * @param netMsg ������Ϣ
		 */
		void onPassActionOption(const NetMessage::Ptr& netMsg);

		/**
		 * ����ָ����һ������Ϣ�������ڹ��ܲ��ԣ�
		 * @param netMsg ������Ϣ
		 */
		void onNextTile(const NetMessage::Ptr& netMsg);

		/**
		 * ���ѡ����һ������ѡ��
		 * @param playerId ���id
		 * @param id ����ѡ��id
		 * @param tileId ��id
		 */
		void doActionOption(const std::string& playerId, int actionId, int tileId);

		// ��Ҳ�ѡ���κζ���ѡ���ѡ�񡰹���
		void passActionOption(const std::string& playerId);

		// �Զ�ִ�ж���ѡ��(����AI����)������bOnlyAuto��ʾ���ε����Ƿ������Զ��������
		void autoActionOption(bool bOnlyAuto);

		// ���ָ����һ���������(�����ڲ����齫����㷨����ȷ��)
		void doNextTile(const std::string& playerId, const std::string& str);

		// ���µ�ǰִ�ж������������
		void updateCurrentActor();

		// ���µ�ǰִ�ж������������
		void updateCurrentActor(int player);

		// ״̬��ת
		void changeState(StateMachine eNewState);

		// ȡ��
		bool fetchTile(bool bBack = false);

		// ���������ҵ����ж���ѡ��
		void clearActionOptions();

		// ���ָ����ҵ����ж���ѡ��
		void clearActionOptions(MahjongAvatar* pAvatar);

		// ִ������Ѿ�ѡ��Ķ���ѡ��
		bool executeActionOptions();

		// ִ�к�����
		bool executeHu();

		// ִ�иܶ���
		bool executeGang();

		// ִ��������
		bool executePeng();

		// ִ�гԶ���
		bool executeChi();

		// ִ�г��ƶ���
		bool executePlay(int tileId);

		// �����ơ����ơ�����֮��֪ͨ���ƻ��߸�(�Ӹܼ�����)
		void afterFetchChiPeng(MahjongAvatar* pAvatar, int fetchedId = -1);

		// ���ƣ�����ʽ�������
		void doHu();

		// ���ִ���
		void noMoreTile();

		// û�г�����
		bool noChiPengGang() const;

	protected:
		// ����
		void dealTiles();

		// ��ȡ��һ�����ڵȴ��Ķ���ѡ��
		bool getFirstWaitingActionOption(MahjongActionOption& ao) const;

		// ��ȡ��������
		void getSettlementData(MahjongSettlement* dt) const;

		// ��ȡ�ط�����
		void getPlaybackData(MahjongPlaybackData& dt) const;

	protected:
		// �Ƿ���Ե���
		virtual bool canDianPao() const;

		// �Ƿ���ǰ����(����)
		virtual bool earlyTermination() const;

		// ����֮���Ƿ���Ҫ�ٴ�����(������Щ�ط��Ĺ���涨�������֮���ٴ����ƶ������ֵ��¼�����)
		virtual bool fetchAgainAfterPlay() const;

		// ֪ͨ����
		virtual void notifyDealTiles();

		// ֪ͨ��ǰ�����߸���
		virtual void notifyActorUpdated(const std::string& playerId);

		// ֪ͨ״̬�仯
		virtual void notifyStateChanged(StateMachine oldState);

		// ֪ͨ�������һ����(���ݾ�����Ϸ��Ŀ�����󣬿������ظú�����֪ͨ�������)
		virtual void notifyFetchTile(MahjongAvatar* pAvatar, bool bBack);

		// ָ֪ͨ����Ҷ���ѡ��
		virtual void notifyActionOptions(MahjongAvatar* pAvatar);

		// ֪ͨ��������ȴ�����״̬
		virtual void notifyWaitingAction(const std::string& playerId);

		// ָ֪ͨ����ҵȴ��������ѡ����ѡ��
		virtual void notifyActionOptionsWaiting(MahjongAvatar* pAvatar);

		// ֪ͨ�����˶���ѡ�����(�ͻ����յ�֪֮ͨ�������ر�ѡ��͵ȴ�����ǰû�ڵȴ�����Һ��Ը�֪ͨ)
		virtual void notifyActionOptionsFinish();

		// ֪ͨ������,��Ҵ��һ����(����������Ϣ)
		virtual void notifyPlayTile(const MahjongTile& mt);

		// ֪ͨ������,��Ҹ���
		virtual void notifyGangTile(MahjongAvatar* pAvatar);

		// ֪ͨ������,�����������
		virtual void notifyPengChiTile(MahjongAvatar* pAvatar, bool bPeng);

		// ָ֪ͨ���������
		virtual void notifyTingTile(MahjongAvatar* pAvatar);

		// ֪ͨ�����ˣ���Һ���
		virtual void notifyHuTile();

		// ��ʾ������ҵ�����
		virtual void notifyShowTiles();

		/**
		 * ��ʾ������������������������
		 * @param avatar �������
		 * @param action 0-������1-����
		 * @param tile �������������
		 */
		virtual void notifyPassTip(MahjongAvatar* avatar, int action, const std::string& tile);

		// ��һ�ֵ�ׯ��
		virtual void bankerNextRound(int huNums, int huPlayer);

		// ������Ʒ�
		virtual void calcHuScore() const = 0;

		// ����
		virtual void doJieSuan() = 0;

		// ����֮��Ĵ������籣��÷ֵ����ݿ⡢����¼���
		virtual void afterHu() = 0;

	protected:
		MahjongRule::Ptr _rule;
		MahjongDealer _dealer;
		MahjongActionList _actions;
		MahjongActorList _actors;

	protected:
		/**
		 * ׯ������(��λ��)
		 */
		int _banker;

		/**
		 * ��ǰ����(���������ᶯ�������)����
		 */
		int _actor;

		/**
		 * �����Ʒ�ʽΪ�����ϻ�ʱ���ñ�����ʾ���ܵ���������������Ʒ�ʽΪ����ʱ��
		 * �ñ�����ʾҪ�Ӹܵ��������
		 */
		int _gangHu;

		/**
		 * ���Ҫʣ��������Ʋ���(���������������)
		 */
		int _tilesLeft;

		/**
		 * �մ������
		 */
		int _playedTileId;

		/**
		 * ������������
		 */
		int _huTileId;

		/**
		 * ��������ȴ�״̬��ʱ�䣬��λ����
		 */
		time_t _waitingTick;

		/**
		 * ����ѡ����
		 */
		MahjongActionOption _acOpPool[ACTION_OPTION_POOL_SIZE];

		/**
		 * ����ѡ��ID������
		 */
		IdentityAllocator _acOpIdAlloc;

		/**
		 * ���ڵȴ��û�ѡ��Ķ���ѡ�ע�����ﳤ��Ϊ4�����鲻��ָ4����ң���������Ϊ�����������ܡ��������������ԡ�!!!
		 */
		std::vector<int> _acOps1[4];

		/**
		 * �û��Ѿ�ѡ��Ķ���ѡ��
		 */
		std::vector<int> _acOps2[4];

	protected:
		// ��ǰ״̬
		StateMachine _state;

		// �Ƿ���Գ���
		bool _chi;

		// �Ƿ���Ե���
		bool _dianPao;

		// �Ƿ����һ�ڶ��죬���������ɽ���Զ��Ȩ��ѡ���������A���ڡ�BΪA�¼ң�CΪA���¼ң�ֻ�е�B����
		// ���������C�ſ��Ժ�(��Щ�ط��Ĺ����ǲ�����һ�ڶ����)
		bool _mutiDianPao;

		// ��һ�ڶ��������£��ǲ���ֻҪ��һ��ѡ����������ܺ��Ķ�������(������֮ǰ���ˡ�����Ҳ���ܺ�)��
		// ����ֵΪfalse������Ҫ������ܺ���ֻҪ��һ��ѡ���˺������滹û��ѡ�������ѡ�����������֮ǰ
		// ѡ���ˡ���������ҽ����ɺ�
		bool _allDianPao;

		// �Ƿ����ڵȴ������������
		bool _waitingQiangGang;

		// �����Ƿ��Ѿ����ˣ������ƾֻ�û����������������
		bool _hu;

		// ���ܵ�����������Ƿ�ɼ�
		bool _anGangVisible;

		// �������Ƿ��㱻���(���������Ҫ��ܷ֣���Ϊ��Щ�ط��Ĺ����Ǹ����ڲ�����ܵķ�)
		bool _gangShangPaoVetoed;

		// �Ƿ�����ӳټӸ�
		bool _delayJiaGang;
	};
}

#endif