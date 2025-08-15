/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

#include "../utility/digest_base.hpp"

namespace dataforge {

enum class sha1_type : int { sha1 };

namespace sha1_detail {

struct sha1_impl : digest_base<sha1_impl, 64>
{
    static constexpr size_t digest_length() { return 20; }
    static constexpr std::endian digest_endianness() { return std::endian::big; }
    static constexpr int input_length_size = 8;

    using word_type = uint_least32_t;
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = 32;

    sha1_impl();

    void process_block(const void* msg);
    void store_bit_count(void* dst) const;
    void reset();

    static constexpr size_t state_size = 5;
    inline std::span<digest_word_type, state_size> digest_span() { return H; }

private:
    static inline word_type Ch(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (~x & z);
    }
    static inline word_type Parity(word_type x, word_type y, word_type z)
    {
        return x ^ y ^ z;
    }
    static inline uint32_t Maj(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    digest_word_type H[state_size];
};

}}

#include "sha1.ipp"
