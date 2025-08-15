/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <unicode/ucnv.h>
#include "icu.hpp"
#include "../../unicode/utf.hpp"

namespace dataforge {

template <typename ErrorHandlerT, size_t ResultBuffSzV, bool isBuffStatic>
class icucp_to_utf16
    : public generic_pusher<ErrorHandlerT>
    , icu_detail::result_buffer<char16_t, ResultBuffSzV, isBuffStatic>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;
    using buffbase_t = icu_detail::result_buffer<char16_t, ResultBuffSzV, isBuffStatic>;

    UConverter* conv_;

    static void callback(const void* context, UConverterToUnicodeArgs* args, const char* codeUnits, int32_t length, UConverterCallbackReason reason, UErrorCode* pErrorCode)
    {
        icucp_to_utf16& h = *reinterpret_cast<icucp_to_utf16*>(const_cast<void*>(context));
        h.on_error(std::span{ codeUnits, (size_t)length}, pErrorCode);
    }

public:
    template <typename SomeErrorHandlerT>
    icucp_to_utf16(icu_qrk<ErrorHandlerT, ResultBuffSzV, isBuffStatic> const& quark, utf16_qrk<SomeErrorHandlerT> const&)
        : base_t{ quark }
    {
        UErrorCode errCode = U_ZERO_ERROR;
        conv_ = ucnv_open(quark.encoding, &errCode);
        if (U_FAILURE(errCode)) {
            on_error("can't create the converter from encoding", quark.encoding, errCode);
        }
        if constexpr (!std::is_void_v<ErrorHandlerT>) {
            ucnv_setToUCallBack(conv_, callback, this, nullptr, nullptr, &errCode);
        }
    }

    icucp_to_utf16(icucp_to_utf16&& rhs) noexcept
        : base_t{ std::move(static_cast<base_t&>(rhs)) }
        , buffbase_t{ std::move(static_cast<buffbase_t&>(rhs)) }
        , conv_{ rhs.conv_ }
    {
        rhs.conv_ = nullptr;
        if constexpr (!std::is_void_v<ErrorHandlerT>) {
            if (conv_) {
                UErrorCode errCode = U_ZERO_ERROR;
                ucnv_setToUCallBack(conv_, callback, this, nullptr, nullptr, &errCode); // to reset ToUContext
            }
        }
    }

    icucp_to_utf16(icucp_to_utf16 const&) = delete;
    icucp_to_utf16& operator=(icucp_to_utf16 const&) = delete;

    ~icucp_to_utf16() noexcept
    {
        if (conv_) ucnv_close(conv_);
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        do_push(span_cast<const char>(ivals), false, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void push(char ival, ConsumerT&& cons)
    {
        do_push(std::span{ &ival, 1 }, false, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void flush(ConsumerT cons)
    {
        if (buffbase_t::buffer() != buffbase_t::buffpos_) {
            cons(std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ });
            buffbase_t::buffpos_ = buffbase_t::buffer();
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        do_push(std::span<char>{}, true, std::forward<ConsumerT>(cons));
        if (buffbase_t::buffer() != buffbase_t::buffpos_) {
            cons(std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ });
        }
        reset();
    }

    inline void reset() noexcept
    {
        ucnv_resetToUnicode(conv_);
        buffbase_t::buffpos_ = buffbase_t::buffer();
    }

    using input_element_type = char;
    using output_element_type = char16_t;
    bool has_replacement = false;

    template <typename ProviderT>
    std::span<const char16_t> pull(std::span<const char> & input, ProviderT p)
    {
        for (;;) {
            if constexpr (requires { this->char16_replacement(); }) {
                if (has_replacement) {
                    has_replacement = false;
                    return this->char16_replacement();
                } else {
                    this->replacement_clear();
                }
                if (auto replacement = this->char16_replacement(); !replacement.empty()) {
                    flush_buffer(true);
                }
            }
        
            if (input.empty()) input = span_cast<const char>(p());
            bool empty_input = input.empty();
            UErrorCode errCode = do_job(input); // if stack is empty then flush
            switch (errCode)
            {
            case U_ZERO_ERROR:
                break;
            case U_BUFFER_OVERFLOW_ERROR:
                return flush_buffer(false);
            case U_INVALID_CHAR_FOUND:
            case U_ILLEGAL_CHAR_FOUND:
            case U_TRUNCATED_CHAR_FOUND:
                if constexpr (requires { this->char16_replacement(); }) {
                    if (auto replacement = this->char16_replacement(); !replacement.empty()) {
                        has_replacement = true;
                        if (buffbase_t::buffer() != buffbase_t::buffpos_) {
                            std::span r{ buffbase_t::buffer(), buffbase_t::buffpos_ };
                            buffbase_t::buffpos_ = buffbase_t::buffer();
                            return r;
                        }
                    }
                    break;
                }
            default:
                if constexpr (std::is_void_v<decltype(on_conv_error(errCode))>) {
                    on_conv_error(errCode);
                    // if here just skip
                } else {
                    return on_conv_error(errCode);
                }
            }
            if (empty_input) {
                return flush_buffer(true);
            }
        }
    }

private:
    UErrorCode do_job(std::span<const char>& ivals)
    {
        const char* b = ivals.data();
        const char* e = b + ivals.size();
        const char16_t* targetlimit = buffbase_t::buffer() + ResultBuffSzV;

        UErrorCode errCode = U_ZERO_ERROR;
        ucnv_toUnicode(conv_, &this->buffpos_, targetlimit, &b, e, nullptr, ivals.empty(), &errCode);
        ivals = ivals.subspan(b - ivals.data());
        return errCode;
    }

    std::span<const char16_t> flush_buffer(bool allow_empty)
    {
        auto result = std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ };
        buffbase_t::buffpos_ = buffbase_t::buffer();
        if (!allow_empty && result.empty()) {
            on_error("internal error: flush empty buffer");
        }

        return result;
    }

    auto on_conv_error(UErrorCode errCode)
    {
        return on_error("can't convert with the converter", errCode, conv_);
    }

    template <typename ConsumerT>
    void do_push(std::span<const char> ivals, bool flush, ConsumerT cons)
    {
        UErrorCode errCode;
        do {
            errCode = do_job(ivals);
            switch (errCode)
            {
            case U_BUFFER_OVERFLOW_ERROR:
                if (auto sp = flush_buffer(false); !sp.empty()) cons(sp);
            case U_ZERO_ERROR:
                break;
            case U_INVALID_CHAR_FOUND:
            case U_ILLEGAL_CHAR_FOUND:
            case U_TRUNCATED_CHAR_FOUND:
                if constexpr (requires { this->char16_replacement(); }) {
                    auto span = this->char16_replacement();
                    size_t sz = (std::min)(span.size(), buffbase_t::available_size());
                    std::memcpy(buffbase_t::buffpos_, span.data(), sz * sizeof(char16_t));
                    buffbase_t::buffpos_ += sz;
                    if (!buffbase_t::available_size()) {
                        cons(std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ });
                        buffbase_t::buffpos_ = buffbase_t::buffer();
                        span = span.subspan(sz);
                        if (!span.empty() && span.size() >= buffbase_t::available_size()) {
                            cons(span);
                        } else {
                            std::memcpy(buffbase_t::buffpos_, span.data(), span.size() * sizeof(char16_t));
                            buffbase_t::buffpos_ += span.size();
                        }
                    }
                    this->replacement_clear();
                }
                break;
            default:
                on_conv_error(errCode);
            }
        } while (!ivals.empty() || errCode == U_BUFFER_OVERFLOW_ERROR);
    }
};

template <typename FromEHT, typename ToEHT, size_t ResultBuffSzV, bool isBuffStatic>
struct cvt_resolver<icu_qrk<FromEHT, ResultBuffSzV, isBuffStatic>, utf16_qrk<ToEHT>>
{
    using type = icucp_to_utf16<FromEHT, ResultBuffSzV, isBuffStatic>;
};

}
