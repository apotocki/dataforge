/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "lz4.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class lz4_extractor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    lz4_detail::state* state_;
    
public:
    template <IntegralBasedQuark<8> DestT>
    lz4_extractor(lz4_qrk<ErrorHandlerT> const& qrk, DestT const&)
        : base_t{ qrk }
    {
        state_ = lz4_detail::state::create(qrk, false);

        size_t ret = LZ4F_createDecompressionContext(&state_->ectx_, LZ4F_VERSION);

        if (LZ4F_isError(ret)) {
            on_error("lz4 extractor initialization error", ret, *this);
        }
    }

    lz4_extractor(lz4_extractor const&) = delete;
    lz4_extractor(lz4_extractor&& rhs)
        : base_t{ std::move(static_cast<base_t&>(rhs)) }
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    lz4_extractor& operator=(lz4_extractor const&) = delete;
    lz4_extractor& operator=(lz4_extractor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_extractor();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
    }

    ~lz4_extractor()
    {
        if (state_) state_->destroy_extractor();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        auto indata = span_cast<const unsigned char>(ivals);
        do {
            size_t ret = state_->extract(indata);
            if (LZ4F_isError(ret)) {
                on_error("lz4 extraction failed", ret, *this);
            } else {
                cons(state_->detach_out_span());
            }
        } while (!indata.empty());
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT && cons)
    {
        uint_least8_t val[1] = { static_cast<uint_least8_t>(ival) };
        push(std::span{val}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        if (state_->avail_out) {
            cons(state_->out_span());
        }
        reset();
    }

    inline void reset() noexcept
    {
        state_->reset([this](const char* errstr, size_t r) { on_error(errstr, r, *this); });
    }

    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                if (state_->processing_finished_) return {};
                input = span_cast<const unsigned char>(p());
            }
            
            size_t ret = state_->extract(input);
            if (LZ4F_isError(ret)) {
                on_error("lz4 extraction failed", ret, *this);
            } else {
                if (!ret) state_->processing_finished_ = 1;
                return state_->detach_out_span();
            }
        }
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<lz4_qrk<FromEHT>, ToQuarkT>
{
    using type = lz4_extractor<FromEHT>;
};

}
