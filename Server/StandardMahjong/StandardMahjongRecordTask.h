// StandardMahjongRecordTask.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.06

#include "MySql/MysqlQueryTask.h"

namespace NiuMa
{
	/**
	 * 麻将一局记录保存任务
	 */
	class StandardMahjongRecordTask : public MysqlQueryTask
	{
	public:
		StandardMahjongRecordTask();
		virtual ~StandardMahjongRecordTask();

	public:
		virtual QueryType buildQuery(std::string& sql) override;

	public:
		// 局号数
		int _roundNo;

		// 庄家座位号
		int _banker;

		// 全部玩家得分
		int _scores[4];

		// 全部玩家赢的金币数量
		int _winGolds[4];

		// 场地id
		std::string _venueId;

		// 全部玩家id
		std::string _playerIds[4];

		// 回放数据，Json字符串用zlib压缩再打包成base64
		std::string _playback;
	};
}