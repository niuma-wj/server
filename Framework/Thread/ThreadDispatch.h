// ThreadDispatch.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.29

#ifndef _NIU_MA_THREAD_DISPATCH_H_
#define _NIU_MA_THREAD_DISPATCH_H_

#include <memory>

namespace NiuMa {
	/**
	 * ���߳���ǲ����
	 * �߳�A(��ǲ��)�����߳�B(ִ����)ִ��һ������
	 */
	class ThreadWorker;
	class ThreadDispatch : public std::enable_shared_from_this<ThreadDispatch> {
	public:
		/**
		 * ��ǲ�����캯��
		 * @param dispatcher ��ǲ��
		 */
		ThreadDispatch(const std::shared_ptr<ThreadWorker>& dispatcher = nullptr);
		virtual ~ThreadDispatch() = default;

		typedef std::shared_ptr<ThreadDispatch> Ptr;

	public:
		/**
		 * ִ�����߳�ִ������
		 */
		void execute();

		/**
		 * ����ִ��֮����ǲ������������
		 */
		virtual void onExecuted();

	protected:
		virtual void executeImpl() = 0;

	protected:
		// ��ǲ��
		std::weak_ptr<ThreadWorker> _dispatcher;
	};
}

#endif // !_NIU_MA_THREAD_DISPATCH_H_