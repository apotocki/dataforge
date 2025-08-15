/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ET>
struct buffer_qrk : cvt_qrk<void>
{
    size_t buffer_sz;

    explicit buffer_qrk(size_t sz)
        : buffer_sz{ sz }
    {
        assert(sz);
    }
};

template <typename ET>
inline buffer_qrk<ET> buffer(size_t buffer_sz)
{
    return buffer_qrk<ET>(buffer_sz);
}

}

#include "../detail/basic/buffer_pusher.hpp"
