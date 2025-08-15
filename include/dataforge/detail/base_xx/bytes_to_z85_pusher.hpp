/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

class bytes_to_z85_pusher : public generic_pusher<void>
{
    uint_least32_t tuple;
    int_least32_t count;
    
    const std::span<const char, 85> alphabet;

public:
    using input_element_type = unsigned char;
    using output_element_type = char;

    template <IntegralBasedQuark<8> SrcTagT, typename ErrorHandlerT>
    bytes_to_z85_pusher(SrcTagT const&, z85_qrk<ErrorHandlerT> const& desttag)
        : tuple{0}, count{0}, alphabet{ desttag.alphabet }
    {}

    template <typename ConsumerT>
    inline void put_tuple(ConsumerT cons)
    {
        char data[5];
        for (int i = 0; i < 5; ++i) {
            data[i] = tuple % 85;
            tuple /= 85;
        }
        int_least32_t lim = 4 - count;
        for (int_least32_t i = 4; i >= lim; --i) {
            cons(alphabet[data[i]]);
        }
    }

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
        tuple |= ((uint_least32_t)(unsigned char)c) << ((3 - count++) * 8);
        if (count < 4) return;

        put_tuple(cons);

        reset();
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons) noexcept
    {
        if (count) {
            put_tuple(cons);
        }
    }

    inline void reset() noexcept
    {
        tuple = 0;
        count = 0;
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, z85_qrk<ToEHT>>
{
    using type = bytes_to_z85_pusher;
};

}
