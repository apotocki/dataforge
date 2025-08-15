/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../utility/digest_base.hpp"

namespace dataforge {
    
enum class sha2_type : int {
    sha224, sha256, sha384, sha512, sha512_256, sha512_224
};

namespace sha2_detail {

template <size_t SzV> struct sha2_def_base;

template <>
struct sha2_def_base<256>
{
    using word_type = uint_least32_t;
    static const int message_schedule_length = 64;
    static const int input_length_size = 8;
    static const int word_type_byte_count = 4;

    static const word_type K[64];

    // values for Sigma and sigma functions
    static const word_type S0[3];
    static const word_type S1[3];
    static const word_type s0[3];
    static const word_type s1[3];
};

template <>
struct sha2_def_base<512>
{
    using word_type = uint_least64_t;
    static const int message_schedule_length = 80;
    static const int input_length_size = 16;
    static const int word_type_byte_count = 8;

    static const word_type K[80];

    // values for Sigma and sigma functions
    static const word_type S0[3];
    static const word_type S1[3];
    static const word_type s0[3];
    static const word_type s1[3];
};

template <sha2_type Type> struct sha2_impl;
template <sha2_type Type> struct sha2_impl_base;

template <>
struct sha2_impl_base<sha2_type::sha224> : sha2_def_base<256>, digest_base<sha2_impl<sha2_type::sha224>, 64>
{
    static constexpr int digest_length() { return 28; }
    static const word_type init_values[8];
};

template <>
struct sha2_impl_base<sha2_type::sha256> : sha2_def_base<256>, digest_base<sha2_impl<sha2_type::sha256>, 64>
{
    static constexpr int digest_length() { return 32; }
    static const word_type init_values[8];
};

template <>
struct sha2_impl_base<sha2_type::sha384> : sha2_def_base<512>, digest_base<sha2_impl<sha2_type::sha384>, 128>
{
    static constexpr int digest_length() { return 48; }
    static const word_type init_values[8];
};

template <>
struct sha2_impl_base<sha2_type::sha512> : sha2_def_base<512>, digest_base<sha2_impl<sha2_type::sha512>, 128>
{
    static constexpr int digest_length() { return 64; }
    static const word_type init_values[8];
};

template <>
struct sha2_impl_base<sha2_type::sha512_224> : sha2_def_base<512>, digest_base<sha2_impl<sha2_type::sha512_224>, 128>
{
    static constexpr int digest_length() { return 28; }
    static const word_type init_values[8];
};

template <>
struct sha2_impl_base<sha2_type::sha512_256> : sha2_def_base<512>, digest_base<sha2_impl<sha2_type::sha512_256>, 128>
{
    static constexpr int digest_length() { return 32; }
    static const word_type init_values[8];
};

template <sha2_type Type>
struct sha2_impl : sha2_impl_base<Type>
{
    static constexpr std::endian digest_endianness() { return std::endian::big; }
    using word_type = typename sha2_impl_base<Type>::word_type;
    static constexpr size_t word_bit_count = sha2_impl_base<Type>::word_type_byte_count * 8;

    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = word_bit_count;

    sha2_impl();

    void reset();

    void process_block(const void* msg);
    void store_bit_count(void* dst) const;

    static constexpr size_t state_size = 8;
    inline std::span<digest_word_type, state_size> digest_span() { return H; }

private:
    static word_type Ch(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (~x & z);
    }

    static word_type Maj(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    static word_type Sigma(word_type x, const word_type y[3])
    {
        return right_rotate<word_bit_count>(x, y[0]) ^ right_rotate<word_bit_count>(x, y[1]) ^ right_rotate<word_bit_count>(x, y[2]);
    }

    static word_type sigma(word_type x, const word_type y[3])
    {
        return right_rotate<word_bit_count>(x, y[0]) ^ right_rotate<word_bit_count>(x, y[1]) ^ (x >> y[2]);
    }

    word_type H[state_size];
};

}}

#include "sha2.ipp"
