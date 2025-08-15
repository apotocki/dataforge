/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

#include "../utility/digest_base.hpp"

namespace dataforge::whirlpool_detail {

struct whirlpool_impl : digest_base<whirlpool_impl, 64, uint_least64_t, 4>
{
    static constexpr size_t digest_length() { return 64; }
    static constexpr std::endian digest_endianness() { return std::endian::big; }
    static constexpr size_t input_length_size = 32;

    using word_type = uint_least64_t;
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = 64;

    whirlpool_impl();

    void reset();

    void process_block(const void* msg);

    void store_bit_count(void* dst) const noexcept;

    static constexpr size_t state_size = 8;
    inline std::span<digest_word_type, state_size> digest_span() { return hash_state; }

private:
    word_type hash_state[state_size];
};

}

#include "whirlpool.ipp"
