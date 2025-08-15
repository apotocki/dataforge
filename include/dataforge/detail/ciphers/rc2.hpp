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
class rc2_cipher
{
public:
    inline static constexpr std::endian cipher_endianness() noexcept { return std::endian::little; };
    inline static constexpr size_t block_wsize() noexcept { return 4; } // in words
    inline static constexpr size_t word_size = 16; // in bits
    using word_type = uint_least16_t;
    
    template <typename QrkT>
    explicit rc2_cipher(QrkT const& q);
    
    void expand_key(std::span<const unsigned char> key);
    void encrypt_block(const word_type* in, word_type* out) noexcept;
    void decrypt_block(const word_type* in, word_type* out) noexcept;

    inline size_t calculate_size() const
    {
        size_t lsz = (max_effective_keylength_ + 7) / 8;
        size_t totalsz = sizeof(DerivedT) + lsz /* L */ + sizeof(word_type) * lsz / 2 /* ekey */;
        return totalsz;
    }

protected:
    // TM = 255 MOD 2^(8 + T1 - 8*T8);
    inline unsigned char tm() const noexcept { return ((unsigned char)255) >> ((8 - (effective_keylength_ % 8)) % 8); }
    inline size_t t8() const noexcept { return (effective_keylength_ + 7) / 8; }
    inline size_t l_size() const noexcept { return (max_effective_keylength_ + 7) / 8; }
    inline size_t ekey_size() const noexcept { return (max_effective_keylength_ + 7) / 16; }

    inline word_type* ekey_begin() noexcept { return reinterpret_cast<word_type*>(reinterpret_cast<unsigned char*>(this) + sizeof(DerivedT)); }
    inline unsigned char* l_begin() noexcept { return reinterpret_cast<unsigned char*>(ekey_begin() + ekey_size()); }

private:
    size_t max_effective_keylength_;
    size_t effective_keylength_;
};

struct rc2_cipher_type_factory
{
    template <typename DerivedT>
    struct apply { using type = rc2_cipher<DerivedT>; };
};

}

#include "rc2.ipp"
