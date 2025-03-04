// Optional.h
// Author: wujian
// Email: 393817707@qq.com
// Date: 2024.08.06

#ifndef _NIU_MA_OPTIONAL_H_
#define _NIU_MA_OPTIONAL_H_

#include <ostream>

namespace NiuMa {
	// 可空类
    template <typename T>
    class Optional {
    public:
        Optional()
            : _hasValue(false)
        {}

        Optional(const T& value)
            : _hasValue(true)
            , _value(value)
        {}

        Optional(const Optional<T>& other) {
            _hasValue = other.hasValue();
            if (_hasValue)
                _value = other.value();
        }

        operator T() const {
            if (!_hasValue)
                throw std::runtime_error("Has no value!");
            return _value;
        }

        Optional<T>& operator = (const T& value) {
            _hasValue = true;
            _value = value;
            return *this;
        }

        Optional<T>& operator = (const Optional<T>& other) {
            _hasValue = other.hasValue();
            if (_hasValue)
                _value = other.value();
            return *this;
        }

        bool hasValue() const { return _hasValue; }

        const T& value() const {
            if (!_hasValue)
                throw std::runtime_error("Has no value!");
            return _value;
        }

        Optional<T>& unset() {
            _hasValue = false;
            return *this;
        }

    private:
        // 是否有数值
        bool _hasValue;

        // 数值
        T _value;
    };

    template <typename T>
    std::ostream& operator << (std::ostream& os, const Optional<T>& val) {
        return os << val.value();
    }
}

#endif