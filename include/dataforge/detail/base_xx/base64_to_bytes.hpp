/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "base64.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class base64_to_bytes
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<base64_to_bytes<ErrorHandlerT>, char, char, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    uint16_t bit : 4;
    uint16_t cache : 12;

    char paddingCV;

public:
    template <IntegralBasedQuark<8> DestT>
    base64_to_bytes(base64_qrk<ErrorHandlerT> const& quark, DestT const&)
        : base_t{ quark }, bit{0}, cache{0}, paddingCV{ quark.paddingCV }
    {}

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT c, ConsumerT cons)
    {
        if (c == paddingCV) { // padding
            if (bit == 0) {
                on_error("unexpected character", c, *this);
            } else {
                bit -= 2;
            }
            return;
        }

        unsigned char v = base64_matrix[static_cast<unsigned char>(c)];

        if (v == 0xff) {
            on_error("unexpected character", c, *this);
            return;
        } else {
            cache <<= 6;
            cache |= v;
        }

        if (bit >= 2) {
            bit -= 2;
            cons(static_cast<char>((cache >> bit) & 0xFF));
        } else {
            bit += 6;
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT)
    {
        if (bit % 8 != 0) {
            on_error("EOF, but the padding character is expected.", *this);
        }
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<base64_qrk<FromEHT>, ToQuarkT>
{
    using type = base64_to_bytes<FromEHT>;
};

}
