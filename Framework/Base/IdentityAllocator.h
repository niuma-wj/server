// IdentityAllocator.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.12.02

#ifndef _NIU_MA_IDENTITY_ALLOCATOR_H_
#define _NIU_MA_IDENTITY_ALLOCATOR_H_

#include <memory>

namespace NiuMa
{
	/**
	 * ID分配器
	 */
	class IdentityAllocatorImpl;
	class IdentityAllocator
	{
	public:
		IdentityAllocator(int start = 0);
		virtual ~IdentityAllocator();

		static const int INVALID_ID;

	private:
		std::shared_ptr<IdentityAllocatorImpl> _pImpl;

	public:
		// 请求一个ID
		int askForId();

		// 回收一个ID
		void recycleId(int id);

		// 全部回收
		void recycleAll();
	};
}

#endif // !_NIU_MA_IDENTITY_ALLOCATOR_H_