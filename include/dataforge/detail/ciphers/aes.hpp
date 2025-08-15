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
class aes_cipher
{
public:
    inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline size_t block_wsize() const noexcept { return nb; } // in words
    inline static constexpr size_t word_size = 32; // in bits
    using word_type = uint_least32_t;

    template <typename QrkT>
    explicit aes_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

    inline size_t calculate_size() const
    {
        size_t algo_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        size_t ekey_sz = sizeof(word_type) * nb * (nr + 1);
        size_t state_sz = sizeof(word_type) * nb;
        return algo_sz + ekey_sz + state_sz;
    }

private:
    inline word_type* ekey_begin() noexcept
    {
        static constexpr size_t aligned_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return reinterpret_cast<word_type*>(reinterpret_cast<char*>(this) + aligned_sz);
    }

    //inline word_type* state_begin() noexcept { return ekey_begin() + nb * (nr + 1); }
    inline unsigned char* state_begin() noexcept { return reinterpret_cast<unsigned char*>(ekey_begin() + nb * (nr + 1)); }

    uint_least16_t nk, nr, nb;
};

struct aes_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = aes_cipher<DerivedT>; };
};

}

#include "aes.ipp"
