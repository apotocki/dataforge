/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "base64.hpp"

namespace dataforge {

class bytes_to_base64
    : public generic_pusher<void>
    , public basic_puller<bytes_to_base64, char, char, 3>
{
    uint_least16_t bit : 4;
    uint_least16_t cache : 12;

    char paddingCV;

public:
    template <IntegralBasedQuark<8> SrcTagT, typename ErrorHandlerT>
    bytes_to_base64(SrcTagT const&, base64_qrk<ErrorHandlerT> const& desttag)
        : bit{ 0 }, cache{ 0 }, paddingCV{ desttag.paddingCV }
    {}

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    auto push(const LEIT ival, ConsumerT cons)
    {
        assert(bit <= 4);
        cache <<= 8;
        cache |= static_cast<unsigned char>(ival);
        bit += 2;

        char elems[2] = { base64_alphabet[(cache >> bit) & 0x3F] };
        
        if (bit == 6) {
            bit = 0;
            elems[1] = base64_alphabet[cache & 0x3F];
            return cons(std::span{ elems, 2 });
        } else {
            return cons(elems[0]);
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        assert(bit < 5);
        if (bit == 4) {
            cache <<= 8;
            const char elems[] = { base64_alphabet[(cache >> 6) & 0x3F], paddingCV };
            bit = 0;
            cons(std::span{ elems, 2 });
        } else if (bit == 2) {
            cache <<= 8;
            const char elems[] = { base64_alphabet[(cache >> 4) & 0x3F], paddingCV, paddingCV };
            bit = 0;
            cons(std::span{ elems, 3 });
        }
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, base64_qrk<ToEHT>>
{
    using type = bytes_to_base64;
};

}
