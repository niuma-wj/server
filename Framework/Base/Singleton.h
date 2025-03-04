// Singleton.h
// Author wujian
// Email 393817707@qq.com
// Date 2024.07.10

#ifndef _NIU_MA_SINGLETON_H_
#define _NIU_MA_SINGLETON_H_

namespace NiuMa {
	/**
	 * 单例模式基类
	 */
	template <typename T>
	class Singleton {
	private:
		Singleton(const Singleton<T>&) = delete;
		Singleton& operator=(const Singleton<T>&) = delete;

	protected:
		Singleton() {}

	private:
		static T* _inst;

	public:
		virtual ~Singleton(void) {}

		static T& getSingleton(void)
		{
			if (_inst == nullptr)
				_inst = new T();
			return *_inst;
		}

		static void deinstantiate() {
			if (_inst != nullptr) {
				delete _inst;
				_inst = nullptr;
			}
		}
	};
}

#endif // !_NIU_MA_SINGLETON_H_


