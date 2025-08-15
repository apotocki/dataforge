/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class base16_to_bytes
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<base16_to_bytes<ErrorHandlerT>, char, char, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    uint_least8_t cache;

public:
    template <typename DestT>
    base16_to_bytes(base16_qrk<ErrorHandlerT> const& srctag, DestT&&)
        : base_t{ srctag }
        , cache{0}
    {}

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT c, ConsumerT cons)
    {
        uint_least8_t r;
        if (c >= '0' && c <= '9') {
            r = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            r = c - 'A' + 0x0a;
        } else if (c >= 'a' && c <= 'f') {
            r = c - 'a' + 0x0a;
        } else {
            on_error("unexpected character", c, *this);
            return;
        }

        if (!cache) {
            cache = (r << 4) | 0x0f;
        } else {
            cons(static_cast<char>((cache & 0xF0) | r));
            cache = 0;
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT)
    {
        if (cache) {
            on_error("unfinished base16 sequence", *this);
        }
    }

    inline void reset() noexcept
    {
        cache = 0;
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<base16_qrk<FromEHT>, ToQuarkT>
{
    using type = base16_to_bytes<FromEHT>;
};

}
