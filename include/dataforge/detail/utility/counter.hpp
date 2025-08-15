/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <array>

#include "data_ops.hpp"

namespace dataforge {

template <typename T, int N>
requires (std::is_unsigned_v<T> && std::is_integral_v<T>)
struct counter
{
    using value_type = T;
    static constexpr int elements = N;

    counter() : data_{}
    {}
    
    explicit inline counter(T x) noexcept
    {
        data_[0] = x;
    }

    template <typename SomeIntegerT>
    requires(std::is_unsigned_v<SomeIntegerT> && std::is_integral_v<SomeIntegerT> && sizeof(SomeIntegerT) <= sizeof(T))
    void sub(SomeIntegerT x)
    {
        if (data_[0] >= x) {
            data_[0] -= x;
            return;
        }
        data_[0] = (std::numeric_limits<T>::max)() - x + data_[0] + 1;

        for (int i = 1; i < N; ++i)
        {
            if (data_[i]) {
                --data_[i];
                return;
            }
            data_[i] = (std::numeric_limits<T>::max)();
        }
        throw std::overflow_error("counter overflow");
    }

    template <typename SomeIntegerT>
    requires(std::is_unsigned_v<SomeIntegerT>&& std::is_integral_v<SomeIntegerT> && sizeof(SomeIntegerT) <= sizeof(T))
    void add(SomeIntegerT x)
    {
        T tmp = (std::numeric_limits<T>::max)() - data_[0];
        if (tmp >= x) {
            data_[0] += x;
            return;
        }
        data_[0] = x - tmp - 1;
        
        for (int i = 1; i < N; ++i)
        {
            if ((std::numeric_limits<T>::max)() != data_[i]) {
                ++data_[i];
                return;
            }
            data_[i] = 0;
        }
        throw std::overflow_error("counter overflow");
        
    }

    template <typename SomeIntegerT>
    requires(std::is_unsigned_v<SomeIntegerT>&& std::is_integral_v<SomeIntegerT> && sizeof(SomeIntegerT) > sizeof(T))
    void add(SomeIntegerT x)
    {
        static constexpr SomeIntegerT mask = (((SomeIntegerT)1) << (sizeof(T) * 8)) - 1;
        T tmp = (std::numeric_limits<T>::max)() - data_[0];
        if (tmp >= x) {
            data_[0] += (T)x;
            return;
        }
        data_[0] = static_cast<T>(x & mask) - tmp - 1;

        T carry = 0;

        for (int i = 1; i < N; ++i)
        {
            x >>= (sizeof(T) * 8);
            T tmp = data_[i];
            data_[i] += 1 + static_cast<T>(x & mask);
            if (data_[i] > tmp) return;
        }
        throw std::overflow_error("counter overflow");
    }

    inline counter& operator=(T x) noexcept
    {
        data_[0] = x;
        std::fill(data_.begin() + 1, data_.end(), 0);
        return *this;
    }

    T& operator [] (unsigned int n) noexcept { return data_[n]; }
    const T& operator [] (unsigned int n) const noexcept { return data_[n]; }

    T* data() noexcept { return data_.data(); }
    const T* data() const noexcept { return data_.data(); }

    void store_as_big_endian(void* vdst, int cnt = N) const
    {
        char* dst = static_cast<char*>(vdst);
        dst += sizeof(T) * cnt;
        for (int i = 0; i < cnt; ++i)
        {
            dst -= sizeof(T);
            T_to_be(dst, &data_[i], 1);
        }
    }

    void store_as_little_endian(void* vdst, int cnt = N) const
    {
        char* dst = static_cast<char*>(vdst);
        for (int i = 0; i < cnt; ++i)
        {
            le_copy<sizeof(T) * CHAR_BIT, 8>(&data_[i], 1, dst);
            dst += sizeof(T);
        }
    }

private:
    std::array<T, N> data_;
};

}
