/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge::rc4_detail {

class rc4_impl
{
    rc4_impl(std::span<const unsigned char> key, uint_least8_t n, size_t skip_bits) noexcept;

public:
    inline uint_least16_t* sblock_begin() noexcept { return reinterpret_cast<uint_least16_t*>(reinterpret_cast<char*>(this) + sizeof(rc4_impl)); }
    
    template <typename ConsumerT>
    void process_data(std::span<const unsigned char> inspan, ConsumerT out) noexcept;

    void reset() noexcept;

    [[nodiscard]]
    static rc4_impl* create(std::span<const unsigned char> key, uint_least8_t n, size_t skip_bits = 0);

    void destroy();

private:
    std::span<const unsigned char> key_;
    size_t skip_bits_;
    uint_least32_t s_sz_;
    uint_least32_t buf;
    uint_least16_t i_, j_;
    uint_least8_t bits_in_buf;
    uint_least8_t n_;
};

template <typename ConsumerT>
void rc4_impl::process_data(std::span<const unsigned char> inspan, ConsumerT out) noexcept
{
    if (inspan.empty()) return;
    size_t mask = s_sz_ - 1;
    size_t k = 0;
    uint_least16_t* sblock = sblock_begin();
    for (; bits_in_buf >= 8 && k < inspan.size(); ++k) {
        out(static_cast<unsigned char>((buf & 255) ^ inspan[k]));
        bits_in_buf -= 8;
        buf >>= 8;
    }
    while (k < inspan.size()) {
        i_ = (i_ + 1) & mask;
        j_ = (j_ + sblock[i_]) & mask;
        std::swap(sblock[i_], sblock[j_]);
        uint_least16_t t = (sblock[i_] + sblock[j_]) & mask;
        buf |= ((uint_least32_t)sblock[t]) << bits_in_buf;
        bits_in_buf += n_;

        for (; bits_in_buf >= 8 && k < inspan.size(); ++k) {
            out(static_cast<unsigned char>((buf & 255) ^ inspan[k]));
            bits_in_buf -= 8;
            buf >>= 8;
        }
    }
}

}

#include "rc4.ipp"
