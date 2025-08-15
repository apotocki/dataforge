/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ErrorHandlerT>
class base32_to_bytes_pusher : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    std::span<const uint8_t> matrix;
    std::vector<uint8_t> matrix_generated_;

    uint16_t bit : 4;
    uint16_t cache : 12;

    char padding_character;

public:
    using input_element_type = char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> DestT>
    base32_to_bytes_pusher(base32_qrk<ErrorHandlerT> const& srctag, DestT const&)
        : base_t{ srctag }
        , matrix{ srctag.matrix }
        , bit{ 0 }, cache{ 0 }
        , padding_character{ srctag.padding_character }
    {
        if (matrix.empty()) {
            matrix_generated_.resize(256, 0xff);
            for (uint8_t i = 0; i < 32; ++i) {
                matrix_generated_[srctag.alphabet[i]] = i;
            }
            matrix = matrix_generated_;
        }
    }

    base32_to_bytes_pusher(base32_to_bytes_pusher const&) = delete;
    base32_to_bytes_pusher(base32_to_bytes_pusher&&) = default;

    base32_to_bytes_pusher& operator=(base32_to_bytes_pusher const&) = delete;
    base32_to_bytes_pusher& operator=(base32_to_bytes_pusher&&) = default;

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT c, ConsumerT && cons)
    {
        if (c == padding_character) {
            if (bit % 8 == 0) {
                on_error("unexpected character", c, *this);
            } else {
                bit += 5;
            }
            return;
        }

        unsigned char v = matrix[static_cast<unsigned char>(c)];
        
        if (v > 31) {
            if (v == 0xfe) return;
            on_error("unexpected character", c, *this);
            return;
        } else {
            cache <<= 5;
            cache |= v;
        }

        if (bit > 2) {
            bit -= 3;
            cons(static_cast<uint8_t>(cache >> bit));
        } else {
            bit += 5;
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT)
    {
        if (padding_character && bit % 8 != 0) {
            on_error("EOF, but the padding character is expected.", *this);
        }
        reset();
    }

    inline void reset() noexcept
    {
        bit = 0;
        cache = 0;
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<base32_qrk<FromEHT>, ToQuarkT>
{
    using type = base32_to_bytes_pusher<FromEHT>;
};

}
