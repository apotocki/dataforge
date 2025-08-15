/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

DECLARE_INTEGRAL_CVT_QUARK(utf7, char, 8)
DECLARE_INTEGRAL_CVT_QUARK(utf8, char, 8)
DECLARE_INTEGRAL_CVT_QUARK(utf16, char16_t, 16)
DECLARE_INTEGRAL_CVT_QUARK(utf32, char32_t, 32)

struct enumerated_graphemes_qrk : cvt_qrk<void>
{
    using cvt_quark_t = cvt_qrk<void>;
    using cvt_quark_t::cvt_quark_t;
};


inline compound_qrk<enumerated_graphemes_qrk, enum_qrk<void>> enumerated_graphemes;

}

#include "../detail/unicode/utf7_to_utf16.hpp"
#include "../detail/unicode/utf8_to_utf32.hpp"
#include "../detail/unicode/utf8_to_utf16.hpp"
#include "../detail/unicode/utf16_to_utf32.hpp"
#include "../detail/unicode/utf16_to_utf8.hpp"
#include "../detail/unicode/utf16_to_utf7.hpp"
#include "../detail/unicode/utf32_to_utf16.hpp"
#include "../detail/unicode/utf32_to_utf8.hpp"
#include "../detail/unicode/utf32_grapheme_enumerator_pusher.hpp"
