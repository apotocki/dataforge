/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
struct bzip2_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;

    uint_least32_t buffer_size;
    uint_least8_t block_size100k;
    uint_least8_t work_factor;

    explicit bzip2_qrk(uint_least32_t bsz, uint_least8_t blsz, uint_least8_t wf, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , buffer_size { bsz }
        , block_size100k{ blsz }
        , work_factor{ wf }
    {
    }
};

inline bzip2_qrk<throw_error_handler> bzip2(uint_least32_t buffer_size = 65536, uint_least8_t block_size100k = 9, uint_least8_t work_factor = 0)
{
    return bzip2_qrk<throw_error_handler>(buffer_size, block_size100k, work_factor);
}

}

#include "../detail/compression/bzip2_compressor.hpp"
#include "../detail/compression/bzip2_extractor.hpp"