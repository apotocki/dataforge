/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <bit>
#include <cstdint>

namespace dataforge {

template <typename DerivedT>
class blowfish_cipher
{
public:
    //inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline std::endian cipher_endianness() noexcept { return compat_mode ? std::endian::little : std::endian::big; };
    inline static constexpr size_t block_wsize() noexcept { return 2; } // in words
    inline static constexpr size_t word_size = 32; // in bits
    using word_type = uint_least32_t;

    static const int rounds = 16;

    template <typename QrkT>
    explicit blowfish_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

private:
    word_type F(word_type x) const noexcept;

    word_type P[rounds + 2];
    word_type S[4][256];

    uint_least8_t compat_mode : 1;
};

struct blowfish_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = blowfish_cipher<DerivedT>; };
};

}

#include "blowfish.ipp"
