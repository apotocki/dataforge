/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class z85_to_bytes_pusher : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    uint_least32_t tuple;
    int_least32_t count;

    std::span<const unsigned char, 256> matrix;

    constexpr static uint_least32_t powers[5] = { 85 * 85 * 85 * 85, 85 * 85 * 85, 85 * 85, 85, 1 };

public:
    using input_element_type = char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> DestT>
    z85_to_bytes_pusher(z85_qrk<ErrorHandlerT> const& srctag, DestT const&)
        : base_t{ srctag }
        , tuple{0}, count{0}, matrix{ srctag.matrix }
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
        
        unsigned char cv = matrix[(unsigned char)c];
        if (cv == 0xff) {
            on_error("Unexpected character", c, *this);
            return;
        }

        tuple += cv * powers[count++];

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
struct cvt_resolver<z85_qrk<FromEHT>, ToQuarkT>
{
    using type = z85_to_bytes_pusher<FromEHT>;
};

}
