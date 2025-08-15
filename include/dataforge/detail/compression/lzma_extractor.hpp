/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <stdexcept>
#include <sstream>
#include "lzma.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
class lzma_extractor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    lzma_detail::state* state_;
    bool is_lzma2 = false;

public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> DestT>
    lzma_extractor(lzma_qrk<lzma_type::lzma1, ErrorHandlerT> const& t, DestT const&)
        : base_t{ t }, is_lzma2 { false }
    {
        state_ = lzma_detail::state::create(static_cast<unsigned int>(t.buffer_size));
        check_ret(lzma_alone_decoder(&state_->strm_, UINT64_MAX));
    }

    template <IntegralBasedQuark<8> DestT>
    lzma_extractor(lzma_qrk<lzma_type::lzma2, ErrorHandlerT> const& t, DestT const&)
        : base_t{ t }, is_lzma2 { true }
    {
        state_ = lzma_detail::state::create(static_cast<unsigned int>(t.buffer_size));
        check_ret(lzma_stream_decoder(&state_->strm_, UINT64_MAX, LZMA_CONCATENATED));
    }

    void check_ret(lzma_ret ret)
    {
        if (ret == LZMA_OK) return;
        switch (ret) {
        case LZMA_MEM_ERROR:
            throw std::runtime_error("Memory allocation failed");
        case LZMA_OPTIONS_ERROR:
            throw std::runtime_error("Unsupported decompressor flags");
        default:
            throw std::runtime_error("Unknown error, possibly a bug");
        }
    }


    lzma_extractor(lzma_extractor const&) = delete;
    lzma_extractor(lzma_extractor&& rhs)
        : base_t{ std::move(static_cast<base_t&>(rhs)) }
        , state_{ rhs.state_ }
        , is_lzma2 { rhs.is_lzma2 }
    {
        rhs.state_ = nullptr;
    }

    lzma_extractor& operator=(lzma_extractor const&) = delete;
    lzma_extractor& operator=(lzma_extractor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy();
        state_ = rhs.state_;
        is_lzma2 = rhs.is_lzma2;
        rhs.state_ = nullptr;
    }

    ~lzma_extractor()
    {
        if (state_) state_->destroy();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = ivals.size();
        state_->strm_.next_in = reinterpret_cast<uint8_t const*>(ivals.data());

        do {
            state_->ret = lzma_code(&state_->strm_, LZMA_RUN);
            if (state_->ret != LZMA_OK && state_->ret != LZMA_STREAM_END) {
                do_error(state_->ret);
            }
            if (!state_->strm_.avail_out || state_->ret == LZMA_STREAM_END) {
                cons(state_->used_span());
                state_->reset_buffer();
            }
            if (state_->ret == LZMA_STREAM_END && state_->strm_.avail_in) {
                on_error("lzma unexpected data error", *this);
            }
        } while (state_->strm_.avail_in);

    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT && cons)
    {
        uint8_t val[1] = { static_cast<uint8_t>(ival) };
        push(std::span{val}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        while (state_->ret != LZMA_STREAM_END) {
            state_->ret = lzma_code(&state_->strm_, LZMA_FINISH);
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (state_->ret != LZMA_OK && state_->ret != LZMA_STREAM_END) {
                do_error(state_->ret);
            }
            if (state_->ret == LZMA_STREAM_END && state_->strm_.avail_in) {
                on_error("unexpected data error", *this);
            }
        }
    }

    inline void reset() noexcept { state_->ret = LZMA_OK; }
    
    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                if (state_->ret != LZMA_STREAM_END) {
                    input = span_cast<const unsigned char>(p());
                }
                if (input.empty()) {
                    if (state_->ret != LZMA_STREAM_END) {
                        state_->ret = lzma_code(&state_->strm_, LZMA_FINISH);
                        if (state_->ret != LZMA_OK && state_->ret != LZMA_STREAM_END) {
                            do_error(state_->ret);
                        }
                    }
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = input.size();
                state_->strm_.next_in = reinterpret_cast<uint8_t const*>(input.data());
            }
            
            state_->ret = lzma_code(&state_->strm_, LZMA_RUN);
            input = std::span<const unsigned char>{ (unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (state_->ret != LZMA_OK && state_->ret != LZMA_STREAM_END) {
                do_error(state_->ret);
            }
            if (!state_->strm_.avail_out || state_->ret == LZMA_STREAM_END) {
                auto us = state_->used_span();
                state_->reset_buffer();
                return us;
            }
        }
    }

private:
    void do_error(lzma_ret r)
    {
        switch (r) {
        case LZMA_MEM_ERROR:
            return on_error("Memory allocation failed");
        case LZMA_FORMAT_ERROR:
            // .xz magic bytes weren't found.
            return on_error(is_lzma2 ? "The input is not in the .xz format" : "The input is not in the .lzma format");
        case LZMA_OPTIONS_ERROR:
            return on_error("Unsupported compression options");
        case LZMA_DATA_ERROR:
            return on_error("Compressed file is corrupt");
        case LZMA_BUF_ERROR:
            return on_error("Compressed file is truncated or otherwise corrupt");
        default:
            return on_error("Unknown error, possibly a bug");
        }
    }

};

template <lzma_type TV, typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<lzma_qrk<TV, FromEHT>, ToQuarkT>
{
    using type = lzma_extractor<FromEHT>;
};

}
