/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>
#include <array>
#include <algorithm>
#include <span>

#include "../utility/digest_base.hpp"

namespace dataforge::belt_detail {

struct belt_hash_impl : digest_base<belt_hash_impl, 32, uint_least64_t, 2>
{
    belt_hash_impl();

    template <typename InitQuarkT>
    explicit belt_hash_impl(InitQuarkT const&);

    void reset() noexcept;

    void process_block(const void* msg);
    void finalize();

    using digest_word_type = uint_least64_t;
    static constexpr size_t digest_word_bit_count = 64;
    static constexpr std::endian digest_endianness() { return std::endian::little; }

    static constexpr size_t state_size = 4;
    inline std::span<digest_word_type, state_size> digest_span() { return h; }

    inline constexpr size_t digest_length() const noexcept { return 32; }

protected:
    void sigma2(const uint_least64_t* x, uint_least64_t* result);

protected:
    std::array<uint_least64_t, state_size> h;
    std::array<uint_least64_t, 2> s;
};

}

#include "belt.ipp"
