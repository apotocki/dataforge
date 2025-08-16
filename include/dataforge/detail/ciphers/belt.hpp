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

template <typename DerivedT>
class belt_cipher
{
public:
    inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline static constexpr size_t block_wsize() noexcept { return 4; } // in words
    inline static constexpr size_t word_size = 32; // in bits
    using word_type = uint_least32_t;

    template <typename QrkT>
    explicit belt_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

    inline size_t calculate_size() const
    {
        size_t algo_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        size_t ks_sz = sizeof(word_type) * 2 * block_wsize();
        return algo_sz + ks_sz;
    }

private:
    inline word_type* ks_begin() noexcept
    {
        static constexpr size_t aligned_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return std::launder(reinterpret_cast<word_type*>(reinterpret_cast<char*>(this) + aligned_sz));
    }
};

struct belt_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = belt_cipher<DerivedT>; };
};

}

#include "belt.ipp"
