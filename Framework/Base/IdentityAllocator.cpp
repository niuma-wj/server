// IdentityAllocator.cpp

#include "IdentityAllocator.h"

#include <list>
#include <mutex>

namespace NiuMa
{
	class IdentityAllocatorImpl
	{
	public:
		IdentityAllocatorImpl(int start)
			: _start(start)
			, _next(start)
		{}

		virtual ~IdentityAllocatorImpl() {}

	private:
		/**
		 * 起始id
		 */
		const int _start;

		/**
		 * 下一个id 
		 */
		int _next;

		/**
		 * 当前已回收id 
		 */
		std::list<int> _recycledIds;

		/**
		 * 信号量
		 */
		mutable std::mutex _mutex;

	public:
		int askForID() {
			std::lock_guard<std::mutex> lck(_mutex);

			int id = IdentityAllocator::INVALID_ID;
			if (_recycledIds.empty()) {
				id = _next;
				_next++;
			} else {
				id = _recycledIds.front();
				_recycledIds.pop_front();
			}
			return id;
		}

		void recycleID(int id) {
			std::lock_guard<std::mutex> lck(_mutex);

			_recycledIds.push_back(id);
		}

		void recycleAll() {
			std::lock_guard<std::mutex> lck(_mutex);

			_recycledIds.clear();
			_next = _start;
		}
	};

	const int IdentityAllocator::INVALID_ID = -1;

	IdentityAllocator::IdentityAllocator(int start) {
		_pImpl = std::make_shared<IdentityAllocatorImpl>(start);
	}

	IdentityAllocator::~IdentityAllocator() {}

	int IdentityAllocator::askForId() {
		return _pImpl->askForID();
	}

	void IdentityAllocator::recycleId(int id) {
		_pImpl->recycleID(id);
	}

	void IdentityAllocator::recycleAll() {
		_pImpl->recycleAll();
	}
}