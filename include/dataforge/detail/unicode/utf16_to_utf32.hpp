/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "utf16.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class utf16_to_utf32
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf16_to_utf32<ErrorHandlerT>, char16_t, char32_t, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char16_t st;

public:
    template <typename ErrorHandlerT2>
    explicit utf16_to_utf32(utf16_qrk<ErrorHandlerT> const& quark, utf32_qrk<ErrorHandlerT2> const&)
        : base_t{ quark }, st{0}
    {}

    template <SpanOf<char16_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <typename ConsumerT>
    void push(char16_t val, ConsumerT cons)
    {
        if (!st) {
            if (val < UTF16_SUR_HIGH_START || val > UTF16_SUR_LOW_END) {
                cons(static_cast<char32_t>(val));
            } else if (val <= UTF16_SUR_HIGH_END) { // found high surrogate
                st = val;
            } else {
                on_error("wrong utf16 character", val, *this);
            }
        } else if (val >= UTF16_SUR_LOW_START && val <= UTF16_SUR_LOW_END) {
            char32_t lead = st;
            char32_t trail = val;
            cons(static_cast<char32_t>((lead << 10) + trail - UTF16_SURROGATE_OFFSET));
            st = 0;
        } else {
            on_error("wrong utf16 character", val, *this);
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        if (st) {
            on_error("unfinished utf16 sequence", *this);
        }
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf16_qrk<FromEHT>, utf32_qrk<ToEHT>>
{
    using type = utf16_to_utf32<FromEHT>;
};

}
