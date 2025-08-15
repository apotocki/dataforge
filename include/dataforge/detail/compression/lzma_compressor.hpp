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
class lzma_compressor : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    lzma_detail::state * state_;

public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <IntegralBasedQuark<8> SrcT>
    lzma_compressor(SrcT const&, lzma_qrk<lzma_type::lzma1, ErrorHandlerT> const& t)
        : base_t{ t }
    {
        state_ = lzma_detail::state::create(static_cast<unsigned int>(t.buffer_size));

        lzma_options_lzma options;
        if (lzma_lzma_preset(&options, t.preset)) {
            throw std::runtime_error((std::ostringstream{} << "the preset " << t.preset << " is not supported").str());
        }
        check_ret(lzma_alone_encoder(&state_->strm_, &options));
    }

    template <IntegralBasedQuark<8> SrcT>
    lzma_compressor(SrcT const&, lzma_qrk<lzma_type::lzma2, ErrorHandlerT> const& t)
        : base_t{ t }
    {
        state_ = lzma_detail::state::create(static_cast<unsigned int>(t.buffer_size));
//#if LZMA_HAS_STR_TO_FILTERS
        if (t.filterscfg) {
            lzma_filter filters[LZMA_FILTERS_MAX + 1];
            int error_pos;
            if (const char *err = lzma_str_to_filters(t.filterscfg, &error_pos, filters, 0, NULL); err) {
                throw std::runtime_error((std::ostringstream{} << err << ", error at: ->" << (t.filterscfg + error_pos)).str());
            }

            lzma_ret ret = lzma_stream_encoder(&state_->strm_, filters, t.check);
            lzma_filters_free(filters, nullptr);
            check_ret(ret);
        } else {
//#else
            check_ret(lzma_easy_encoder(&state_->strm_, t.preset, t.check));//t.preset, LZMA_CHECK_CRC64)); //t.check));
        }
//#endif
    }

    void check_ret(lzma_ret ret)
    {
        // Return successfully if the initialization went fine.
        if (ret == LZMA_OK) return;

        // Something went wrong. The possible errors are documented in
        // lzma/container.h (src/liblzma/api/lzma/container.h in the source
        // package or e.g. /usr/include/lzma/container.h depending on the
        // install prefix).
        switch (ret) {
        case LZMA_MEM_ERROR:
            throw std::runtime_error("Memory allocation failed");
        case LZMA_OPTIONS_ERROR:
            throw std::runtime_error("Specified preset is not supported");
        case LZMA_UNSUPPORTED_CHECK:
            throw std::runtime_error("Specified integrity check is not supported");
        default:
            // This is most likely LZMA_PROG_ERROR indicating a bug in
            // this program or in liblzma. It is inconvenient to have a
            // separate error message for errors that should be impossible
            // to occur, but knowing the error code is important for
            // debugging. That's why it is good to print the error code
            // at least when there is no good error message to show.
            throw std::runtime_error("Unknown error, possibly a bug");
        }
    }

    lzma_compressor(lzma_compressor const&) = delete;
    lzma_compressor(lzma_compressor&& rhs)
        : base_t{std::move(static_cast<base_t&>(rhs))}
        , state_{ rhs.state_ }
    {
        rhs.state_ = nullptr;
    }

    lzma_compressor& operator=(lzma_compressor const&) = delete;
    lzma_compressor& operator=(lzma_compressor&& rhs)
    {
        base_t::operator= (std::move(static_cast<base_t&>(rhs)));
        if (state_) state_->destroy();
        state_ = rhs.state_;
        rhs.state_ = nullptr;
        return *this;
    }

    ~lzma_compressor()
    {
        if (state_) state_->destroy();
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        state_->strm_.avail_in = ivals.size();
        state_->strm_.next_in = reinterpret_cast<uint8_t const*>(ivals.data());

        do {
            lzma_ret ret = lzma_code(&state_->strm_, LZMA_RUN);
            if (ret != LZMA_OK) {
                on_error("lzma compressor error", ret, *this);
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
        uint8_t val[1] = { static_cast<uint8_t>(ival) };
        push(std::span{val}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        for (;;) {
            lzma_ret ret = lzma_code(&state_->strm_, LZMA_FINISH);
            auto us = state_->used_span();
            if (!us.empty()) {
                cons(us);
                state_->reset_buffer();
            }
            if (ret == LZMA_STREAM_END) break;
            if (!us.empty() && (ret == LZMA_OK || ret == LZMA_STREAM_END)) continue;
            on_error("lzma compressor can't flush", ret, *this);
        }
    }

    inline void reset() noexcept { }

    template <typename ProviderT>
    std::span<const unsigned char> pull(std::span<const unsigned char>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const unsigned char>(p());
                if (input.empty()) {
                    int ret = lzma_code(&state_->strm_, LZMA_FINISH);
                    auto us = state_->used_span();
                    state_->reset_buffer();
                    return us;
                }
                state_->strm_.avail_in = input.size();
                state_->strm_.next_in = reinterpret_cast<uint8_t const*>(input.data());
            }

            int ret = lzma_code(&state_->strm_, LZMA_RUN);
            input = std::span{ (const unsigned char*)state_->strm_.next_in, state_->strm_.avail_in };

            if (ret != LZMA_OK) {
                on_error("lzma compressor error", ret, *this);
            }
            if (!state_->strm_.avail_out) {
                state_->reset_buffer();
                return state_->span();
            }
        }
    }
};

template <IntegralBasedQuark<8> FromQuarkT, lzma_type TV, typename ToEHT>
struct cvt_resolver<FromQuarkT, lzma_qrk<TV, ToEHT>>
{
    using type = lzma_compressor<ToEHT>;
};

}
