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
class utf32_to_utf16
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf32_to_utf16<ErrorHandlerT>, char32_t, char16_t, 2>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

public:
    template <typename ErrorHandlerT2>
    explicit utf32_to_utf16(utf32_qrk<ErrorHandlerT> const& quark, utf16_qrk<ErrorHandlerT2> const&)
        : base_t{ quark }
    {}

    template <SpanOf<char32_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <typename ConsumerT>
    void push(char32_t uchar, ConsumerT cons)
    {
        if (uchar < UTF16_SUR_HIGH_START || (uchar > UTF16_SUR_LOW_END && uchar <= 0xffff)) {
            cons(static_cast<char16_t>(uchar));
        } else {
            if (uchar >= 0x00010000UL && uchar <= 0x0010ffff) {
                const char16_t elems[] = {
                    static_cast<char16_t>(UTF16_LEAD_OFFSET + (uchar >> 10)),
                    static_cast<char16_t>(UTF16_SUR_LOW_START + (uchar & 0x3FF))
                };
                cons(std::span{ elems, 2 });
            } else {
                on_error("wrong utf32 character", uchar);
            }
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept {}
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf32_qrk<FromEHT>, utf16_qrk<ToEHT>>
{
    using type = utf32_to_utf16<FromEHT>;
};

}
