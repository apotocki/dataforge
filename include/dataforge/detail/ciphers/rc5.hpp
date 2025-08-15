/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <new>

namespace dataforge::rc5_detail {

template <uint_least8_t W> struct rc5_defs;

template <> struct rc5_defs<16>
{
    using word_type = uint_least16_t;
    static constexpr word_type P = 0xb7e1;
    static constexpr word_type Q = 0x9e37;
};

template <> struct rc5_defs<32>
{
    using word_type = uint_least32_t;
    static constexpr word_type P = 0xb7e15163;
    static constexpr word_type Q = 0x9e3779b9;
};

template <> struct rc5_defs<64>
{
    using word_type = uint_least64_t;
    static constexpr word_type P = 0xb7e151638aed2a6b;
    static constexpr word_type Q = 0x9e3779b97f4a7c15;
};

}

namespace dataforge {

template <typename DerivedT, uint_least8_t W>
class rc5_cipher : public rc5_detail::rc5_defs<W>
{
    using defs_t = rc5_detail::rc5_defs<W>;

public:
    inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline static constexpr size_t block_wsize() noexcept { return 2; } // in words
    inline static constexpr size_t word_size = W; // in bits
    using word_type = typename defs_t::word_type;

    static constexpr uint_least8_t u = W / 8;

    template <typename QrkT>
    explicit rc5_cipher(QrkT const& q);

    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

    inline size_t calculate_size() const
    {
        uint_least16_t t = 2 * (r_ + 1);
        size_t algo_sz = (sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1);
        return algo_sz + sizeof(word_type) * t;
    }

private:
    inline word_type* s() noexcept
    {
        return std::launder(reinterpret_cast<word_type*>(reinterpret_cast<char*>(this) + ((sizeof(DerivedT) + sizeof(word_type) - 1) & ~(sizeof(word_type) - 1))));
    }

    uint_least8_t r_;
};

template <uint_least8_t W>
struct rc5_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = rc5_cipher<DerivedT, W>; };
};

}

#include "rc5.ipp"
