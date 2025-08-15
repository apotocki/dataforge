/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
struct deflate_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;

    int compression_level; // 0-9; -1 = default
    uint_least32_t buffer_size : 31;
    uint_least32_t gzip: 1;

    explicit deflate_qrk(bool gzip_val, uint_least32_t bsz, int clevel = -1, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_qrk_t{ eh }
        , compression_level{ clevel }
        , buffer_size{ bsz }
        , gzip{ gzip_val ? 1u : 0u }
    {}
};

template <typename ErrorHandlerT>
struct inflate_qrk : int_qrk<8, ErrorHandlerT>
{
    using cvt_quark_t = int_qrk<8, ErrorHandlerT>;

    uint_least32_t buffer_size : 31;
    uint_least32_t gzip : 1;

    explicit inflate_qrk(bool gzip_val, uint_least32_t bsz, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , buffer_size{ bsz }
        , gzip{ gzip_val ? 1u : 0u }
    {}
};

inline deflate_qrk<throw_error_handler> deflated(bool gzip = false, uint_least32_t buffer_size = 65536, int compression_level = -1)
{
    return deflate_qrk<throw_error_handler>{gzip, buffer_size, compression_level};
}

inline compound_qrk<inflate_qrk<throw_error_handler>, int_qrk<8, void>> inflated(bool gzip = false, uint_least32_t buffer_size = 65536)
{
    return {inflate_qrk<throw_error_handler>(gzip, buffer_size), {}};
}

}

#include "../detail/compression/deflater.hpp"
#include "../detail/compression/inflater.hpp"
