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
class lz4_compressor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    lz4_detail::state * state_;

public:
    template <IntegralBasedQuark<8> SrcT>
    lz4_compressor(SrcT const&, lz4_qrk<ErrorHandlerT> const& qrk)
        : base_t{ qrk }
    {
        state_ = lz4_detail::state::create(qrk, true);
        reset();
        write_frame_header();
    }

    lz4_compressor(lz4_compressor const&) = delete;
    lz4_compressor(lz4_compressor&& rhs)
        : base_t{std::move(static_cast<base_t&>(rhs))}
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    lz4_compressor& operator=(lz4_compressor const&) = delete;
    lz4_compressor& operator=(lz4_compressor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy_compressor();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
        return *this;
    }

    ~lz4_compressor()
    {
        if (state_) state_->destroy_compressor();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        auto isp = span_cast<const unsigned char>(ivals);
        do {
            size_t size2compress = isp.size();
            for (;;)
            {
                size_t max_exp_sz = LZ4F_compressBound(size2compress, &state_->prefs_);
                if (state_->free_size() >= max_exp_sz) break;
                if (state_->buffer_size >= max_exp_sz) {
                    cons(state_->detach_out_span());
                    break;
                } else {
                    size2compress >>= 1;
                    if (!size2compress) {
                        on_error("lz4 compression failed, buffer is too small", *this);
                    }
                }
            }

            size_t compressed_size = state_->compress_update(std::span{isp.data(), size2compress});
            if (LZ4F_isError(compressed_size)) {
                on_error("lz4 compression failed", compressed_size, *this);
            } else {
                isp = isp.subspan(size2compress);
                state_->avail_out += compressed_size;
            }
        } while (!isp.empty());
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
        size_t max_exp_sz = LZ4F_compressBound(0, &state_->prefs_);
        if (state_->free_size() < max_exp_sz) {
            cons(state_->detach_out_span());
            assert(max_exp_sz >= state_->free_size());
        }
        size_t compressed_size = state_->compress_end();
        if (LZ4F_isError(compressed_size)) {
            on_error("lz4 compression end failed", compressed_size, *this);
        }
        state_->avail_out += compressed_size;
        if (state_->avail_out) cons(state_->detach_out_span());
        state_->context_ready_ = 1;
        reset();
    }

    inline void reset()
    {
        state_->reset([this](const char* errstr, size_t r) { on_error(errstr, r, *this); });
        write_frame_header();
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
                if (input.empty()) {
                    size_t max_exp_sz = LZ4F_compressBound(0, &state_->prefs_);
                    if (state_->free_size() < max_exp_sz) {
                        return state_->detach_out_span();
                    }
                    size_t compressed_size = state_->compress_end();
                    if (LZ4F_isError(compressed_size)) {
                        on_error("lz4 compression end failed", compressed_size, *this);
                    }
                    state_->avail_out += compressed_size;
                    state_->processing_finished_ = 1;
                    state_->context_ready_ = 1;
                    return state_->detach_out_span();
                }
            }

            if (state_->context_ready_) {
                write_frame_header();
            }

            size_t size2compress = input.size();
            for (;;)
            {
                size_t max_exp_sz = LZ4F_compressBound(size2compress, &state_->prefs_);
                if (state_->free_size() >= max_exp_sz) break;
                if (state_->buffer_size >= max_exp_sz) return state_->detach_out_span();
                
                size2compress >>= 1;
                if (!size2compress) {
                    on_error("lz4 compression failed, buffer is too small", *this);
                }
            }

            size_t compressed_size = state_->compress_update(std::span{ input.data(), size2compress });
            if (LZ4F_isError(compressed_size)) {
                on_error("lz4 compression failed", compressed_size, *this);
            } else {
                input = input.subspan(size2compress);
                state_->avail_out += compressed_size;
            }
        }
    }

private:
    void write_frame_header()
    {
        size_t header_size = state_->compress_begin();
        if (LZ4F_isError(header_size)) {
            on_error("Failed to start compression", header_size, *this);
        }
        state_->avail_out = header_size;
        state_->context_ready_ = 0;
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, lz4_qrk<ToEHT>>
{
    using type = lz4_compressor<ToEHT>;
};

}
