/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <cstring>

namespace dataforge::sha1_detail {

inline const uint_least32_t init_values[5] =
{
    0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
};

inline const uint_least32_t K[4] = {
    0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6
};

inline sha1_impl::sha1_impl()
{
    std::memcpy(H, init_values, sizeof(H));
}

inline void sha1_impl::reset()
{
    std::memcpy(H, init_values, sizeof(H));
    bit_count = 0;
}

// processes one chunk of 64 bytes 
inline void sha1_impl::process_block(const void* msg)
{
    word_type Ws[80];
    word_type* W = be_to_T<8, 32>(Ws, msg, block_size);

    for (int t = 16; t < 80; ++t)
        W[t] = left_rotate<32>(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);

    word_type a = H[0];
    word_type b = H[1];
    word_type c = H[2];
    word_type d = H[3];
    word_type e = H[4];

    for (int t = 0; t < 20; ++t)
    {
        word_type T = left_rotate<32>(a, 5) + Ch(b, c, d) + e + K[0] + W[t];
        e = d;
        d = c;
        c = left_rotate<32>(b, 30);
        b = a;
        a = T;
    }
    for (int t = 20; t < 40; ++t)
    {
        word_type T = left_rotate<32>(a, 5) + Parity(b, c, d) + e + K[1] + W[t];
        e = d;
        d = c;
        c = left_rotate<32>(b, 30);
        b = a;
        a = T;
    }
    for (int t = 40; t < 60; ++t)
    {
        word_type T = left_rotate<32>(a, 5) + Maj(b, c, d) + e + K[2] + W[t];
        e = d;
        d = c;
        c = left_rotate<32>(b, 30);
        b = a;
        a = T;
    }
    for (int t = 60; t < 80; ++t)
    {
        word_type T = left_rotate<32>(a, 5) + Parity(b, c, d) + e + K[3] + W[t];
        e = d;
        d = c;
        c = left_rotate<32>(b, 30);
        b = a;
        a = T;
    }

    H[0] += a;
    H[1] += b;
    H[2] += c;
    H[3] += d;
    H[4] += e;
}

inline void sha1_impl::store_bit_count(void* dst) const
{
    bit_count.store_as_big_endian(dst, 1);
}

}

