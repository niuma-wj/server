// ThreadWorkerDumb.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.07.31

#ifndef _NIU_MA_THREAD_WORKER_DUMB_H_
#define _NIU_MA_THREAD_WORKER_DUMB_H_

#include "ThreadWorker.h"

namespace NiuMa {
	/**
	 * �߳�ѭ�������κ�����Ĺ�����
	 * ÿ���߳�ѭ��ֻ�Ǽ�����ָ��ʱ�䳤��(����)
	 * �����ڽ�ִ���̼߳���ǲ�������������
	 */
	class ThreadWorkerDumb : public ThreadWorker {
	public:
		ThreadWorkerDumb(const ThreadStopFlag::Ptr& flag, int millisecond);
		virtual ~ThreadWorkerDumb() = default;

	protected:
		virtual int oneLoopEx() override;

	private:
		// ÿ��ѭ������ʱ�䳤��(����)��С�ڵ���0������
		int _millisecond;
	};
}

#endif