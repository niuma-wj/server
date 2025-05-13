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
	 * 场地内部消息处理器(游戏逻辑)
	 * 这类消息处理器通常内部创建独占的消息队列，同一个场地内的消息只能固定同一个处理器处理，这样
	 * 做的好处是场地内部的业务逻辑不需要做线程同步，从而提高系统整体并发性能
	 * 注意这类消息处理器的业务逻辑尽量不要有长时间的阻塞操作（例如较慢的数据库查询、网络IO、复杂运算，
	 * 这些操作尽量用异步请求来完成），以免造成游戏逻辑响应不及时
	 */
	class VenueInnerHandler : public PlayerSignatureHandler {
	public:
		VenueInnerHandler(const MessageQueue::Ptr& queue = nullptr);
		virtual ~VenueInnerHandler();

	public:
		/**
		 * 检查是否支持指定的游戏类型
		 * @param gameType 游戏类型
		 * @return true-支持，false-不支持
		 */
		bool checkGameType(int gameType) const;

		/**
		 * 添加场地
		 * @param venue 场地
		 */
		void addVenue(const Venue::Ptr& venue);

		/**
		 * 查找场地
		 * @param id 场地id
		 * @return 场地
		 */
		Venue::Ptr getVenue(const std::string& id) const;

		/**
		 * 设置场地玩家数量
		 * @param id 场地id
		 * @param count 玩家数量
		 */
		void setPlayerCount(const std::string& id, int count);

		/**
		 * 返回当前总玩家人数 
		 */
		int getTotalCount() const;

	public:
		virtual void initialize() override;
		virtual bool receive(const NetMessage::Ptr& netMsg) const override;

	protected:
		virtual bool onMessage(const NetMessage::Ptr& netMsg) override;

	protected:
		/**
		 * 30毫秒定时任务
		 * @return 是否结束定时任务，true-是，false-否
		 */
		virtual bool onTimer();

		/**
		 * 添加支持的游戏类型(子类初始化时调用)
		 * @param gameType 游戏类型
		 */
		void addGameType(int gameType);

		/**
		 * 判断是否存在场地
		 * @param id 场地id
		 * @return true-存在，false-不存在
		 */
		bool hasVenue(const std::string& id) const;

		/**
		 * 获取内部线程专用的场地列表
		 */
		std::vector<Venue::Ptr>& getVenueList();

	private:
		/**
		 * 更新内部线程专用的场地列表
		 */
		void updateVenueList();

		/**
		 * 删除场地
		 * @param id 场地id
		 */
		void removeVenue(const std::string& id);

	private:
		// 支持的游戏类型(初始化时配置)
		std::unordered_set<int> _gameTypes;

		// 场地列表
		// key-场地id，value-场地
		std::unordered_map<std::string, Venue::Ptr> _venues;

		// 场地列表，内部线程专用，方便遍历场地减少线程锁的使用
		std::vector<Venue::Ptr> _venueList;

		// 场地玩家数量表
		// key-场地id，value-玩家数量
		std::unordered_map<std::string, int> _playerCounts;

		// 当前总玩家人数
		int _totalCount;

		// 场地计数器，用于数据同步
		int _counterMap;

		// 场地计数器，用于数据同步
		int _counterList;

		// 信号量
		mutable std::mutex _mtx;
	};
}

#endif // !_NIU_MA_VENUE_INNER_HANDLER_H_