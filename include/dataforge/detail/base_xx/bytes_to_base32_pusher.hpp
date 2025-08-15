/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

class bytes_to_base32_pusher : public generic_pusher<void>
{
    const std::span<const char> alphabet;
    uint16_t bit : 4;
    uint16_t cache : 12;
    char padding_character;

public:
    using input_element_type = unsigned char;
    using output_element_type = char;

    template <typename SrcTagT, typename ErrorHandlerT>
    bytes_to_base32_pusher(SrcTagT const&, base32_qrk<ErrorHandlerT> const& desttag)
        : alphabet{ desttag.alphabet }, bit{ 0 }, cache{ 0 }, padding_character { desttag.padding_character }
    {}

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT c, ConsumerT cons)
    {
        cache <<= 8;
        cache |= static_cast<unsigned char>(c);
        bit += 3;
        cons(alphabet[(cache >> bit) & 0x1F]);

        if (bit >= 5) {
            bit -= 5;
            cons(alphabet[(cache >> bit) & 0x1F]);
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        if (!bit) return;
        cache <<= 8;
        bit += 3;
        cons(alphabet[(cache >> bit) & 0x1F]);

        if (!padding_character) {
            reset();
            return;
        }
        while (bit) {
            if (bit >= 5) {
                cons(padding_character);
                bit -= 5;
            } else {
                bit += 8;
            }
        }
    }

    inline void reset() noexcept
    {
        bit = 0;
        cache = 0;
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, base32_qrk<ToEHT>>
{
    using type = bytes_to_base32_pusher;
};

}
