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
class bzip2_compressor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    bzip2_detail::state * state_;

public:
    template <IntegralBasedQuark<8> SrcT>
    bzip2_compressor(SrcT const&, bzip2_qrk<ErrorHandlerT> const& dt)
        : base_t{ dt }
    {
        state_ = bzip2_detail::state::create(static_cast<unsigned int>(dt.buffer_size));

        int ret = BZ2_bzCompressInit(&state_->strm_, dt.block_size100k, 0, dt.work_factor);

        if (ret != Z_OK) {
            on_error("bzip2 compressor initialization error", ret, *this);
        }
    }

    bzip2_compressor(bzip2_compressor const&) = delete;
    bzip2_compressor(bzip2_compressor&& rhs)
        : base_t{std::move(static_cast<base_t&>(rhs))}
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    bzip2_compressor& operator=(bzip2_compressor const&) = delete;
    bzip2_compressor& operator=(bzip2_compressor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_compressor();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
    }

    ~bzip2_compressor()
    {
        if (state_) state_->destroy_compressor();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = static_cast<unsigned int>(ivals.size());
        state_->strm_.next_in = const_cast<char*>(reinterpret_cast<char const*>(ivals.data()));

        do {
            int ret = BZ2_bzCompress(&state_->strm_, BZ_RUN);
            if (ret < 0 && ret != BZ_OUTBUFF_FULL) {
                on_error("bzip2 compressor error", ret, *this);
            }
            if (!state_->strm_.avail_out) {
                cons(state_->span());
                state_->reset_buffer();
            }
        } while (state_->strm_.avail_in);
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT && cons)
    {
        char val[1] = { static_cast<char>(ival) };
        push(std::span{val}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        for (;;) {
            int ret = BZ2_bzCompress(&state_->strm_, BZ_FINISH);
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (ret == BZ_STREAM_END) break;
            if (!us.empty() && (ret == Z_OK || ret == BZ_OUTBUFF_FULL || ret == BZ_FINISH_OK)) continue;
            on_error("bzip2 compressor can't flush", ret, *this);
        }
    }

    inline void reset() noexcept { }

    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const unsigned char>(p());
                if (input.empty()) {
                    int ret = BZ2_bzCompress(&state_->strm_, BZ_FINISH);
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = static_cast<unsigned int>(input.size());
                state_->strm_.next_in = const_cast<char*>(reinterpret_cast<char const*>(input.data()));
            }
            
            int ret = BZ2_bzCompress(&state_->strm_, BZ_RUN);
            input = std::span<const unsigned char>{ (unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (ret < 0 && ret != BZ_OUTBUFF_FULL) {
                on_error("bzip2 compressor error", ret, *this);
            }
            if (!state_->strm_.avail_out || ret == BZ_OUTBUFF_FULL) {
                state_->reset_buffer();
                return state_->span();
            }
        }
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, bzip2_qrk<ToEHT>>
{
    using type = bzip2_compressor<ToEHT>;
};

}
