/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>
#include <cstdint>

namespace dataforge {

template <typename DerivedT, std::endian EndiannessV>
class magma_cipher
{
public:
    inline static constexpr std::endian cipher_endianness() noexcept { return EndiannessV; };
    inline static constexpr size_t block_wsize() noexcept { return 2; } // in words
    inline static constexpr size_t word_size = 32; // in bits
    using word_type = uint_least32_t;

    template <typename QrkT>
    explicit magma_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

    inline size_t calculate_size() const
    {
        size_t algo_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        size_t key_sz = sizeof(word_type) * 8;
        return algo_sz + key_sz;
    }

private:
    const unsigned char(&sbox)[8][16];

    inline word_type* key_begin() noexcept
    {
        static constexpr size_t aligned_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return reinterpret_cast<word_type*>(reinterpret_cast<char*>(this) + aligned_sz);
    }

    inline uint_least32_t t(uint_least32_t val);

};

template <std::endian Endianness>
struct magma_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = magma_cipher<DerivedT, Endianness>; };
};

}

#include "magma.ipp"
