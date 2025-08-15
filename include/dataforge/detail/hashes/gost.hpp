/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>
#include <array>
#include <algorithm>

#include "../utility/digest_base.hpp"

namespace dataforge::gost_detail {

struct gost_impl : digest_base<gost_impl, 32, uint_least32_t, 8>
{
    explicit gost_impl(bool crypto_pro_sbox);

    template <typename InitQuarkT>
    explicit gost_impl(InitQuarkT const& dt);

    void reset() noexcept;

    void process_block(const void* msg);
    void finalize();

    using digest_word_type = uint_least32_t;
    static constexpr size_t digest_word_bit_count = 32;
    static constexpr std::endian digest_endianness() { return std::endian::little; }

    static constexpr size_t state_size = 8;
    inline std::span<digest_word_type, state_size> digest_span() { return hash; }

    inline constexpr size_t digest_length() const noexcept { return 32; }

protected:
    void init_sbox(uint_least8_t(&sbox)[8][16]);
    void process_block(const uint_least32_t* mps);
    void gH(const uint_least32_t* m);
    void gRound(uint_least32_t k1, uint_least32_t k2);

protected:
    uint_least32_t* sbox_1;
    uint_least32_t* sbox_2;
    uint_least32_t* sbox_3;
    uint_least32_t* sbox_4;
    std::array<uint_least32_t, state_size> hash;
    std::array<uint_least32_t, 8> sum;
    uint_least32_t r, l;
};

}

#include "gost.ipp"
