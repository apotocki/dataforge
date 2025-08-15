/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

#include "../utility/digest_base.hpp"

namespace dataforge::md2_detail {

struct md2_impl : digest_base<md2_impl, 16>
{
    static constexpr int digest_length() { return 16; }
    static constexpr std::endian digest_endianness() { return std::endian::native; }

    using element_type = uint_least8_t;
    using word_type = uint_least32_t;

    using digest_word_type = uint_least8_t;
    static constexpr size_t digest_word_bit_count = 8;

    md2_impl();

    void reset();

    void process_block(const void* msg);
    void finalize();

    static constexpr size_t state_size = 16;
    inline std::span<digest_word_type, state_size> digest_span() { return state; }

    digest_word_type state[state_size];
    uint_least8_t checksum[16];
    uint_least8_t buffer[16];
};

}

#include "md2.ipp"
