/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "dataforge/detail/quarks.hpp"

namespace dataforge {

struct bsd_checksum_qrk : cvt_qrk<void> {};

inline bsd_checksum_qrk bsd_checksum;

}

#include "../detail/bsd_checksum/bytes_to_bsd_checksum_pusher.hpp"
