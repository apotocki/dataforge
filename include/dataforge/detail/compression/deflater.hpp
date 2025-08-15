/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "deflate.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class deflater : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    deflate_detail::state * state_;

public:
    template <IntegralBasedQuark<8> SrcT>
    deflater(SrcT const&, deflate_qrk<ErrorHandlerT> const& dt)
        : base_t{ dt }
    {
        state_ = deflate_detail::state::create(static_cast<uInt>(dt.buffer_size));

        int ret = deflateInit2(&state_->strm_, (int)(dt.compression_level % 10), Z_DEFLATED, dt.gzip ? 16 + MAX_WBITS : -15, 9, Z_DEFAULT_STRATEGY);

        if (ret != Z_OK) {
            on_error("deflate compressor initialization error", ret, *this);
        }
    }

    deflater(deflater const&) = delete;
    deflater(deflater&& rhs)
        : base_t{std::move(static_cast<base_t&>(rhs))}
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    deflater& operator=(deflater const&) = delete;
    deflater& operator=(deflater&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_deflate();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
        return *this;
    }

    ~deflater()
    {
        if (state_) state_->destroy_deflate();
    }

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = static_cast<uInt>(ivals.size());
        state_->strm_.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(ivals.data()));

        do {
            int ret = ::deflate(&state_->strm_, Z_NO_FLUSH);
            if (ret < 0 && ret != Z_BUF_ERROR) {
                on_error("deflate compressor error", ret, *this);
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
        uint_least8_t val[1] = { static_cast<uint_least8_t>(ival) };
        push(std::span{val}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        for (;;) {
            int ret = ::deflate(&state_->strm_, Z_FINISH);
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (ret == Z_STREAM_END) break;
            if (!us.empty() && (ret == Z_OK || ret == Z_BUF_ERROR)) continue;
            on_error("deflate compressor can't flush", ret, *this);
        }
    }

    inline void reset() noexcept { state_->ret = Z_OK; }

    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const unsigned char>(p());
                if (input.empty()) {
                    if (int ret = ::deflate(&state_->strm_, Z_FINISH); ret < 0) {
                        on_error("deflate compressor error", ret, *this);
                    }
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = static_cast<uInt>(input.size());
                state_->strm_.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(input.data()));
            }
            
            int ret = ::deflate(&state_->strm_, Z_NO_FLUSH);
            input = std::span<const unsigned char>{ (unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (ret < 0 && ret != Z_BUF_ERROR) {
                on_error("deflate compressor error", ret, *this);
            }
            if (!state_->strm_.avail_out || ret == Z_BUF_ERROR) {
                state_->reset_buffer();
                return state_->span();
            }
        }
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, deflate_qrk<ToEHT>>
{
    using type = deflater<ToEHT>;
};

}
