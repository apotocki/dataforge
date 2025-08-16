/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../utility/digest_base.hpp"

namespace dataforge {

enum class blake_type : int {
    blake224 = 224, blake256 = 256, blake384 = 384, blake512 = 512
};

enum class blake2_type : int {
    blake2s224 = 224, blake2s256 = 256, blake2b384 = 384, blake2b512 = 512
};

namespace blake_detail {

template <size_t WordSzV> struct blake_def_base;
template <size_t BitsV> struct blake_impl_iv_base;
template <blake_type Type, size_t WordBitsV> struct blake_impl_base;
template <typename EnumT, EnumT Type> struct blake_impl;

template <>
struct blake_impl_iv_base<224>
{
    using word_type = uint_least32_t;
    static const word_type iv[8];
};

template <>
struct blake_impl_iv_base<256>
{
    using word_type = uint_least32_t;
    static const word_type iv[8];
};

template <>
struct blake_impl_iv_base<384>
{
    using word_type = uint_least64_t;
    static const word_type iv[8];
};

template <>
struct blake_impl_iv_base<512>
{
    using word_type = uint_least64_t;
    static const word_type iv[8];
};

template <>
struct blake_def_base<32>
{
    static constexpr size_t word_size = 32;
    static constexpr int input_length_size = 8;
    static const uint_least32_t c[16];
};

template <>
struct blake_def_base<64>
{
    static constexpr size_t word_size = 64;
    static constexpr int input_length_size = 16;
    static const uint_least64_t c[16];
};

template <blake_type Type, size_t WordBitsV>
struct blake_impl_base
    : blake_def_base<WordBitsV>
    , blake_impl_iv_base<(int)Type>
    , digest_base<blake_impl<blake_type, Type>, 2 * WordBitsV, typename blake_impl_iv_base<(int)Type>::word_type>
{
    static constexpr int digest_length() { return (int)Type / 8; }
    static constexpr std::endian digest_endianness() { return std::endian::big; }
    static constexpr std::byte special_padding_byte { ((int)Type % 256) ? 0u : 1u };
    static constexpr size_t digest_word_bit_count = WordBitsV;
};

template <blake_type Type>
struct blake_impl<blake_type, Type> : blake_impl_base<Type, (int)Type < 384 ? 32 : 64>
{
    using word_type = typename blake_impl::word_type;
    using digest_word_type = word_type;
    
    blake_impl();

    template <typename InitQuarkT>
    explicit blake_impl(InitQuarkT const&);

    void reset();

    void process_block(const void* msg);
    
    void store_bit_count(void* dst);

    static constexpr size_t state_size = 8;
    inline std::span<digest_word_type, state_size> digest_span() { return h; }

private:
    void G(word_type* m, word_type* v, int round, int a, int b, int c, int d, int i);

    digest_word_type h[state_size];
    word_type s[4];
};

template <blake2_type Type, size_t WordBitsV>
struct blake2_impl_base
    : blake_impl_iv_base<WordBitsV == 32 ? 256 : 512>
    , digest_base<blake_impl<blake2_type, Type>, 2 * WordBitsV, typename blake_impl_iv_base<WordBitsV == 32 ? 256 : 512>::word_type>
{
    using base_t = blake2_impl_base;
    static constexpr size_t word_size = WordBitsV;
    static constexpr size_t input_length_size = WordBitsV / 4;
    static constexpr size_t digest_length() { return (int)Type / 8; }
    static constexpr std::endian digest_endianness() { return std::endian::little; }
};

template <blake2_type Type>
struct blake_impl<blake2_type, Type> : blake2_impl_base<Type, (int)Type < 384 ? 32 : 64>
{
    using word_type = typename blake_impl::word_type;
    static constexpr size_t word_size = blake_impl::base_t::word_size;
    static constexpr std::byte padding_byte { 0x0u };
    
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = word_size;
    static constexpr bool allow_full_buffer = true;

    blake_impl();

    template <typename InitQuarkT>
    explicit blake_impl(InitQuarkT const& dt);

    void reset();

    inline void set_lastnode() { f[1] = (std::numeric_limits<word_type>::max)(); }
    inline bool is_lastblock() { return f[0] != 0; }
    inline void set_lastblock()
    {
        if (last_node) set_lastnode();
        f[0] = (std::numeric_limits<word_type>::max)();
    }

    void process_block(const void* msg);

    void input(const void* vdata, size_t len)
    {
        if (len) {
            blake_impl::base_t::input(vdata, len);
            full_buff = !(blake_impl::bit_count[0] % blake_impl::block_size);
        }
    }

    static constexpr size_t state_size = 8;
    inline std::span<digest_word_type, state_size> digest_span() { return h; }

    void finalize();

    unsigned int bytes_in_buf() const noexcept { return full_buff ? blake_impl::block_size : blake_impl::bit_count[0] % blake_impl::block_size; }
    void count_bytes(size_t x) { blake_impl::bit_count.add(x); }

private:
    void G(const word_type* m, word_type* v, int round, int a, int b, int c, int d, int i);

    static constexpr digest_word_type magic_xor_value = 0x01010000u;

    unsigned char key[blake_impl::block_size];
    word_type f[2];
    digest_word_type h[state_size];
    uint8_t full_buff : 1;
    uint8_t last_node : 1;
    uint8_t key_sz = 0;
};

}}

#include "blake.ipp"
