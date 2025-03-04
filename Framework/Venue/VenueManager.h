// VenueManager.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.09.25

#ifndef _NIU_MA_VENUE_MANAGER_H_
#define _NIU_MA_VENUE_MANAGER_H_

#include "Base/Singleton.h"
#include "VenueLoader.h"
#include "VenueInnerHandler.h"

namespace NiuMa {
	/**
	 * 场地管理者
	 * 管理和分配场地内消息处理器
	 */
	class VenueManager : public Singleton<VenueManager> {
	private:
		VenueManager();

	public:
		virtual ~VenueManager();

		friend class Singleton<VenueManager>;
		friend class VenueInnerHandler;
		friend class VenueLoader;

	public:
		/**
		 * 初始化
		 * @param serverId 服务器id
		 * @param directExchange RabbitMQ定向消息交换机名称
		 * @param consumerTag RabbitMQ消费者标签
		 */
		void init(const std::string& serverId, const std::string& directExchange, const std::string& consumerTag);

		/**
		 * 注册场地加载器
		 * @param loader 场地加载器
		 */
		void registLoader(const VenueLoader::Ptr& loader);

		/**
		 * 注册场地内消息处理器
		 * @param handler 消息处理器
		 */
		void registHandler(const std::shared_ptr<VenueInnerHandler>& handler);

		/**
		 * 加载场地
		 * @param id 场地id
		 * @param gameType 游戏类型
		 * @param error 返回错误码(2-游戏类型错误，3-场地加载失败，4-场地状态错误)
		 * @return 场地
		 */
		Venue::Ptr loadVenue(const std::string& id, int gameType, int& error);

		/**
		 * 获取场地
		 * @param id 场地id
		 */
		Venue::Ptr getVenue(const std::string& id);

		/**
		 * 删除场地
		 * @param id 场地id
		 */
		void removeVenue(const std::string& id);

	private:
		/**
		 * 判定场地是否已加载
		 * @param id 场地id
		 */
		bool hasVenue(const std::string& id);

		/**
		 * 添加场地
		 * @param venue 场地
		 */
		void addVenue(const Venue::Ptr& venue);

		/**
		 * 获取场地加载器
		 * @param gameType 游戏类型
		 */
		VenueLoader::Ptr getVenueLoader(int gameType);

		/**
		 * 锁定要加载的场地id加锁，以免重复加载
		 * @param id 场地id
		 */
		bool lockLoading(const std::string& id);

		/**
		 * 对正在加载的场地id解锁
		 * @param id 场地id
		 */
		void unlockLoading(const std::string& id);

		/**
		 * 为场地分配消息处理器
		 * @param venue 场地
		 * @return 消息处理器
		 */
		std::shared_ptr<VenueInnerHandler> distributeHandler(const Venue::Ptr& venue);

		/**
		 * 处理RabbitMQ消息
		 * @param msgType 消息类型
		 * @param json 消息体
		 */
		void handleMessage(const std::string& msgType, const std::string& json);

		/**
		 * 计算总玩家人数
		 * @return 总玩家人数
		 */
		int getTotalCount();

		/**
		 * 定时任务
		 * 每2秒向Redis更新服务器总玩家数量，用于系统的负债均衡
		 */
		bool onTimer();

	private:
		// 服务器id
		std::string _serverId;

		// RabbitMQ定向消息交换机
		std::string _directExchange;

		// 定时任务持有者，当该持有者被销毁，则说明本单例实例被销毁
		// 定时任务可直接退出
		std::shared_ptr<int> _timer;

		// 场地加载器表
		// key-游戏类型，value-加载器
		std::unordered_map<int, VenueLoader::Ptr> _venueLoaders;

		// 已经加载的场地列表
		std::unordered_map<std::string, Venue::Ptr> _venues;

		// 当前正在加载的场地列表
		// 以免重复加载
		std::unordered_set<std::string> _loadingVenues;

		// 场地内消息处理器表
		std::vector<std::shared_ptr<VenueInnerHandler> > _innerHandlers;

		// 场地与场地内消息处理器映射表
		// key-场地id，value-场地内消息处理器
		// 在将场地分配到VenueInnerHandler时填入该表，场地关闭时从该表中删除
		std::unordered_map<std::string, std::shared_ptr<VenueInnerHandler> > _venueHandlerMap;

		// 信号量
		std::mutex _mtx;
	};
}

#endif // !_NIU_MA_VENUE_MANAGER_H_