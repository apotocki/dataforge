/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include "../utility/data_ops.hpp"
#include "belt_encrypt.hpp"

namespace dataforge::belt_detail {

inline void belt_decrypt(const uint_least32_t* key, const uint_least32_t* inBlock, uint_least32_t* outBlock)
{
    uint_least32_t a = *inBlock++;
    uint_least32_t b = *inBlock++;
    uint_least32_t c = *inBlock++;
    uint_least32_t d = *inBlock;

    // KeyIndex[y][x] = ((8 - y) % 8 + x) % 8;
    for (int_least32_t i = 7; i >= 0; --i)
    {
        int j = (8 - i) % 8;

        b ^= G(a + key[(j + 6) % 8], 5);
        c ^= G(d + key[(j + 5) % 8], 21);
        a = (uint32_t)(a - G(b + key[(j + 4) % 8], 13));
        uint_least32_t e = (G(b + c + key[(j + 3) % 8], 21) ^ (uint32_t)(i + 1));
        b += e;
        c = (uint32_t)(c - e);
        d += G(c + key[(j + 2) % 8], 13);
        b ^= G(a + key[(j + 1) % 8], 21);
        c ^= G(d + key[j], 5);
        std::swap(a, b);
        std::swap(c, d);
        std::swap(a, d);
    }

    *outBlock++ = c;
    *outBlock++ = a;
    *outBlock++ = d;
    *outBlock = b;
}

}

namespace dataforge {
    
template <typename DerivedT>
template <typename QrkT>
belt_cipher<DerivedT>::belt_cipher(QrkT const&)
{

}

template <typename DerivedT>
void belt_cipher<DerivedT>::expand_key(std::span<const unsigned char> key)
{
    auto* ks = ks_begin();
    std::fill(ks, ks + 8, 0);
    size_t ksz = (std::min)(key.size(), (size_t)32);
    le_copy<8, 32>(key.data(), ksz, ks);

    if (ksz <= 16) {
        std::copy(ks, ks + 4, ks + 4);
    }
    else if (ksz <= 24) {
        ks[6] = ks[0] ^ ks[1] ^ ks[2];
        ks[7] = ks[3] ^ ks[4] ^ ks[5];
    }
}

template <typename DerivedT>
void belt_cipher<DerivedT>::encrypt_block(const word_type* in, word_type* out) noexcept
{
    belt_detail::belt_encrypt(ks_begin(), in, out);
}

template <typename DerivedT>
void belt_cipher<DerivedT>::decrypt_block(const word_type* in, word_type* out) noexcept
{
    belt_detail::belt_decrypt(ks_begin(), in, out);
}

}
