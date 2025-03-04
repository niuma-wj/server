// PlayerManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.29

#ifndef _NIU_MA_PLAYER_MANAGER_H_
#define _NIU_MA_PLAYER_MANAGER_H_

#include "Base/Singleton.h"
#include "Player.h"
#include <unordered_map>

namespace NiuMa {
	/**
	 * ��ҹ�����
	 */
	class PlayerManager : public Singleton<PlayerManager> {
	private:
		PlayerManager();

	public:
		virtual ~PlayerManager();
		friend class Singleton<PlayerManager>;

	public:
		/**
		 * ��ʼ��
		 */
		void init();

		/**
		 * ��ȡ��ң����������
		 * @param playerId ���id
		 * @return ���
		 */
		Player::Ptr getPlayer(const std::string& playerId) const;

		/**
		 * �����ݿ���������Ϣ
		 * @param playerId ���id
		 * @return ���
		 */
		Player::Ptr loadPlayer(const std::string& playerId);

		/**
		 * ��֤��Ϣ�����ǩ������
		 * @param playerId ���id
		 * @param timestamp unixʱ�������λ�룩
		 * @param nonce �����
		 * @param signature md5ǩ��
		 * @param outdate ������Ϣ�Ƿ��ʱ
		 * @return �Ƿ���֤�ɹ�
		 */
		bool verifySignature(const std::string& playerId, const std::string& timestamp, const std::string& nonce, const std::string& signature, bool& outdate);

		/**
		 * ���ûỰid��Ӧ�����id
		 * @param sessionId �Ựid
		 * @param playerId ���id
		 */
		void setSessionPlayerId(const std::string& sessionId, const std::string& playerId);

		/**
		 * ���ỰId�����id��������
		 * @param sessionId �Ựid
		 */
		void removeSessionId(const std::string& sessionId);

		/**
		 * ͨ���Ựid��ȡ���id
		 * @param sessionId �Ựid
		 * @param playerId ���id
		 * @return true-����ָ���Ựid��false-������
		 */
		bool getPlayerId(const std::string& sessionId, std::string& playerId);

		/**
		 * ͨ���Ựid��ȡ���
		 * @param sessionId �Ựid
		 * @return ���
		 */
		Player::Ptr getPlayerBySessionId(const std::string& sessionId);

		/**
		 * ����������
		 * @param playerId ���id
		 */
		void addOfflinePlayer(const std::string& playerId);

	private:
		/**
		 * �����ݿ�����������
		 * @param player ���
		 * @return �Ƿ���سɹ�
		 */
		bool loadPlayer(const Player::Ptr& player) const;

		/**
		 * ������
		 * @param player ���
		 */
		bool addPlayer(const Player::Ptr& player);

		/**
		 * 5���Ӷ�ʱ������
		 */
		bool onTimer();

		/**
		 * �ͷų�ʱ�䲻�������˻�
		 */
		void freeDormantPlayers();

	private:
		// ��ʱ��������ߣ����ó����߱����٣���˵��������ʵ��������
		// ��ʱ�����ֱ���˳�
		std::shared_ptr<int> _timer;

		// ��ұ�
		std::unordered_map<std::string, Player::Ptr> _players;

		// �Ựid�����idӳ���
		// key-�Ựid, value-���id
		std::unordered_map<std::string, std::string> _sessionMap;

		// �������id�б�
		std::list<std::string> _offlineIds;

		// ����ȷ�ĵ�ʱʱ�䣬ÿ5�����һ�Σ��Լ���Ƶ����ϵͳ��ȡ��ǰʱ��������˷�
		// ��Ϊÿ��������ҵ�ʱ����Ҫ������ҵ���������ʱ�䣬����������Ǹ���Ƶ����
		time_t _inexactTime;

		// �ź���
		mutable std::mutex _mtx;
	};
}

#endif // !_NIU_MA_PLAYER_MANAGER_H_
