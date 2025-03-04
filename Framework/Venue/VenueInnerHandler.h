// VenueInnerHandler.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.25

#ifndef _NIU_MA_VENUE_INNER_HANDLER_H_
#define _NIU_MA_VENUE_INNER_HANDLER_H_

#include "Player/PlayerSignatureHandler.h"
#include "Venue.h"

#include <unordered_map>
#include <vector>

namespace NiuMa {
	/**
	 * �����ڲ���Ϣ������(��Ϸ�߼�)
	 * ������Ϣ������ͨ���ڲ�������ռ����Ϣ���У�ͬһ�������ڵ���Ϣֻ�̶ܹ�ͬһ����������������
	 * ���ĺô��ǳ����ڲ���ҵ���߼�����Ҫ���߳�ͬ�����Ӷ����ϵͳ���岢������
	 * ע��������Ϣ��������ҵ���߼�������Ҫ�г�ʱ�������������������������ݿ��ѯ������IO���������㣬
	 * ��Щ�����������첽��������ɣ������������Ϸ�߼���Ӧ����ʱ
	 */
	class VenueInnerHandler : public PlayerSignatureHandler {
	public:
		VenueInnerHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~VenueInnerHandler();

	public:
		/**
		 * ����Ƿ�֧��ָ������Ϸ����
		 * @param gameType ��Ϸ����
		 * @return true-֧�֣�false-��֧��
		 */
		bool checkGameType(int gameType) const;

		/**
		 * ��ӳ���
		 * @param venue ����
		 */
		void addVenue(const Venue::Ptr& venue);

		/**
		 * ���ҳ���
		 * @param id ����id
		 * @return ����
		 */
		Venue::Ptr getVenue(const std::string& id) const;

		/**
		 * ���ó����������
		 * @param id ����id
		 * @param count �������
		 */
		void setPlayerCount(const std::string& id, int count);

		/**
		 * ���ص�ǰ��������� 
		 */
		int getTotalCount() const;

	public:
		virtual void initialize() override;
		virtual bool receive(const NetMessage::Ptr& netMsg) const override;

	protected:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		/**
		 * 30���붨ʱ����
		 * @return �Ƿ������ʱ����true-�ǣ�false-��
		 */
		virtual bool onTimer();

		/**
		 * ���֧�ֵ���Ϸ����(�����ʼ��ʱ����)
		 * @param gameType ��Ϸ����
		 */
		void addGameType(int gameType);

		/**
		 * �ж��Ƿ���ڳ���
		 * @param id ����id
		 * @return true-���ڣ�false-������
		 */
		bool hasVenue(const std::string& id) const;

		/**
		 * ��ȡ�ڲ��߳�ר�õĳ����б�
		 */
		std::vector<Venue::Ptr>& getVenueList();

	private:
		/**
		 * �����ڲ��߳�ר�õĳ����б�
		 */
		void updateVenueList();

		/**
		 * ɾ������
		 * @param id ����id
		 */
		void removeVenue(const std::string& id);

	private:
		// ֧�ֵ���Ϸ����(��ʼ��ʱ����)
		std::unordered_set<int> _gameTypes;

		// �����б�
		// key-����id��value-����
		std::unordered_map<std::string, Venue::Ptr> _venues;

		// �����б��ڲ��߳�ר�ã�����������ؼ����߳�����ʹ��
		std::vector<Venue::Ptr> _venueList;

		// �������������
		// key-����id��value-�������
		std::unordered_map<std::string, int> _playerCounts;

		// ��ǰ���������
		int _totalCount;

		// ���ؼ���������������ͬ��
		int _counterMap;

		// ���ؼ���������������ͬ��
		int _counterList;

		// �ź���
		mutable std::mutex _mtx;
	};
}

#endif // !_NIU_MA_VENUE_INNER_HANDLER_H_