/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include "../utility/data_ops.hpp"

namespace dataforge {

template <typename DerivedT, uint_least8_t W>
template <typename QrkT>
rc5_cipher<DerivedT, W>::rc5_cipher(QrkT const& q)
    : r_{ q.r }
{}

template <typename DerivedT, uint_least8_t W>
void rc5_cipher<DerivedT, W>::expand_key(std::span<const unsigned char> key)
{
    assert(key.size() < 256);

    const uint_least8_t u = W / 8;
    uint_least16_t t = 2 * (r_ + 1);
    uint_least8_t c = static_cast<uint_least8_t>((key.size() + u - 1) / u);

    // init S-table
    s()[0] = defs_t::P;
    for (uint_least16_t i = 1; i < t; ++i)
    {
        s()[i] = s()[i - 1] + defs_t::Q;
    }

    if (key.empty()) {
        word_type A = 0, B = 0;
        for (uint_least16_t i = 0, n = 3 * t; n > 0; --n, i = (1 + i) % t)
        {
            s()[i] = A = left_rotate<W, word_type>((s()[i] + A + B), 3);
            B += A;
            B = left_rotate<W, word_type>(B, B);
        }
    } else {
        // init L-table
        word_type L[32]; // 32 is max possible c
        std::fill(L, L + c, 0);
        le_copy<8, W>(key.data(), key.size(), L);
        
        // mixing S
        word_type A = 0, B = 0;
        for (uint_least16_t i = 0, j = 0, n = 3 * (t > c ? t : c); n > 0; --n, i = (1 + i) % t, j = (1 + j) % c)
        {
            s()[i] = A = left_rotate<W, word_type>((s()[i] + A + B), 3);
            B += A;
            L[j] = B = left_rotate<W, word_type>(L[j] + B, B);
        }
    }
}

template <typename DerivedT, uint_least8_t W>
void rc5_cipher<DerivedT, W>::encrypt_block(const word_type* in, word_type* out) noexcept
{
    out[0] = in[0]; out[1] = in[1];
    word_type &A = out[0], &B = out[1];

    A += s()[0]; B += s()[1];
    for (uint_least16_t i = 1; i <= r_; ++i)
    {
        A = left_rotate<W, word_type>(A ^ B, B) + s()[2 * i];
        B = left_rotate<W, word_type>(A ^ B, A) + s()[2 * i + 1];
    }
}

template <typename DerivedT, uint_least8_t W>
void rc5_cipher<DerivedT, W>::decrypt_block(const word_type* in, word_type* out) noexcept
{
    out[0] = in[0]; out[1] = in[1];
    word_type &A = out[0], &B = out[1];

    for (uint_least16_t i = r_; i > 0; --i)
    {
        B = A ^ right_rotate<W, word_type>(B - s()[2 * i + 1], A);
        A = B ^ right_rotate<W, word_type>(A - s()[2 * i], B);
    }
    A -= s()[0]; B -= s()[1];
}

}
