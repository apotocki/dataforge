/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../utility/digest_base.hpp"

namespace dataforge {

enum class ripemd_type : int {
    ripemd128, ripemd160, ripemd256, ripemd320
};

namespace ripemd_detail {

template <ripemd_type Type> struct ripemd_impl_base;
template <ripemd_type Type> struct ripemd_impl;

template <>
struct ripemd_impl_base<ripemd_type::ripemd128> : digest_base<ripemd_impl<ripemd_type::ripemd128>, 64>
{
    using word_type = uint_least32_t;

    static const word_type init_values[4];

    void process_block(const void* msg);

    word_type h[4];
};

template <>
struct ripemd_impl_base<ripemd_type::ripemd160> : digest_base<ripemd_impl<ripemd_type::ripemd160>, 64>
{
    using word_type = uint_least32_t;

    static const word_type init_values[5];

    void process_block(const void* msg);

    word_type h[5];
};

template <>
struct ripemd_impl_base<ripemd_type::ripemd256> : digest_base<ripemd_impl<ripemd_type::ripemd256>, 64>
{
    using word_type = uint_least32_t;

    static const word_type init_values[8];

    void process_block(const void* msg);

    word_type h[8];
};

template <>
struct ripemd_impl_base<ripemd_type::ripemd320> : digest_base<ripemd_impl<ripemd_type::ripemd320>, 64>
{
    using word_type = uint_least32_t;

    static const word_type init_values[10];

    void process_block(const void* msg);

    word_type h[10];
};

template <ripemd_type Type>
struct ripemd_impl : ripemd_impl_base<Type>
{
    using base_t = ripemd_impl_base<Type>;
    using word_type = typename base_t::word_type;
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = 32;

    static const int input_length_size = 8;
    static constexpr int digest_length() { return sizeof(ripemd_impl::init_values) / sizeof(ripemd_impl::word_type) * 4; }
    static constexpr std::endian digest_endianness() { return std::endian::little; }

    explicit ripemd_impl();

    void reset();

    static constexpr size_t state_size = sizeof(base_t::h) / sizeof(word_type);
    inline std::span<digest_word_type, state_size> digest_span() { return this->h; }
};

}}

#include "ripemd.ipp"
