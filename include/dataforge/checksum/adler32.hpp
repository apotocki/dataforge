/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "dataforge/detail/quarks.hpp"

namespace dataforge {

struct adler32_qrk : cvt_qrk<void> {};

inline adler32_qrk adler32;

}

#include "../detail/adler32/bytes_to_adler32_pusher.hpp"
