/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

class bytes_to_base16_pusher
    : public generic_pusher<void>
    , public basic_puller<bytes_to_base16_pusher, unsigned char, char, 8>
{
    const char* alphabet;
    
public:
    template <IntegralBasedQuark<8> SrcTagT, typename ErrorHandlerT>
    bytes_to_base16_pusher(SrcTagT const&, base16_qrk<ErrorHandlerT> const& desttag)
        : alphabet{ desttag.ulet ? "0123456789ABCDEF" : "0123456789abcdef" }
    {}

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT c, ConsumerT cons)
    {
        const char elems[2] = { alphabet[(c >> 4) & 0x0f], alphabet[c & 0x0f] };
        cons(std::span{ elems, 2 });
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons) noexcept {}

    inline void reset() noexcept {}
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, base16_qrk<ToEHT>>
{
    using type = bytes_to_base16_pusher;
};

}
