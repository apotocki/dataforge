/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

static constexpr char32_t UTF16_SUR_HIGH_START = 0xD800;
static constexpr char32_t UTF16_SUR_HIGH_END = 0xDBFF;
static constexpr char32_t UTF16_SUR_LOW_START = 0xDC00;
static constexpr char32_t UTF16_SUR_LOW_END = 0xDFFF;

static constexpr char32_t UTF16_SURROGATE_OFFSET = (0xD800 << 10) + 0xDC00 - 0x10000;
static constexpr char32_t UTF16_LEAD_OFFSET = 0xD800 - (0x10000 >> 10);

}
