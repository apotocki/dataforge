/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#pragma once

#include "../quarks.hpp"

namespace dataforge {

template <typename ImplT, typename ErrorHandlerT = void>
class digest_generic_pusher 
    : protected ImplT
    , protected generic_pusher<ErrorHandlerT>
{
    static constexpr size_t digest_word_byte_count = ImplT::digest_word_bit_count / 8;

public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    using generic_base_t = digest_generic_pusher;
    digest_generic_pusher() = default;

    template <IntegralBasedQuark<8> SrcTagT, typename DestTagT>
    requires (std::is_default_constructible_v<ImplT> && !std::is_constructible_v<ImplT, DestTagT const&>)
    digest_generic_pusher(SrcTagT const&, DestTagT const& dt)
        : generic_pusher<ErrorHandlerT>{ dt }
    {}

    template <IntegralBasedQuark<8> SrcTagT, typename DestTagT>
    requires (std::is_constructible_v<ImplT, DestTagT const&>)
    digest_generic_pusher(SrcTagT const&, DestTagT const& dt)
        : ImplT{ dt }
        , generic_pusher<ErrorHandlerT>{ dt }
    {}

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&&)
    {
        ImplT::input(ivals.data(), ivals.size());
    }

    template <Integral<8> LEIT, typename ConsumerT>
    inline void push(const LEIT ival, ConsumerT&&)
    {
        ImplT::input(&ival, 1);
    }

    template <typename ConsumerT>
    void finish(ConsumerT && cons)
    {
        do_finish(std::forward<ConsumerT>(cons));
        ImplT::reset();
        finished = 0;
    }

    void reset()
    {
        ImplT::reset();
        finished = 0;
    }

    size_t finished = 0;
    output_element_type cached;

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        if constexpr (digest_word_byte_count == sizeof(typename ImplT::digest_word_type)) {
            if (finished) return {};
            if (input.empty()) {
                input = span_cast<const input_element_type>(p());
            }
        } else {
            if (finished == ImplT::digest_length()) return {};
            if (!finished && input.empty()) {
                input = span_cast<const input_element_type>(p());
            }
        }
        for (;;) {
            if (finished || input.empty()) {
                std::span<const output_element_type> result;
                if constexpr (digest_word_byte_count <= sizeof(typename ImplT::digest_word_type)) {
                    do_finish([&result](auto span) { result = span; });
                    assert(result.size() == ImplT::digest_length());
                } else {
                    if (!finished) { ImplT::finalize(); }
                    if constexpr (ImplT::digest_endianness() == std::endian::big) {
                        cached = static_cast<output_element_type>(ImplT::digest_span()[finished / digest_word_byte_count] >> (ImplT::digest_word_bit_count - 8 * (1 + finished % digest_word_byte_count)));
                    } else {
                        cached = static_cast<output_element_type>(ImplT::digest_span()[finished / digest_word_byte_count] >> (8 * (finished % digest_word_byte_count)));
                    }
                    result = std::span<const output_element_type>{ &cached, 1 };
                }
                finished += result.size();
                return result;
            }
            push(input, nullptr);
            input = span_cast<const input_element_type>(p());
        }
    }

protected:
    template <typename ConsumerT>
    void do_finish(ConsumerT cons)
    {
        ImplT::finalize();
        if constexpr (ImplT::digest_endianness() != std::endian::native || digest_word_byte_count != sizeof(typename ImplT::digest_word_type))
        {
            if constexpr (digest_word_byte_count == sizeof(typename ImplT::digest_word_type)) {
                for (typename ImplT::digest_word_type& w : ImplT::digest_span()) {
                    w = reverse_bytes(w);
                }
            } else {
                for (int i = 0; i < ImplT::digest_length(); ++i) {
                    if constexpr (ImplT::digest_endianness() == std::endian::big) {
                        cons(static_cast<output_element_type>(ImplT::digest_span()[i / digest_word_byte_count] >> (ImplT::digest_word_bit_count - 8 * (1 + i % digest_word_byte_count))));
                    } else {
                        cons(static_cast<output_element_type>(ImplT::digest_span()[i / digest_word_byte_count] >> (8 * (i % digest_word_byte_count))));
                    }
                }
                return;
            }
        }
        cons(std::span{ reinterpret_cast<const output_element_type*>(ImplT::digest_span().data()), ImplT::digest_length() });
    }
};

}
