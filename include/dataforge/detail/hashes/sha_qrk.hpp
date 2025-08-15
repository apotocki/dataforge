/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../quarks.hpp"

namespace dataforge {

template <typename EnumT, EnumT type>
struct sha_qrk : cvt_qrk<void> {};

}
