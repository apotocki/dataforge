/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "utf7.hpp"
#include "utf16.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class utf16_to_utf7
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<utf16_to_utf7<ErrorHandlerT>, char16_t, char, 8>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    char32_t cache : 20;
    char32_t bit : 5;
    char32_t shifting : 1;
    char32_t opt : 1;

    char16_t wcached;

public:
    template <typename SomeErrorHandlerT>
    explicit utf16_to_utf7(utf16_qrk<SomeErrorHandlerT> const& quark, utf7_qrk<ErrorHandlerT> const&)
        : base_t{ quark }, cache{ 0 }, bit{ 0 }, shifting{ 0 }, opt{ 0 }, wcached{ 0 }
    {}

    template <SpanOf<char16_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        for (auto c : chars) {
            push(c, cons);
        }
    }

    inline char wc_pop(char16_t wc)
    {
        assert(bit <= 4);
        cache <<= 16;
        cache |= (static_cast<char32_t>(wc) & 0xFFFF);
        bit += 10;
        return base64_alphabet[(cache >> bit) & 0x3F];
    }

    template <typename ConsumerT>
    void push(char16_t val, ConsumerT cons)
    {
        if (!wcached) {
            if (val < 128) {
                unsigned char ctype = utf7_matrix[val];
                if (ctype == 1 || (!!opt && (ctype == 2))) {
                    if (!shifting) {
                        cons(static_cast<char>(val));
                        return;
                    } else if (bit > 0) {
                        assert(bit > 0 && bit <= 4);
                        cache <<= (6 - bit);
                        bit = 0;
                        cons(base64_alphabet[cache & 0x3F]);
                    }
                    const char elems[] = { UTF7SHIFT_OUT, static_cast<char>(val) };
                    cons(std::span{elems, 2});
                    shifting = 0;
                    return;
                } else if (!shifting) {
                    cons(UTF7SHIFT_IN);
                    if (val == UTF7SHIFT_IN) {
                        cons(UTF7SHIFT_OUT);
                        return;
                    }
                }
            } else if (val < UTF16_SUR_HIGH_START || val > UTF16_SUR_LOW_END) {
                if (!shifting) { cons(UTF7SHIFT_IN); }
            } else if (val <= UTF16_SUR_HIGH_END) { // found high surrogate
                wcached = val;
                shifting = 1;
                return;
            } else {
                on_error("wrong utf16 character (unexpected low surrogate)", val, *this);
                return;
            }

            cons(wc_pop(val));
            while (bit >= 6) {
                bit -= 6;
                cons(base64_alphabet[(cache >> bit) & 0x3F]);
            }
            shifting = 1;
        } else if (val >= UTF16_SUR_LOW_START && val <= UTF16_SUR_LOW_END) {
            cons(wc_pop(wcached));
            while (bit >= 6) {
                bit -= 6;
                cons(base64_alphabet[(cache >> bit) & 0x3F]);
            }
            cons(wc_pop(val));
            while (bit >= 6) {
                bit -= 6;
                cons(base64_alphabet[(cache >> bit) & 0x3F]);
            }
            wcached = 0;
        } else {
            on_error("Wrong utf16 character (low surrogate is expected)", val, *this);
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        if (wcached) {
            on_error("Not all input was converted before unshifting.", *this);
        }
        if (bit > 0) {
            assert(bit <= 4);
            cache <<= (6 - bit);
            bit = 0;
            cons(base64_alphabet[cache & 0x3F]);
        }
        if (shifting) {
            cons(UTF7SHIFT_OUT);
            shifting = 0;
        }
    }

    void reset() noexcept
    {
        bit = 0;
        shifting = 0;
        wcached = 0;
        cache = 0;
        opt = 0;
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<utf16_qrk<FromEHT>, utf7_qrk<ToEHT>>
{
    using type = utf16_to_utf7<ToEHT>;
};

}
