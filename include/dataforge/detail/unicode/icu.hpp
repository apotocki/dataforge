/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once
#include <concepts>
#include <cstring>
#include <array>

namespace dataforge::icu_detail {

template <std::integral T, size_t ResultBuffSzV, bool isBuffStatic>
struct result_buffer
{
    std::array<T, ResultBuffSzV> buff_;
    T* buffpos_;

    result_buffer()
    {
        buffpos_ = buff_.data();
    }

    result_buffer(result_buffer const&) = delete;
    result_buffer& operator=(result_buffer const&) = delete;

    inline result_buffer(result_buffer&& rhs) noexcept
    {
        this->operator= (std::move(rhs));
    }

    inline result_buffer& operator=(result_buffer&& rhs) noexcept
    {
        size_t rhssz = static_cast<size_t>(rhs.buffpos_ - rhs.buff_.data());
        std::memcpy(buff_.data(), rhs.buff_.data(), rhssz);
        buffpos_ = buff_.data() + rhssz;
        return *this;
    }

    inline T* buffer() noexcept { return buff_.data(); };
    size_t available_size() const { return buff_.data() + ResultBuffSzV - buffpos_; }
};

template <std::integral T, size_t ResultBuffSzV>
struct result_buffer<T, ResultBuffSzV, false>
{
    T* buffer_;
    T* buffpos_;

    result_buffer()
    {
        buffpos_ = buffer_ = new T[ResultBuffSzV];
    }

    result_buffer(result_buffer const&) = delete;
    inline result_buffer(result_buffer&& rhs) noexcept : buffer_{ rhs.buffer_ }, buffpos_{ rhs.buffpos_ } { rhs.buffer_ = nullptr; }
    result_buffer& operator=(result_buffer const&) = delete;
    inline result_buffer& operator=(result_buffer&& rhs) noexcept { buffer_ = rhs.buffer_; buffpos_ = rhs.buffpos_; rhs.buffer_ = nullptr; }

    ~result_buffer() noexcept
    {
        delete[] buffer_;
    }

    inline T* buffer() noexcept { return buffer_; };
    size_t available_size() const { return buffer_ + ResultBuffSzV - buffpos_; }
};

}
