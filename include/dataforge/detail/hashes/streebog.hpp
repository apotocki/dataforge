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

namespace dataforge::streebog_detail {

struct streebog_impl : digest_base<streebog_impl, 64>
{
    explicit streebog_impl(size_t hash_size_val);

    template <typename InitQuarkT>
    explicit streebog_impl(InitQuarkT const& dt);

    void reset() noexcept;

    void process_block(const void* msg);
    void finalize();

    static constexpr uint8_t padding_byte = 1u;

    using digest_word_type = uint_least64_t;
    static constexpr size_t digest_word_bit_count = 64;
    static constexpr std::endian digest_endianness() { return std::endian::little; }

    inline std::span<digest_word_type> digest_span() { return {&h[8 - hs / 64], hs}; }
    
    inline size_t digest_length() const noexcept { return hs / 8; }

protected:
    std::array<uint_least64_t, 8> h;
    std::array<uint_least64_t, 8> S;
    size_t hs;
    size_t pos;
    uint_least64_t total;
};

}

#include "streebog.ipp"
