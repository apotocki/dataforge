/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <utility>

#include "md6.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class bytes_to_md6_pusher 
    : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    using conf_t = md6_detail::md6_conf<>;
    using md6_state = md6_detail::md6_state_general<conf_t>;
    md6_state state_;

    int bitsize, rounds, l;
    std::span<const uint_least8_t> key;

public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> SrcTagT>
    bytes_to_md6_pusher(SrcTagT const&, md6_qrk<ErrorHandlerT> const& quark)
        : base_t{ quark }, bitsize{ quark.bitsize }, rounds{ quark.rounds }, l{ quark.l }, key{ quark.key }
    {
        reset();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&&)
    {
        using namespace md6_detail;
        md6_error_code code = md6_update(&state_, ivals.data(), ivals.size() * 8);
        if (code != md6_error_code::MD6_SUCCESS) {
            on_error("md6_update error", code, *this);
        }
    }

    template <Integral<8> LEIT, typename ConsumerT>
    inline void push(const LEIT ival, ConsumerT&& cons)
    {
        push(std::span<const LEIT, 1>(&ival, 1), nullptr);
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        using namespace md6_detail;
        md6_error_code code = md6_final(&state_, nullptr);
        if (code != md6_error_code::MD6_SUCCESS) {
            on_error("md6_final error", code, *this);
        }
        cons(std::span<const unsigned char>{ state_.hashval, static_cast<size_t>((state_.d + 7) / 8) });
        reset();
    }

    void reset()
    {
        using namespace md6_detail;
        md6_error_code code = md6_init(&state_, bitsize, rounds, key, l);
        if (code != md6_error_code::MD6_SUCCESS) {
            on_error("md6_init error", code, *this);
        }
        finished = false;
    }


    bool finished = false;

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        if (finished) return {};
        if (input.empty()) {
            input = span_cast<const input_element_type>(p());
        }
        for (;;) {
            if (input.empty()) {
                using namespace md6_detail;
                md6_error_code code = md6_final(&state_, nullptr);
                if (code != md6_error_code::MD6_SUCCESS) {
                    on_error("md6_final error", code, *this);
                }
                finished = true;
                return std::span<const unsigned char>{ state_.hashval, static_cast<size_t>((state_.d + 7) / 8) };
            }
            push(input, nullptr);
            input = span_cast<const input_element_type>(p());
        }
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, md6_qrk<ToEHT>>
{
    using type = bytes_to_md6_pusher<ToEHT>;
};

}
