// GameDumb.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.10.16

#ifndef _NIU_MA_GAME_DUMB_H_
#define _NIU_MA_GAME_DUMB_H_

#include "Venue/VenueLoader.h"

#include <unordered_set>

namespace NiuMa {
	/**
	 * ����Ϸ�����ڲ��Կ�ܴ���
	 * ����Ϸû���κ����ݣ�ֻ�н��롢�뿪�����������߼�
	 * ����Ϸ���͹̶�Ϊ1��������Ϸ���������������Ϸ��ͻ
	 */
	class GameDumb : public Venue {
	public:
		GameDumb(const std::string& id, const std::string& name, int maxPlayers);
		virtual ~GameDumb();

	public:
		virtual void onDisconnect(const std::string& playerId) override;

	protected:
		virtual bool hasPlayer(const std::string& playerId) override;
		virtual void getPlayerIds(std::vector<std::string>& playerIds) override;
		virtual int getPlayerCount() override;
		virtual bool enterImpl(const std::string& playerId, const std::string& base64, std::string& errMsg) override;
		virtual int leaveImpl(const std::string& playerId, std::string& errMsg) override;

	private:
		// ��Ϸ����
		const std::string _name;

		// �������������
		const int _maxPlayers;

		// ���id����
		std::unordered_set<std::string> _playerIds;
	};

	/**
	 * ����Ϸ������
	 */
	class GameDumbLoader : public VenueLoader {
	public:
		GameDumbLoader();
		virtual ~GameDumbLoader();

	public:
		virtual Venue::Ptr load(const std::string& id) override;
	};
}

#endif // !_NIU_MA_GAME_DUMB_H_