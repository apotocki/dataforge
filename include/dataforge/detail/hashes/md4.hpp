/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

#include "../utility/digest_base.hpp"

namespace dataforge::md4_detail {

struct md4_impl : digest_base<md4_impl, 64>
{
    static constexpr int digest_length() { return 16; }
    static constexpr std::endian digest_endianness() { return std::endian::little; }
    static constexpr int input_length_size = 8;

    using element_type = uint_least8_t;
    using word_type = uint_least32_t;
    
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = 32;

    md4_impl();

    void reset();

    void process_block(const void* msg);

    static constexpr size_t state_size = 4;
    inline std::span<digest_word_type, state_size> digest_span() { return state; }

    // auxiliary functions
    struct aux_f
    {
        word_type operator()(word_type x, word_type y, word_type z) const
        {
            return (x & y) | (~x & z);
        }
    };
    struct aux_g
    {
        word_type operator()(word_type x, word_type y, word_type z) const
        {
            return (x & y) | (x & z) | (y & z);
        }
    };
    struct aux_h
    {
        word_type operator()(word_type x, word_type y, word_type z) const
        {
            return x ^ y ^ z;
        }
    };

    template<typename AuxFunctor>
    static void transform(word_type& a, word_type& b, word_type& c, word_type& d,
        word_type k, word_type s, word_type i)
    {
        a = left_rotate<32>(a + AuxFunctor()(b, c, d) + k + i, s);
    }

    digest_word_type state[state_size];
};

}

#include "md4.ipp"
