/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class utf32_to_utf8
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf32_to_utf8<ErrorHandlerT>, char32_t, char8_t, 4>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

public:
    template <typename ErrorHandlerT2>
    explicit utf32_to_utf8(utf32_qrk<ErrorHandlerT> const& quark, utf8_qrk<ErrorHandlerT2> const&)
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
        if (uchar <= 0x007F) {
            cons(static_cast<char8_t>(uchar));
        } else if (uchar <= 0x07FF) {
            char8_t c1 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            const char8_t elems[] = { static_cast<char8_t>((uchar & 0x1F) | 0xC0), c1 };
            cons(std::span{ elems, 2 });
        } else if (uchar <= 0xFFFF) {
            char8_t c2 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            char8_t c1 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            const char8_t elems[] = { static_cast<char8_t>((uchar & 0x0F) | 0xE0), c1, c2 };
            cons(std::span{ elems, 3 });
        } else if (uchar <= 0x10FFFF) {
            char8_t c3 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            char8_t c2 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            char8_t c1 = static_cast<char8_t>((uchar & 0x3F) | 0x80); uchar >>= 6;
            const char8_t elems[] = { static_cast<char8_t>((uchar & 0x07) | 0xF0), c1, c2, c3 };
            cons(std::span{ elems, 4 });
        } else {
            on_error("wrong utf32 character", uchar);
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept {}
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf32_qrk<FromEHT>, utf8_qrk<ToEHT>>
{
    using type = utf32_to_utf8<FromEHT>;
};

}
