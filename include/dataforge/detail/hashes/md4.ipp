/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <cstring>

namespace dataforge::md4_detail {

inline const uint_least32_t init_values[4] =
{
    0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

inline md4_impl::md4_impl()
{
    std::memcpy(state, init_values, sizeof(state));
}

inline void md4_impl::reset()
{
    std::memcpy(state, init_values, sizeof(state));
    bit_count = 0;
}

// processes one chunk of 16 bytes 
inline void md4_impl::process_block(const void* msg)
{
    word_type xs[16];
    const word_type * x = le_to_T<8, 32, const word_type>(xs, msg, block_size);

    word_type a = state[0];
    word_type b = state[1];
    word_type c = state[2];
    word_type d = state[3];

    // round 1
    transform<aux_f>(a, b, c, d, x[0], 3, 0);
    transform<aux_f>(d, a, b, c, x[1], 7, 0);
    transform<aux_f>(c, d, a, b, x[2], 11, 0);
    transform<aux_f>(b, c, d, a, x[3], 19, 0);
    transform<aux_f>(a, b, c, d, x[4], 3, 0);
    transform<aux_f>(d, a, b, c, x[5], 7, 0);
    transform<aux_f>(c, d, a, b, x[6], 11, 0);
    transform<aux_f>(b, c, d, a, x[7], 19, 0);
    transform<aux_f>(a, b, c, d, x[8], 3, 0);
    transform<aux_f>(d, a, b, c, x[9], 7, 0);
    transform<aux_f>(c, d, a, b, x[10], 11, 0);
    transform<aux_f>(b, c, d, a, x[11], 19, 0);
    transform<aux_f>(a, b, c, d, x[12], 3, 0);
    transform<aux_f>(d, a, b, c, x[13], 7, 0);
    transform<aux_f>(c, d, a, b, x[14], 11, 0);
    transform<aux_f>(b, c, d, a, x[15], 19, 0);

    // round 2
    transform<aux_g>(a, b, c, d, x[0], 3, 0x5a827999);
    transform<aux_g>(d, a, b, c, x[4], 5, 0x5a827999);
    transform<aux_g>(c, d, a, b, x[8], 9, 0x5a827999);
    transform<aux_g>(b, c, d, a, x[12], 13, 0x5a827999);
    transform<aux_g>(a, b, c, d, x[1], 3, 0x5a827999);
    transform<aux_g>(d, a, b, c, x[5], 5, 0x5a827999);
    transform<aux_g>(c, d, a, b, x[9], 9, 0x5a827999);
    transform<aux_g>(b, c, d, a, x[13], 13, 0x5a827999);
    transform<aux_g>(a, b, c, d, x[2], 3, 0x5a827999);
    transform<aux_g>(d, a, b, c, x[6], 5, 0x5a827999);
    transform<aux_g>(c, d, a, b, x[10], 9, 0x5a827999);
    transform<aux_g>(b, c, d, a, x[14], 13, 0x5a827999);
    transform<aux_g>(a, b, c, d, x[3], 3, 0x5a827999);
    transform<aux_g>(d, a, b, c, x[7], 5, 0x5a827999);
    transform<aux_g>(c, d, a, b, x[11], 9, 0x5a827999);
    transform<aux_g>(b, c, d, a, x[15], 13, 0x5a827999);

    // round 3
    transform<aux_h>(a, b, c, d, x[0], 3, 0x6ed9eba1);
    transform<aux_h>(d, a, b, c, x[8], 9, 0x6ed9eba1);
    transform<aux_h>(c, d, a, b, x[4], 11, 0x6ed9eba1);
    transform<aux_h>(b, c, d, a, x[12], 15, 0x6ed9eba1);
    transform<aux_h>(a, b, c, d, x[2], 3, 0x6ed9eba1);
    transform<aux_h>(d, a, b, c, x[10], 9, 0x6ed9eba1);
    transform<aux_h>(c, d, a, b, x[6], 11, 0x6ed9eba1);
    transform<aux_h>(b, c, d, a, x[14], 15, 0x6ed9eba1);
    transform<aux_h>(a, b, c, d, x[1], 3, 0x6ed9eba1);
    transform<aux_h>(d, a, b, c, x[9], 9, 0x6ed9eba1);
    transform<aux_h>(c, d, a, b, x[5], 11, 0x6ed9eba1);
    transform<aux_h>(b, c, d, a, x[13], 15, 0x6ed9eba1);
    transform<aux_h>(a, b, c, d, x[3], 3, 0x6ed9eba1);
    transform<aux_h>(d, a, b, c, x[11], 9, 0x6ed9eba1);
    transform<aux_h>(c, d, a, b, x[7], 11, 0x6ed9eba1);
    transform<aux_h>(b, c, d, a, x[15], 15, 0x6ed9eba1);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

}
