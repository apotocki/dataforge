/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class ascii85_to_bytes_pusher : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    uint_least32_t tuple;
    int_least32_t count : 16;
    int_least32_t y_abbr : 1;

    constexpr static uint_least32_t powers[5] = { 85 * 85 * 85 * 85, 85 * 85 * 85, 85 * 85, 85, 1 };

public:
    using input_element_type = char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> DestT>
    ascii85_to_bytes_pusher(ascii85_qrk<ErrorHandlerT> const& srctag, DestT const&)
        : base_t{ srctag }
        , tuple{0}, count{0}, y_abbr{static_cast<int_least32_t>(srctag.y_abbr ? 1 : 0)}
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
        if (c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r') return;
        
        if (!count) {
            if (c == 'z') {
                unsigned char buff[4] = { 0, 0, 0, 0 };
                cons(std::span{ buff });
                return;
            }
            if (c == 'y' && y_abbr) {
                unsigned char buff[4] = { 0x20, 0x20, 0x20, 0x20 };
                cons(std::span{ buff });
                return;
            }
        }
        if (c < '!' || c > 0x75) {
            on_error("Unexpected character", c, *this);
            return;
        }

        tuple += (c - '!') * powers[count++];

        if (count < 5) return;

        if constexpr (std::endian::native == std::endian::big && sizeof(uint_least8_t) * 4 == sizeof(uint_least32_t)) {
            cons(std::span<const unsigned char, 4>{ reinterpret_cast<const unsigned char*>(&tuple), 4 });
        } else {
            for (int i = 0; i < 4; ++i) {
                cons(static_cast<unsigned char>(tuple >> ((3 - i) * 8)));
            }
        }
        reset();
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        if (count) {
            tuple += powers[count - 1];
        }
        for (int i = 1; i < count; ++i) {
            cons(static_cast<unsigned char>(tuple >> ((4 - i) * 8)));
        }
    }

    inline void reset() noexcept
    {
        tuple = 0;
        count = 0;
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<ascii85_qrk<FromEHT>, ToQuarkT>
{
    using type = ascii85_to_bytes_pusher<FromEHT>;
};

}
