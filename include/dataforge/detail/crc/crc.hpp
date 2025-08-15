/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

#include "../utility/data_ops.hpp"

namespace dataforge::crc_detail {

template <typename CRCT>
struct crc_base
{
    using size_type = size_t;
    using crc_t = CRCT;

    void generate_table(crc_t polynomial, bool reflectin, bool reflectout, crc_t init, crc_t out) noexcept;

    void input(const void* vdata, size_t len) noexcept;

    void finalize() noexcept;

    void reset() noexcept { crc = crc0; }

    crc_t crc, crc0, xorout;
    crc_t table[256];
    bool reflectin_, reflectout_;
};

template <typename CRCT>
void crc_base<CRCT>::generate_table(crc_t polynomial, bool reflectin, bool reflectout, crc_t init, crc_t out) noexcept
{
    crc = init;
    xorout = out;
    const crc_t mask = ((crc_t)1) << (8 * sizeof(crc_t) - 1);

    reflectin_ = reflectin;
    reflectout_ = reflectout;

    for (uint16_t i = 0; i < 256; ++i)
    {
        crc_t crc = ((crc_t)i) << (8 * sizeof(crc_t) - 8);
        for (uint8_t bit = 0; bit < 8; ++bit)
        {
            if (crc & mask)
                crc = (crc << 1) ^ polynomial;
            else
                crc <<= 1;
        }
        table[i] = crc;
    }
    crc0 = crc;
}

template <typename CRCT>
void crc_base<CRCT>::input(const void* vdata, size_t len) noexcept
{
    const uint8_t* buf = reinterpret_cast<const uint8_t*>(vdata);
    for (size_t i = 0; i < len; ++i) {
        uint8_t rval = *buf++;
        crc = (crc << 8) ^ table[0xff & ((crc >> (sizeof(crc_t) * 8 - 8)) ^ (reflectin_ ? reverse_bits(rval) : rval))];
    }
}

template <typename CRCT>
void crc_base<CRCT>::finalize() noexcept
{
    if (reflectout_) {
        crc = reverse_bits(crc);
    }
}

template <std::integral ResultT>
class bytes_to_crc_pusher_base : protected crc_detail::crc_base<ResultT>
{
    using base_t = crc_detail::crc_base<ResultT>;

public:
    using input_element_type = unsigned char;
    using output_element_type = ResultT;

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&&)
    {
        base_t::input(ivals.data(), ivals.size());
    }

    template <Integral<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&& cons)
    {
        base_t::input(&ival, 1);
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        base_t::finalize();
        cons(base_t::crc ^ base_t::xorout);
        base_t::reset();
    }
};

}
