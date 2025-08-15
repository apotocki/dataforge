/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>

namespace dataforge {

enum class padding_type : uint_least8_t
{
    none,
    zero,
    pkcs
};

// 1st bit signals the no need for a padding
enum class cipher_mode_type : uint_least8_t
{
    ECB = 0,
    CBC = 2,
    CFB = 3,
    OFB = 5,
    CTR = 7,
    PCBC = 8
};

}
