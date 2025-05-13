// ThreadDispatch.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.29

#ifndef _NIU_MA_THREAD_DISPATCH_H_
#define _NIU_MA_THREAD_DISPATCH_H_

#include <memory>

namespace NiuMa {
	/**
	 * 跨线程派遣任务
	 * 线程A(派遣者)请求线程B(执行者)执行一个任务
	 */
	class ThreadWorker;
	class ThreadDispatch : public std::enable_shared_from_this<ThreadDispatch> {
	public:
		/**
		 * 派遣任务构造函数
		 * @param dispatcher 派遣者
		 */
		ThreadDispatch(const std::shared_ptr<ThreadWorker>& dispatcher = nullptr);
		virtual ~ThreadDispatch() = default;

		typedef std::shared_ptr<ThreadDispatch> Ptr;

	public:
		/**
		 * 执行者线程执行任务
		 */
		void execute();

		/**
		 * 任务被执行之后，派遣者做后续处理
		 */
		virtual void onExecuted();

	protected:
		virtual void executeImpl() = 0;

	protected:
		// 派遣者
		std::weak_ptr<ThreadWorker> _dispatcher;
	};
}

#endif // !_NIU_MA_THREAD_DISPATCH_H_