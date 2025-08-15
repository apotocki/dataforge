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
class utf16_to_utf8
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf16_to_utf8<ErrorHandlerT>, char16_t, char8_t, 4>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char16_t st;

public:
    template <typename ErrorHandlerT2>
    explicit utf16_to_utf8(utf16_qrk<ErrorHandlerT> const& quark, utf8_qrk<ErrorHandlerT2> const&)
        : base_t{ quark }, st{0}
    {}

    template <CompatibleSpan<char16_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <IntegralAtLeast<16> CT, typename ConsumerT>
    void push(CT val, ConsumerT cons)
    {
        if (!st) {
            if (val < UTF16_SUR_HIGH_START || val > UTF16_SUR_LOW_END) {
                if (val <= 0x007F) {
                    cons(static_cast<char8_t>(val));
                } else if (val <= 0x07FF) {
                    char8_t c1 = static_cast<char8_t>((val & 0x3F) | 0x80); val >>= 6;
                    const char8_t elems[] = { static_cast<char8_t>((val & 0x1F) | 0xC0), c1 };
                    cons(std::span{ elems, 2 });
                } else {
                    char8_t c2 = static_cast<char8_t>((val & 0x3F) | 0x80); val >>= 6;
                    char8_t c1 = static_cast<char8_t>((val & 0x3F) | 0x80); val >>= 6;
                    const char8_t elems[] = { static_cast<char8_t>((val & 0x0F) | 0xE0), c1, c2 };
                    cons(std::span{ elems, 3 });
                }
            } else if (val <= UTF16_SUR_HIGH_END) { // found high surrogate
                st = static_cast<char16_t>(val);
            } else {
                on_error("wrong utf16 character (unexpected low surrogate)", val, *this);
            }
        } else if (val >= UTF16_SUR_LOW_START && val <= UTF16_SUR_LOW_END) {
            char32_t lead = st;
            char32_t trail = val;
            char32_t uchar = (lead << 10) + trail - UTF16_SURROGATE_OFFSET;
            char8_t c3 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            char8_t c2 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            char8_t c1 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            const char8_t elems[] = { static_cast<char8_t>((uchar & 0x07) | 0xF0), c1, c2, c3 };
            cons(std::span{ elems, 4 });
            st = 0;
        } else {
            on_error("wrong utf16 character (low surrogate is expected)", val, *this);
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
struct cvt_resolver<utf16_qrk<FromEHT>, utf8_qrk<ToEHT>>
{
    using type = utf16_to_utf8<FromEHT>;
};

}
