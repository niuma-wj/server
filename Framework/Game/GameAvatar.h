// GameAvatar.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.11.22

#ifndef _NIU_MA_GAME_AVATAR_H_
#define _NIU_MA_GAME_AVATAR_H_

#include "Network/Session.h"

namespace NiuMa
{
	/**
	 * ����������
	 * ����һ��������������������Ϸ�е�״̬���ݣ������е��������ݺͷ���������ͬһ
	 * �߳��ڷ��ʵģ�����Ҫ�����߳�ͬ��������
	 */
	class GameAvatar {
	public:
		GameAvatar(const std::string& playerId, bool robot = false);
		virtual ~GameAvatar();

		typedef std::shared_ptr<GameAvatar> Ptr;

	public:
		const std::string& getPlayerId() const;
		const std::string& getNickname() const;
		void setNickname(const std::string& s);
		const std::string& getPhone() const;
		void setPhone(const std::string& s);
		int getSex() const;
		void setSex(int s);
		const std::string& getHeadUrl() const;
		void setHeadUrl(const std::string& s);
		bool isRobot() const;
		int getSeat() const;
		void setSeat(int s);
		int64_t getGold();
		void setGold(int64_t gold);
		int64_t getCashPledge() const;
		void setCashPledge(int64_t s);
		bool isAuthorize() const;
		void setAuthorize(bool s);
		bool isReady() const;
		void setReady(bool s);
		bool isOffline() const;
		void setOffline(bool s);
		void setSession(const Session::Ptr& session);
		Session::Ptr getSession();
		// ��þ�γ��
		void getGeolocation(double& lat, double& lon, double& alt) const;

		// ���þ�γ��
		void setGeolocation(double lat, double lon, double alt);

	private:
		// ���id
		const std::string _playerId;

		// ����ǳ�
		std::string _nickname;

		// ��ϵ�绰
		std::string _phone;

		// �Ա�
		int _sex;

		// ͷ��url
		std::string _headUrl;

		// �Ƿ�Ϊ������
		const bool _robot;

		// �������Ϸ���ϵ���λ������-1��ʾ����λ
		int _seat;

		// ���������ע�⣬����ֵ�����ڷ����ѯ�������Ǿ���׼ȷ�ģ���������ڴ����������˽�ң�
		// ������������ӳ����ֵ�С�����Ҫ������Ϊ����ֵ��׼ȷ�Զ�Ӱ�쵽��Ϸ�߼�����Ϊ��Ϸ�߼�
		// �в��������ڸ���ֵ���������ݿ��е���ҽ��������
		int64_t _gold;

		// ��ǰѺ����
		int64_t _cashPledge;

		// �Ƿ��йܣ���ν�йܼ���ϵͳ�Զ�Ϊ���ִ����Ϸ����
		bool _authorize;

		// �Ƿ���׼������
		bool _ready;

		// �Ƿ�����
		bool _offline;

		// ����Ự
		std::weak_ptr<Session> _session;

		// γ��
		double _latitude;

		// ����
		double _longitude;

		// ����
		double _altitude;
	};
}

#endif // !_NIU_MA_GAME_AVATAR_H_