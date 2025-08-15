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
class utf16_to_icucp 
    : public generic_pusher<ErrorHandlerT>
    , icu_detail::result_buffer<char, ResultBuffSzV, isBuffStatic>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;
    using buffbase_t = icu_detail::result_buffer<char, ResultBuffSzV, isBuffStatic>;

    UConverter* conv_;
    
    static void callback(const void* context, UConverterFromUnicodeArgs* /*args*/, const UChar* /*codeUnits*/, int32_t /*length*/, UChar32 codePoint, UConverterCallbackReason /*reason*/, UErrorCode* pErrorCode)
    {
        utf16_to_icucp& h = *reinterpret_cast<utf16_to_icucp*>(const_cast<void*>(context));
        h.on_error(codePoint, pErrorCode);
    }

public:
    template <typename SomeErrorHandlerT>
    utf16_to_icucp(utf16_qrk<SomeErrorHandlerT> const&, icu_qrk<ErrorHandlerT, ResultBuffSzV, isBuffStatic> const& quark)
        : base_t{ quark }
    {
        UErrorCode errCode = U_ZERO_ERROR;
        conv_ = ucnv_open(quark.encoding, &errCode);
        if (U_FAILURE(errCode)) {
            on_error("can't create the converter to encoding", quark.encoding, errCode);
        }
        if constexpr (!std::is_void_v<ErrorHandlerT>) {
            ucnv_setFromUCallBack(conv_, callback, this, nullptr, nullptr, &errCode);
        }
    }

    utf16_to_icucp(utf16_to_icucp&& rhs) noexcept
        : base_t{ std::move(static_cast<base_t&>(rhs)) }
        , buffbase_t{ std::move(static_cast<buffbase_t&>(rhs)) }
        , conv_{ rhs.conv_ }
    {
        rhs.conv_ = nullptr;
        if constexpr (!std::is_void_v<ErrorHandlerT>) {
            UErrorCode errCode = U_ZERO_ERROR;
            ucnv_setFromUCallBack(conv_, callback, this, nullptr, nullptr, &errCode); // to reset fromUContext
        }
    }

    utf16_to_icucp& operator=(utf16_to_icucp&& rhs) noexcept
    {
        base_t::operator=(std::move(static_cast<base_t&>(rhs)));
        buffbase_t::operator=(std::move(static_cast<buffbase_t&>(rhs)));
        if (conv_) ucnv_close(conv_);
        conv_ = rhs.conv_;
        rhs.conv_ = nullptr;
        return this;
    }

    utf16_to_icucp(utf16_to_icucp const&) = delete;
    utf16_to_icucp& operator=(utf16_to_icucp const&) = delete;
    
    ~utf16_to_icucp() noexcept
    {
        if (conv_) ucnv_close(conv_);
    }

    template <CompatibleSpan<char16_t> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT&& cons)
    {
        do_push(span_cast<const char16_t>(ivals), false, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void push(char16_t ival, ConsumerT&& cons)
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
    inline void finish(ConsumerT&& cons)
    {
        do_push(std::span<char16_t>{}, true, std::forward<ConsumerT>(cons));
        if (buffbase_t::buffer() != buffbase_t::buffpos_) {
            cons(std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ });
        }
        reset();
    }

    inline void reset() noexcept
    {
        ucnv_resetFromUnicode(conv_);
        buffbase_t::buffpos_ = buffbase_t::buffer();
    }

    using input_element_type = char16_t;
    using output_element_type = char;
    std::span<const char16_t> replacement_stack_;

    template <typename ProviderT>
    std::span<const char> pull(std::span<const char16_t> & input, ProviderT p)
    {
        for (;;)
        {
            if (!replacement_stack_.empty()) for (;;) {
                UErrorCode errCode = do_job(replacement_stack_);
                if (errCode == U_BUFFER_OVERFLOW_ERROR) {
                    return flush_buffer(false);
                }
                if (errCode == U_INVALID_CHAR_FOUND || errCode == U_ILLEGAL_CHAR_FOUND || errCode == U_TRUNCATED_CHAR_FOUND) {
                    if constexpr (requires { this->char16_replacement(); }) {
                        on_error("the given encoding can't encode both the codepoint and its escape sequence");
                        this->replacement_clear();
                    }
                } else if (U_FAILURE(errCode)) {
                    on_conv_error(errCode);
                }
                if constexpr (requires { this->replacement_clear(); }) {
                    if (replacement_stack_.empty()) {
                        this->replacement_clear();
                        break;
                    }
                }
            }

            for (;;) {
                if (input.empty()) input = span_cast<const char16_t>(p());
                bool empty_input = input.empty();
                UErrorCode errCode = do_job(input); // if stack is empty then flush
                if (errCode == U_BUFFER_OVERFLOW_ERROR) {
                    return flush_buffer(false);
                }
                if (U_FAILURE(errCode)) {
                    if (errCode == U_INVALID_CHAR_FOUND || errCode == U_ILLEGAL_CHAR_FOUND || errCode == U_TRUNCATED_CHAR_FOUND) {
                        if constexpr (requires { this->char16_replacement(); }) {
                            replacement_stack_ = this->char16_replacement();
                            break;
                        }
                    }
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
    }

private:
    UErrorCode do_job(std::span<const char16_t>& ivals)
    {
        const char16_t* b = ivals.data();
        const char16_t* e = b + ivals.size();
        const char* targetlimit = buffbase_t::buffer() + ResultBuffSzV;

        UErrorCode errCode = U_ZERO_ERROR;
        ucnv_fromUnicode(conv_, &this->buffpos_, targetlimit, &b, e, nullptr, ivals.empty(), &errCode);
        ivals = ivals.subspan(b - ivals.data());
        return errCode;
    }

    std::span<const char> flush_buffer(bool allow_empty)
    {
        auto result = std::span{ buffbase_t::buffer(), buffbase_t::buffpos_ };
        buffbase_t::buffpos_ = buffbase_t::buffer();
        if (!allow_empty && result.empty()) {
            // I don't think it will ever happen
            on_error("internal error: flush empty buffer");
        }

        return result;
    }

    auto on_conv_error(UErrorCode errCode)
    {
        return on_error("can't convert with the converter", errCode, conv_);
    }

    template <typename ConsumerT>
    void do_push(std::span<const char16_t> ivals, bool /* flush */, ConsumerT && cons)
    {
        UErrorCode errCode;
        do {
            errCode = do_job(ivals);
            if (errCode == U_BUFFER_OVERFLOW_ERROR) {
                auto sp = flush_buffer(false);
                if (!sp.empty()) cons(sp);
            } else if (errCode == U_INVALID_CHAR_FOUND || errCode == U_ILLEGAL_CHAR_FOUND || errCode == U_TRUNCATED_CHAR_FOUND) {
                if constexpr (requires { this->char16_replacement(); }) {
                    if (auto span = this->char16_replacement(); !span.empty()) {
                        push(span, std::forward<ConsumerT>(cons));
                        this->replacement_clear();
                    } else {
                        on_error("the given encoding can't encode both the codepoint and its escape sequence");
                    }
                }
            } else if (U_FAILURE(errCode)) {
                on_conv_error(errCode);
            }
        } while (!ivals.empty() || errCode == U_BUFFER_OVERFLOW_ERROR);
    }
};


template <typename FromEHT, typename ToEHT, size_t ResultBuffSzV, bool isBuffStatic>
struct cvt_resolver<utf16_qrk<FromEHT>, icu_qrk<ToEHT, ResultBuffSzV, isBuffStatic>>
{
    using type = utf16_to_icucp<ToEHT, ResultBuffSzV, isBuffStatic>;
};

}
