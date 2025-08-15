/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <vector>

namespace dataforge {

template <std::integral BufferElementT, std::integral DblBufferElementT>
class bytes_to_generic_base_pusher : public generic_pusher<void>
{
    static_assert(2 * sizeof(BufferElementT) == sizeof(DblBufferElementT));

protected:
    const std::span<const char> alphabet;
    std::vector<BufferElementT> input;
    std::vector<char> output;

    uint_least8_t head_state: 1;
    uint_least8_t reserved : 1;
    uint_least8_t input_byte_offset: 6;

public:
    using input_element_type = unsigned char;
    using output_element_type = char;

    template <IntegralBasedQuark<8> SrcTagT, typename ErrorHandlerT>
    bytes_to_generic_base_pusher(SrcTagT const&, base_qrk<ErrorHandlerT> const& desttag)
        : alphabet{ desttag.alphabet }, head_state{ 1 }, reserved{ 0 }, input_byte_offset{ 0 }
    {}

    inline void push_element(BufferElementT e)
    {
        if constexpr (sizeof(BufferElementT) > 1) {
            e &= 0xff;
            //e <<= 8 * input_byte_offset;
            if (!input_byte_offset) {
                input.push_back(e);
            } else {
                input.back() = (input.back() << 8) | e;
            }
            ++input_byte_offset;
            if (input_byte_offset == sizeof(BufferElementT)) {
                input_byte_offset = 0;
            }
        } else {
            input.push_back(static_cast<BufferElementT>(e));
        }
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ispan, ConsumerT cons)
    {
        auto ivals = std::span{ ispan.data(), ispan.size() }; // to ensure subspan availability
        if (head_state) {
            while (!ivals.empty() && !ivals.front()) {
                cons(alphabet.front());
                ivals = ivals.subspan(1);
            }
            if (ivals.empty()) return;
            head_state = 0;
        }
        // to do: optimization for big endian platform?
        if constexpr (sizeof(BufferElementT) > 1) {
            for (auto ival : ivals) {
                push_element(ival);
            }
        } else {
            input.insert(input.end(), ivals.begin(), ivals.end());
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        if (head_state) {
            if (!ival) {
                cons(alphabet.front());
                return;
            } else {
                head_state = 0;
            }
        }
        push_element(static_cast<BufferElementT>(ival));
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        if (!input_byte_offset) input_byte_offset = sizeof(BufferElementT);
        std::span<BufferElementT> in = input;
        while (!in.empty()) {
            DblBufferElementT carry = 0;
            auto it = in.begin();
            for (auto eit = in.end() - 1; it != eit; ++it) {
                carry <<= 8 * sizeof(BufferElementT);
                carry += *it;
                *it = static_cast<BufferElementT>(carry / alphabet.size());
                carry = carry % alphabet.size();
            }
            carry <<= 8 * input_byte_offset;
            carry += *it;
            *it = static_cast<BufferElementT>(carry / alphabet.size());
            output.push_back(static_cast<char>(carry % alphabet.size()));
            if (!in.front()) in = in.subspan(1);
        }
        for (auto rit = output.rbegin(), reit = output.rend(); rit != reit; ++rit) {
            cons(alphabet[static_cast<unsigned char>(*rit)]);
        }

        reset();
    }

    void reset()
    {
        output.clear();
        input_byte_offset = 0;
        input.clear();
        head_state = 1;
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, base_qrk<ToEHT>>
{
    using type = bytes_to_generic_base_pusher<uint_least32_t, uint_least64_t>;
    //using type = byte_to_generic_base_pusher<uint_least16_t, uint_least32_t>;
    //using type = byte_to_generic_base_pusher<uint_least8_t, uint_least16_t>;
};

}
