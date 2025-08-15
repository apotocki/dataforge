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
class utf8_to_utf16
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf8_to_utf16<ErrorHandlerT>, char8_t, char16_t, 2>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char32_t num : 2;
    char32_t data : 30;

public:
    template <typename SomeErrorHandlerT>
    explicit utf8_to_utf16(utf8_qrk<ErrorHandlerT> const& quark, utf16_qrk<SomeErrorHandlerT> const&)
        : base_t{ quark }, num{ 0 }, data{ 0 }
    {}

    template <CompatibleSpan<char8_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <Integral<8> CT, typename ConsumerT>
    void push(const CT val, ConsumerT cons)
    {
        char16_t c = static_cast<char16_t>(val) & 0xff;

        if (!num) {
            if (c < 128) {
                cons(c);
                return;
            } else if ((c & 0xe0) == 0xc0) {
                if (c != 0xc1 && c != 0xc0)
                {
                    num++; data = c & 0x1f; return;
                }
            } else if ((c & 0xf0) == 0xe0) {
                num += 2; data = c & 0x0f; return;
            } else if ((c & 0xf8) == 0xf0 && c < 0xf5) {
                num += 3; data = c & 0x07; return;
            }
        } else if ((c & 0xc0) == 0x80) {
            data <<= 6;
            data |= (c & 0x3f);
            if (!--num) {
                if (data < UTF16_SUR_HIGH_START || data > UTF16_SUR_LOW_END) {
                    if (data <= 0xFFFF) {
                        //if (data != UTF16_BOM) {
                        cons(static_cast<char16_t>(data));
                        //}
                    } else {
                        char16_t elems[] = {
                            static_cast<char16_t>(UTF16_LEAD_OFFSET + (data >> 10)),
                            static_cast<char16_t>(UTF16_SUR_LOW_START + (data & 0x3FF))
                        };
                        cons(std::span{ elems, 2 });;
                    }
                } else {
                    on_error("wrong decoded utf16 value", *this);
                }
            }
            return;
        }
        on_error("wrong utf8 character", val, *this);
        num = 0;
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
struct cvt_resolver<utf8_qrk<FromEHT>, utf16_qrk<ToEHT>>
{
    using type = utf8_to_utf16<FromEHT>;
};

}
