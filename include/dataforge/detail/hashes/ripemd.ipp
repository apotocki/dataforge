/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <algorithm>
#include <cstring>
#include <utility>

namespace dataforge::ripemd_detail {

// offsets into X array
inline static const int r1[80] = {
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
    7,  4, 13,  1, 10,  6, 15,  3, 12,  0,  9,  5,  2, 14, 11,  8,
    3, 10, 14,  4,  9, 15,  8,  1,  2,  7,  0,  6, 13, 11,  5, 12,
    1,  9, 11, 10,  0,  8, 12,  4, 13,  3,  7, 15, 14,  5,  6,  2,
    4,  0,  5,  9,  7, 12,  2, 10, 14,  1,  3,  8, 11,  6, 15, 13
};

inline static const int r2[80] = {
    5, 14,  7,  0,  9,  2, 11,  4, 13,  6, 15,  8,  1, 10,  3, 12,
    6, 11,  3,  7,  0, 13,  5, 10, 14, 15,  8, 12,  4,  9,  1,  2,
    15,  5,  1,  3,  7, 14,  6,  9, 11,  8, 12,  2, 10,  0,  4, 13,
    8,  6,  4,  1,  3, 11, 15,  0,  5, 12,  2, 13,  9,  7, 10, 14,
    12, 15, 10,  4,  1,  5,  8,  7,  6,  2, 13, 14,  0,  3,  9, 11
};

// values for rotate left
inline static const int s1[80] = {
    11, 14, 15, 12,  5,  8,  7,  9, 11, 13, 14, 15,  6,  7,  9,  8,
    7,  6,  8, 13, 11,  9,  7, 15,  7, 12, 15,  9, 11,  7, 13, 12,
    11, 13,  6,  7, 14,  9, 13, 15, 14,  8, 13,  6,  5, 12,  7,  5,
    11, 12, 14, 15, 14, 15,  9,  8,  9, 14,  5,  6,  8,  6,  5, 12,
    9, 15,  5, 11,  6,  8, 13, 12,  5, 12, 13, 14, 11,  8,  5,  6
};

inline static const int s2[80] = {
    8,  9,  9, 11, 13, 15, 15,  5,  7,  7,  8, 11, 14, 14, 12,  6,
    9, 13, 15,  7, 12,  8,  9, 11,  7,  7, 12,  7,  6, 15, 13, 11,
    9,  7, 15, 11,  8,  6,  6, 14, 12, 13,  5, 14, 13, 13,  7,  5,
    15,  5,  8, 11, 14, 14,  6, 14,  6,  9, 12,  9, 12,  5, 15,  8,
    8,  5, 12,  9, 12,  5, 14,  6,  8, 13,  6,  5, 15, 13, 11, 11
};

struct f1
{
    uint_least32_t operator()(uint_least32_t x, uint_least32_t y, uint_least32_t z) const
    {
        return x ^ y ^ z;
    }
};

struct f2
{
    uint_least32_t operator()(uint_least32_t x, uint_least32_t y, uint_least32_t z) const
    {
        return (x & y) | (~x & z);
    }
};

struct f3
{
    uint_least32_t operator()(uint_least32_t x, uint_least32_t y, uint_least32_t z) const
    {
        return (x | ~y) ^ z;
    }
};

struct f4
{
    uint_least32_t operator()(uint_least32_t x, uint_least32_t y, uint_least32_t z) const
    {
        return (x & z) | (y & ~z);
    }
};

struct f5
{
    uint_least32_t operator()(uint_least32_t x, uint_least32_t y, uint_least32_t z) const
    {
        return x ^ (y | ~z);
    }
};

template<class F>
static void transform(uint_least32_t& a, uint_least32_t& b, uint_least32_t& c, uint_least32_t& d,
    uint_least32_t x, uint_least32_t k, uint_least32_t s)
{
    uint_least32_t T = left_rotate<32>(a + F()(b, c, d) + x + k, s);
    a = d;
    d = c;
    c = b;
    b = T;
}

template<class F>
static void transform(uint_least32_t& a, uint_least32_t& b, uint_least32_t& c, uint_least32_t& d,
    uint_least32_t& e, uint_least32_t x, uint_least32_t k, uint_least32_t s)
{
    uint_least32_t T = left_rotate<32>(a + F()(b, c, d) + x + k, s) + e;
    a = e;
    e = d;
    d = left_rotate<32>(c, 10);
    c = b;
    b = T;
}

inline const uint_least32_t ripemd_impl_base<ripemd_type::ripemd128>::init_values[4] =
{
    0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u
};

inline const uint_least32_t ripemd_impl_base<ripemd_type::ripemd160>::init_values[5] =
{
    0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u, 0xc3d2e1f0u
};

inline const uint_least32_t ripemd_impl_base<ripemd_type::ripemd256>::init_values[8] =
{
    0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u,
    0x76543210u, 0xfedcba98u, 0x89abcdefu, 0x01234567u
};

inline const uint_least32_t ripemd_impl_base<ripemd_type::ripemd320>::init_values[10] =
{
  0x67452301u, 0xefcdab89u, 0x98badcfeu, 0x10325476u, 0xc3d2e1f0u,
  0x76543210u, 0xfedcba98u, 0x89abcdefu, 0x01234567u, 0x3c2d1e0fu
};

template <ripemd_type Type>
ripemd_impl<Type>::ripemd_impl()
{
    reset();
}

template <ripemd_type Type>
void ripemd_impl<Type>::reset()
{
    std::memcpy(ripemd_impl::h, ripemd_impl::init_values, sizeof(ripemd_impl::init_values));
}

inline void ripemd_impl_base<ripemd_type::ripemd128>::process_block(const void* msg)
{
    word_type Xs[16];
    const word_type* X = le_to_T<8, 32, const word_type>(Xs, msg, ripemd_impl_base::block_size);
    
    word_type A1 = h[0], B1 = h[1], C1 = h[2], D1 = h[3];
    word_type A2 = h[0], B2 = h[1], C2 = h[2], D2 = h[3];

    // round 1
    for (int j = 0; j < 16; ++j)
    {
        transform<f1>(A1, B1, C1, D1, X[r1[j]], 0x00000000, s1[j]);
        transform<f4>(A2, B2, C2, D2, X[r2[j]], 0x50a28be6, s2[j]);
    }
    // round 2
    for (int j = 16; j < 32; ++j)
    {
        transform<f2>(A1, B1, C1, D1, X[r1[j]], 0x5a827999, s1[j]);
        transform<f3>(A2, B2, C2, D2, X[r2[j]], 0x5c4dd124, s2[j]);
    }
    // round 3
    for (int j = 32; j < 48; ++j)
    {
        transform<f3>(A1, B1, C1, D1, X[r1[j]], 0x6ed9eba1, s1[j]);
        transform<f2>(A2, B2, C2, D2, X[r2[j]], 0x6d703ef3, s2[j]);
    }
    // round 4
    for (int j = 48; j < 64; ++j)
    {
        transform<f4>(A1, B1, C1, D1, X[r1[j]], 0x8f1bbcdc, s1[j]);
        transform<f1>(A2, B2, C2, D2, X[r2[j]], 0x00000000, s2[j]);
    }

    word_type T = h[1] + C1 + D2;
    h[1] = h[2] + D1 + A2;
    h[2] = h[3] + A1 + B2;
    h[3] = h[0] + B1 + C2;
    h[0] = T;
}

inline void ripemd_impl_base<ripemd_type::ripemd160>::process_block(const void* msg)
{
    word_type Xs[16];
    const word_type* X = le_to_T<8, 32, const word_type>(Xs, msg, ripemd_impl_base::block_size);
    
    word_type A1 = h[0], B1 = h[1], C1 = h[2], D1 = h[3], E1 = h[4];
    word_type A2 = h[0], B2 = h[1], C2 = h[2], D2 = h[3], E2 = h[4];

    // round 1
    for (int j = 0; j < 16; ++j)
    {
        transform<f1>(A1, B1, C1, D1, E1, X[r1[j]], 0x00000000, s1[j]);
        transform<f5>(A2, B2, C2, D2, E2, X[r2[j]], 0x50a28be6, s2[j]);
    }
    // round 2
    for (int j = 16; j < 32; ++j)
    {
        transform<f2>(A1, B1, C1, D1, E1, X[r1[j]], 0x5a827999, s1[j]);
        transform<f4>(A2, B2, C2, D2, E2, X[r2[j]], 0x5c4dd124, s2[j]);
    }
    // round 3
    for (int j = 32; j < 48; ++j)
    {
        transform<f3>(A1, B1, C1, D1, E1, X[r1[j]], 0x6ed9eba1, s1[j]);
        transform<f3>(A2, B2, C2, D2, E2, X[r2[j]], 0x6d703ef3, s2[j]);
    }
    // round 4
    for (int j = 48; j < 64; ++j)
    {
        transform<f4>(A1, B1, C1, D1, E1, X[r1[j]], 0x8f1bbcdc, s1[j]);
        transform<f2>(A2, B2, C2, D2, E2, X[r2[j]], 0x7a6d76e9, s2[j]);
    }
    // round 5
    for (int j = 64; j < 80; ++j)
    {
        transform<f5>(A1, B1, C1, D1, E1, X[r1[j]], 0xa953fd4e, s1[j]);
        transform<f1>(A2, B2, C2, D2, E2, X[r2[j]], 0x00000000, s2[j]);
    }

    word_type T = h[1] + C1 + D2;
    h[1] = h[2] + D1 + E2;
    h[2] = h[3] + E1 + A2;
    h[3] = h[4] + A1 + B2;
    h[4] = h[0] + B1 + C2;
    h[0] = T;
}

inline void ripemd_impl_base<ripemd_type::ripemd256>::process_block(const void* msg)
{
    word_type Xs[16];
    const word_type* X = le_to_T<8, 32, const word_type>(Xs, msg, ripemd_impl_base::block_size);

    word_type Y[8];
    std::memcpy(Y, h, sizeof(Y));

    // round 1
    for (int j = 0; j < 16; ++j)
    {
        transform<f1>(Y[0], Y[1], Y[2], Y[3], X[r1[j]], 0x00000000, s1[j]);
        transform<f4>(Y[4], Y[5], Y[6], Y[7], X[r2[j]], 0x50a28be6, s2[j]);
    }
    std::swap(Y[0], Y[4]);
    // round 2
    for (int j = 16; j < 32; ++j)
    {
        transform<f2>(Y[0], Y[1], Y[2], Y[3], X[r1[j]], 0x5a827999, s1[j]);
        transform<f3>(Y[4], Y[5], Y[6], Y[7], X[r2[j]], 0x5c4dd124, s2[j]);
    }
    std::swap(Y[1], Y[5]);
    // round 3
    for (int j = 32; j < 48; ++j)
    {
        transform<f3>(Y[0], Y[1], Y[2], Y[3], X[r1[j]], 0x6ed9eba1, s1[j]);
        transform<f2>(Y[4], Y[5], Y[6], Y[7], X[r2[j]], 0x6d703ef3, s2[j]);
    }
    std::swap(Y[2], Y[6]);
    // round 4
    for (int j = 48; j < 64; ++j)
    {
        transform<f4>(Y[0], Y[1], Y[2], Y[3], X[r1[j]], 0x8f1bbcdc, s1[j]);
        transform<f1>(Y[4], Y[5], Y[6], Y[7], X[r2[j]], 0x00000000, s2[j]);
    }
    std::swap(Y[3], Y[7]);

    for (int i = 0; i < 8; ++i)
        h[i] += Y[i];
}

inline void ripemd_impl_base<ripemd_type::ripemd320>::process_block(const void* msg)
{
    word_type Xs[16];
    const word_type* X = le_to_T<8, 32, const word_type>(Xs, msg, ripemd_impl_base::block_size);

    word_type Y[10];
    std::memcpy(Y, h, sizeof(Y));

    // round 1
    for (int j = 0; j < 16; ++j)
    {
        transform<f1>(Y[0], Y[1], Y[2], Y[3], Y[4], X[r1[j]], 0x00000000, s1[j]);
        transform<f5>(Y[5], Y[6], Y[7], Y[8], Y[9], X[r2[j]], 0x50a28be6, s2[j]);
    }
    std::swap(Y[1], Y[6]);
    // round 2
    for (int j = 16; j < 32; ++j)
    {
        transform<f2>(Y[0], Y[1], Y[2], Y[3], Y[4], X[r1[j]], 0x5a827999, s1[j]);
        transform<f4>(Y[5], Y[6], Y[7], Y[8], Y[9], X[r2[j]], 0x5c4dd124, s2[j]);
    }
    std::swap(Y[3], Y[8]);
    // round 3
    for (int j = 32; j < 48; ++j)
    {
        transform<f3>(Y[0], Y[1], Y[2], Y[3], Y[4], X[r1[j]], 0x6ed9eba1, s1[j]);
        transform<f3>(Y[5], Y[6], Y[7], Y[8], Y[9], X[r2[j]], 0x6d703ef3, s2[j]);
    }
    std::swap(Y[0], Y[5]);
    // round 4
    for (int j = 48; j < 64; ++j)
    {
        transform<f4>(Y[0], Y[1], Y[2], Y[3], Y[4], X[r1[j]], 0x8f1bbcdc, s1[j]);
        transform<f2>(Y[5], Y[6], Y[7], Y[8], Y[9], X[r2[j]], 0x7a6d76e9, s2[j]);
    }
    std::swap(Y[2], Y[7]);
    // round 5
    for (int j = 64; j < 80; ++j)
    {
        transform<f5>(Y[0], Y[1], Y[2], Y[3], Y[4], X[r1[j]], 0xa953fd4e, s1[j]);
        transform<f1>(Y[5], Y[6], Y[7], Y[8], Y[9], X[r2[j]], 0x00000000, s2[j]);
    }
    std::swap(Y[4], Y[9]);

    for (int i = 0; i < 10; ++i)
        h[i] += Y[i];
}

}
