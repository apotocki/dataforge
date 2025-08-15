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
class des_cipher
{
public:
    inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline static constexpr size_t block_wsize() noexcept { return 1; } // in words
    inline static constexpr size_t word_size = 64; // in bits
    using word_type = uint_least64_t;

    template <typename QrkT>
    explicit des_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;
    
    inline size_t calculate_size() const
    {
        size_t algo_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return algo_sz + sizeof(word_type) * 16 * levels_;
    }

private:
    void do_expand_key(std::span<const unsigned char> key);
    void process_block(const word_type* in, word_type* out, bool is_decryption) noexcept;

    inline word_type* sub_key() noexcept
    {
        static constexpr size_t aligned_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return reinterpret_cast<word_type*>(reinterpret_cast<char*>(this) + aligned_sz) + 16 * level_;
    }

    uint_least8_t ed_mode_ : 1;
    uint_least8_t levels_ : 7;
    int_least8_t level_;
};

struct des_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = des_cipher<DerivedT>; };
};

}

#include "des.ipp"
