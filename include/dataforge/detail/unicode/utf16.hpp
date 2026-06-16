/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

static constexpr char16_t UTF16_SUR_HIGH_START = u'\xD800';
static constexpr char16_t UTF16_SUR_HIGH_END   = u'\xDBFF';
static constexpr char16_t UTF16_SUR_LOW_START  = u'\xDC00';
static constexpr char16_t UTF16_SUR_LOW_END    = u'\xDFFF';

static constexpr char32_t UTF16_SURROGATE_OFFSET = (0xD800 << 10) + 0xDC00 - 0x10000;
static constexpr char32_t UTF16_LEAD_OFFSET = 0xD800 - (0x10000 >> 10);

}
