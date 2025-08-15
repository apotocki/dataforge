/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "bzip2.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class bzip2_extractor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    bzip2_detail::state* state_;
    
public:
    template <IntegralBasedQuark<8> DestT>
    bzip2_extractor(bzip2_qrk<ErrorHandlerT> const& it, DestT const&)
        : base_t{ it }
    {
        initialize(it);
    }

    template <typename QuarkT>
    void initialize(QuarkT const& t)
    {
        state_ = bzip2_detail::state::create(static_cast<unsigned int>(t.buffer_size));

        int ret = BZ2_bzDecompressInit(&state_->strm_, 0, 0);

        if (ret != Z_OK) {
            on_error("bzip2 extractor initialization error", ret, *this);
        }
    }

    bzip2_extractor(bzip2_extractor const&) = delete;
    bzip2_extractor(bzip2_extractor&& rhs)
        : base_t{ std::move(static_cast<base_t&>(rhs)) }
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    bzip2_extractor& operator=(bzip2_extractor const&) = delete;
    bzip2_extractor& operator=(bzip2_extractor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_extractor();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
        return *this;
    }

    ~bzip2_extractor()
    {
        if (state_) state_->destroy_extractor();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = static_cast<unsigned int>(ivals.size());
        state_->strm_.next_in = const_cast<char*>(reinterpret_cast<char const*>(ivals.data()));

        do {
            int ret = state_->decompress();
            if (ret < 0 && ret != BZ_OUTBUFF_FULL) {
                if (ret == BZ_DATA_ERROR) {
                    on_error("bzip2 extractor data error", ret, *this);
                } else {
                    on_error("bzip2 extractor error", ret, *this);
                }
            }
            if (!state_->strm_.avail_out || ret == BZ_STREAM_END) {
                cons(state_->used_span());
                state_->reset_buffer();
            }
            if (ret == BZ_STREAM_END && state_->strm_.avail_in) {
                on_error("bzip2 unexpected data error", *this);
            }
        } while (state_->strm_.avail_in);
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
        int& ret = state_->ret;
        while (ret != BZ_STREAM_END) {
            state_->decompress();
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (ret < 0 && ret != BZ_OUTBUFF_FULL) {
                on_error("bzip extructor can't flush", ret, *this);
            }
            if (ret == BZ_STREAM_END && state_->strm_.avail_in) {
                on_error("bzip2 unexpected data error", *this);
            }
            if (us.empty()) {
                on_error("bzip2 unexpected end of input data", *this);
            }
        }
    }

    inline void reset() noexcept { state_->ret = BZ_OK; }

    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                if (state_->ret != BZ_STREAM_END) {
                    input = span_cast<const unsigned char>(p());
                }
                if (input.empty()) {
                    if (state_->ret != BZ_STREAM_END) {
                        state_->decompress();
                    }
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = static_cast<unsigned int>(input.size());
                state_->strm_.next_in = const_cast<char*>(reinterpret_cast<char const*>(input.data()));
            }
            
            int ret = state_->decompress();
            input = std::span<const unsigned char>{ (unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (ret < 0 && ret != BZ_OUTBUFF_FULL) {
                if (ret == BZ_DATA_ERROR) {
                    on_error("bzip2 extractor data error", ret, *this);
                } else {
                    on_error("bzip2 extractor error", ret, *this);
                }
            }
            if (!state_->strm_.avail_out || ret == BZ_OUTBUFF_FULL || ret == BZ_STREAM_END) {
                auto us = state_->used_span();
                state_->reset_buffer();
                return us;
            }
        }
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<bzip2_qrk<FromEHT>, ToQuarkT>
{
    using type = bzip2_extractor<FromEHT>;
};

}
