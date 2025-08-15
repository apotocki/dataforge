/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
#include <unicode/ucnv.h>

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ReplacementContainerT = std::vector<char16_t>>
struct icu_basic_escaper_handler
{
    mutable ReplacementContainerT replacement;

    std::span<const char16_t> char16_replacement() const { return replacement; }
    void replacement_clear() { replacement.clear(); }

    inline static char16_t half_byte(uint_least8_t hb)
    {
        hb &= 0xf;
        return hb < 0xa ? ('0' + hb) : 'a' - 0xa + hb;
    }

    void operator()(UChar32 codePoint, UErrorCode* pErrorCode) const
    {
        if (*pErrorCode != U_INVALID_CHAR_FOUND && *pErrorCode != U_TRUNCATED_CHAR_FOUND && *pErrorCode != U_ILLEGAL_CHAR_FOUND) return;
        if (!replacement.empty()) { // already in replacement mode
            replacement.clear();
            return;
        }
        replacement.push_back('\\');
        if (codePoint < 0x10000) {
            replacement.push_back('u');
            replacement.push_back(half_byte(codePoint >> 12));
            replacement.push_back(half_byte(codePoint >> 8));
            replacement.push_back(half_byte(codePoint >> 4));
            replacement.push_back(half_byte(codePoint));
        } else {
            replacement.push_back('U');
            replacement.resize(10);
            for (int i = 28; i >= 0; i -= 4) {
                replacement[9 - i / 4] = half_byte(codePoint >> i);
            }
        }
    }

    void operator()(std::span<const char> units, UErrorCode* pErrorCode) const
    {
        if (*pErrorCode == U_ZERO_ERROR) return;
        for (char c : units) {
            replacement.push_back('\\');
            replacement.push_back('x');
            replacement.push_back(half_byte(c >> 4));
            replacement.push_back(half_byte(c));
        }
    }

    void operator()(std::string const& err) const
    {
        throw std::runtime_error(err);
    }

    void operator()(std::string const& err, std::string const& enc, UErrorCode errCode) const
    {
        std::ostringstream errss;
        errss << err << ": '" << enc << "', error: " << (int)errCode;
        throw std::runtime_error(errss.str());
    }

    void operator()(std::string const& err, UErrorCode errCode, UConverter* conv) const
    {
        UErrorCode errCode2 = U_ZERO_ERROR;
        auto const* encname = ucnv_getName(conv, &errCode2);
        std::ostringstream errss;
        errss << err << "'" << (U_FAILURE(errCode2) ? "unknown" : encname) << "', error: " << (int)errCode;
        throw std::runtime_error(errss.str());
    }
};

template <typename ErrorHandlerT = void, size_t ResultBuffSzV = 32, bool isBuffStatic = true>
struct icu_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;
    using cvt_quark_t::cvt_quark_t;

    const char* encoding;
    explicit icu_qrk(const char* enc_name) : encoding{ enc_name } {}
};

}

#include "../detail/unicode/utf16_to_icucp.hpp"
#include "../detail/unicode/icucp_to_utf16.hpp"
