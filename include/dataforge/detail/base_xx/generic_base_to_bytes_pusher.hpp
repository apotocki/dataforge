/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <vector>

namespace dataforge {

template <std::integral BufferElementT, std::integral DblBufferElementT, typename ErrorHandlerT>
class generic_base_to_bytes_pusher : public generic_pusher<ErrorHandlerT>
{
    static_assert(2 * sizeof(BufferElementT) == sizeof(DblBufferElementT));

protected:
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    DblBufferElementT base_val;
    std::span<const uint8_t> matrix;
    std::vector<uint8_t> input;
    std::vector<BufferElementT> output;
    std::vector<uint8_t> matrix_generated_;

    uint32_t zeros_{0};
    const char zpad_char_;
    uint8_t head_state: 1;
    //uint8_t reserved : 1;

public:
    using input_element_type = char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> DestT>
    generic_base_to_bytes_pusher(base_qrk<ErrorHandlerT> const& srctag, DestT const&)
        : base_t{ srctag }
        , base_val { static_cast<DblBufferElementT>(srctag.alphabet.size()) }
        , matrix{ srctag.matrix }
        , zpad_char_{ srctag.alphabet[0] }, head_state{ 1 }
    {
        if (matrix.empty()) {
            matrix_generated_.resize(256, 0xff);
            for (uint8_t i = 0; i < base_val; ++i) {
                matrix_generated_[srctag.alphabet[i]] = i;
            }
            matrix = matrix_generated_;
        }
    }

    generic_base_to_bytes_pusher(generic_base_to_bytes_pusher const&) = delete;
    generic_base_to_bytes_pusher(generic_base_to_bytes_pusher&&) = default;

    generic_base_to_bytes_pusher& operator=(generic_base_to_bytes_pusher const&) = delete;
    generic_base_to_bytes_pusher& operator=(generic_base_to_bytes_pusher&&) = default;

    inline void push_element(uint8_t ival)
    {
        uint8_t val = matrix[ival];
        if (val == 0xff) {
            on_error("wrong character", ival, *this);
        }
        input.push_back(val);
    }

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ispan, ConsumerT cons)
    {
        auto ivals = std::span { ispan.data(), ispan.size() }; // to ensure subspan availability
        if (head_state) {
            while (!ivals.empty() && ivals.front() == zpad_char_) {
                cons(0);
                ivals = ivals.subspan(1);
                ++zeros_;
            }
            if (ivals.empty()) return;
            head_state = 0;
        }

        for (auto ival : ivals) {
            push_element(ival);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        if (head_state) {
            if (ival == zpad_char_) {
                cons(0);
                ++zeros_;
                return;
            } else {
                head_state = 0;
            }
        }
        push_element(ival);
    }

    void handle_input()
    {
        constexpr DblBufferElementT mask = (static_cast<DblBufferElementT>(1) << 8 * sizeof(BufferElementT)) - 1;
        for (uint8_t v : input) {
            for (auto it = output.begin(), eit = output.end(); it != eit; ++it) {
                DblBufferElementT t = ((DblBufferElementT)*it) * base_val + v;
                v = static_cast<uint8_t>(t >> 8 * sizeof(BufferElementT));
                *it = t & mask;
            }
            if (v) {
                output.push_back(v);
            }
        }
    }

    template <typename ConsumerT>
    void finish(ConsumerT && cons)
    {
        handle_input();
        
        auto bit = output.begin(), it = output.end();
        if (bit != it) {
            --it;
            finish_impl(std::move(bit), std::move(it), std::forward<ConsumerT>(cons));
        }
        reset();
    }

    template <typename IteratorT, typename ConsumerT>
    void finish_impl(IteratorT bit, IteratorT it, ConsumerT && cons)
    {
        /*
        if (!*it) {
            if (bit == it) return;
            --it;
        }
        */
        bool is_head = true;
        for (int i = sizeof(BufferElementT) - 1; i >= 0; --i) {
            BufferElementT e = (*it >> (i * 8)) & 0xff;
            if (is_head) {
                if (!e) continue;
                is_head = false;
            }
            cons(static_cast<char>(e));
        }

        while (it != bit) {
            --it;
            if constexpr (std::endian::native == std::endian::big) {
                cons(std::span{ reinterpret_cast<const char*>(&*it), sizeof(BufferElementT) });
            } else {
                for (int i = sizeof(BufferElementT) - 1; i >= 0; --i) {
                    cons(static_cast<char>((*it >> (i * 8)) & 0xff));
                }
            }
        }
    }

    void reset()
    {
        output.clear();
        input.clear();
        head_state = 1;
        zeros_ = 0;
    }
};

template <typename FromEHT, typename ToEHT>
struct cvt_resolver<base_qrk<FromEHT>, int_qrk<8, ToEHT>>
{
    using type = generic_base_to_bytes_pusher<uint_least32_t, uint_least64_t, FromEHT>;
    //using type = generic_base_to_bytes_pusher<uint_least16_t, uint_least32_t, FromEHT>;
    //using type = generic_base_to_bytes_pusher<uint_least8_t, uint_least16_t, FromEHT>;
};

}
