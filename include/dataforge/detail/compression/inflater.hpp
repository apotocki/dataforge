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
class inflater : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    deflate_detail::state * state_;

public:
    template <IntegralBasedQuark<8> SrcT>
    inflater(SrcT const&, inflate_qrk<ErrorHandlerT> const& it)
        : base_t{ it }
    {
        initialize(it);
    }

    template <IntegralBasedQuark<8> DestT>
    inflater(deflate_qrk<ErrorHandlerT> const& dt, DestT const&)
        : base_t{ dt }
    {
        initialize(dt);
    }

    template <typename QuarkT>
    void initialize(QuarkT const& t)
    {
        state_ = deflate_detail::state::create(static_cast<uInt>(t.buffer_size));

        int ret = inflateInit2(&state_->strm_, t.gzip ? 16 + MAX_WBITS : -15);

        if (ret != Z_OK) {
            on_error("inflate initialization error", ret, *this);
        }
    }

    inflater(inflater const&) = delete;
    inflater(inflater&& rhs)
        : base_t{std::move(static_cast<base_t&>(rhs))}
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    inflater& operator=(inflater const&) = delete;
    inflater& operator=(inflater&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_inflate();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
    }

    ~inflater()
    {
        if (state_) state_->destroy_inflate();
    }

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = static_cast<uInt>(ivals.size());
        state_->strm_.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(ivals.data()));

        do {
            int ret = ::inflate(&state_->strm_, Z_NO_FLUSH);
            if (ret < 0 && ret != Z_BUF_ERROR) {
                if (ret == Z_DATA_ERROR) {
                    on_error("inflate data error", ret, *this);
                } else {
                    on_error("inflate error", ret, *this);
                }
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
            state_->ret = ::inflate(&state_->strm_, Z_FINISH);
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (state_->ret == Z_STREAM_END) break;
            if (!us.empty() && (state_->ret == Z_OK || state_->ret == Z_BUF_ERROR)) continue;
            on_error("inflate can't flush", state_->ret, *this);
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
                if (state_->ret != Z_STREAM_END) {
                    input = span_cast<const unsigned char>(p());
                }
                if (input.empty()) {
                    if (state_->ret != Z_STREAM_END) {
                        state_->ret = ::inflate(&state_->strm_, Z_FINISH);
                    }
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = static_cast<uInt>(input.size());
                state_->strm_.next_in = const_cast<Bytef*>(reinterpret_cast<Bytef const*>(input.data()));
            }
            
            state_->ret = ::inflate(&state_->strm_, Z_NO_FLUSH);
            input = std::span<const unsigned char>{ (unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (state_->ret < 0 && state_->ret != Z_BUF_ERROR) {
                if (state_->ret == Z_DATA_ERROR) {
                    on_error("inflate data error", state_->ret, *this);
                } else {
                    on_error("inflate error", state_->ret, *this);
                }
            }
            if (!state_->strm_.avail_out || state_->ret == Z_BUF_ERROR || state_->ret == Z_STREAM_END) {
                auto us = state_->used_span();
                state_->reset_buffer();
                return us;
            }
        }
    }
};

//template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
//struct cvt_resolver<deflate_qrk<FromEHT>, ToQuarkT>
//{
//    using type = inflater<FromEHT>;
//};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<deflate_qrk<FromEHT>, ToQuarkT>
{
    using type = inflater<FromEHT>;
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, inflate_qrk<ToEHT>>
{
    using type = inflater<ToEHT>;
};

}
