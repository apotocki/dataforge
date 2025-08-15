/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "utf7.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class utf7_to_utf16
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf7_to_utf16<ErrorHandlerT>, char, char16_t, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char32_t cache : 20;
    char32_t bit : 5;
    char32_t mode : 3;
    char32_t opt : 1;

public:
    template <typename SomeErrorHandlerT>
    utf7_to_utf16(utf7_qrk<ErrorHandlerT> const& quark, utf16_qrk<SomeErrorHandlerT> const&)
        : base_t{ quark }, cache{0}, bit{0}, mode{0}, opt{0}
    {}

    template <SpanOf<char> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    template <typename ConsumerT>
    void push(char val, ConsumerT cons)
    {
        switch (mode)
        {
        case 0:
            if (val != UTF7SHIFT_IN) {
                unsigned char ctype = utf7_matrix[(int)val];
                if (ctype == 1 || (!!opt && (ctype == 2))) {
                    cons(static_cast<char16_t>(val));
                } else {
                    on_error("bad input utf7 character", val, *this);
                }
                return;
            }
            mode = 1;
            return;
        case 1:
            if (val != UTF7SHIFT_OUT) {
                unsigned char v = base64_matrix[static_cast<unsigned char>(val)];
                if (v == 0xff) {
                    on_error("bad input utf7 character", val, *this);
                    return;
                }
                cache <<= 6;
                cache |= v;
                bit += 6;
                mode = 2;
            } else {
                cons(static_cast<char16_t>(UTF7SHIFT_IN));
                mode = 0;
            }
            return;
        case 2: // getting first char16_t character
            unsigned char v = base64_matrix[static_cast<unsigned char>(val)];
            if (v == 0xff) {
                if (val == UTF7SHIFT_OUT && bit < 6) { // getting unshifting character
                    bit = 0;
                    mode = 0;
                    return;
                }
                on_error("bad input utf7 character", val, *this);
                return;
            }

            cache <<= 6;
            cache |= v;
            bit += 6;

            if (bit >= 16) {
                bit -= 16;
                cons(static_cast<char16_t>((cache >> bit) & 0xFFFF));
            }
            return;
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT)
    {
        cache = 0; bit = 0; mode = 0;
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf7_qrk<FromEHT>, utf16_qrk<ToEHT>>
{
    using type = utf7_to_utf16<FromEHT>;
};

}
