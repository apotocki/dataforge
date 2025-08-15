/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class utf8_to_utf32
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf8_to_utf32<ErrorHandlerT>, char8_t, char32_t, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char32_t num : 2;
    char32_t data : 30;

public:
    template <typename SomeErrorHandlerT>
    explicit utf8_to_utf32(utf8_qrk<ErrorHandlerT> const& quark, utf32_qrk<SomeErrorHandlerT> const&)
        : base_t{ quark }, num{ 0 }, data{ 0 }
    {}

    void reset()
    {
        data = num = 0;
    }

    template <CompatibleSpan<char8_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <typename ConsumerT>
    void push(char8_t c, ConsumerT cons)
    {
        char32_t c32 = c & 0xff;
        if (!num) {
            if (c32 < 0x80) {
                return cons(c32);
            } else if ((c32 & 0xe0) == 0xc0) {
                if (c32 != 0xc1 && c32 != 0xc0) {
                    num = 1; data = c32 & 0x1f;
                }
            } else if ((c32 & 0xf0) == 0xe0) {
                num = 2; data = c32 & 0x0f;
            } else if ((c32 & 0xf8) == 0xf0 && c32 < 0xf5) {
                num = 3; data = c32 & 0x07;
            } else {
                on_error("wrong utf8 character", c, *this);
            }
        } else if ((c32 & 0xc0) == 0x80) {
            data <<= 6;
            data |= (c32 & 0x3f);
            if (!--num) {
                return cons(data);
            }
        } else {
            on_error("wrong utf8 character", c, *this);
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT&&)
    {
        if (num) {
            on_error("unfinished utf8 sequence", *this);
        }
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf8_qrk<FromEHT>, utf32_qrk<ToEHT>>
{
    using type = utf8_to_utf32<FromEHT>;
};

}
