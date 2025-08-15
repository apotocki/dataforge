/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"
#include "../detail/compression/lz4.hpp"

namespace dataforge {

template <typename ErrorHandlerT>
struct lz4_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;

    LZ4F_blockSizeID_t blockSizeID;
    size_t buffer_size;
    int compression_level;
    int block_linked: 1;
    int checksum : 1;
    int block_checksum : 1;
    explicit lz4_qrk(int level, size_t bsz, LZ4F_blockSizeID_t bszid, bool bl, bool cs, bool bcs, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , blockSizeID{ bszid }
        , buffer_size{ bsz }
        , compression_level { level }
        , block_linked { bl ? 1 : 0 }
        , checksum{ cs ? 1 : 0 }
        , block_checksum{ bcs ? 1 : 0 }
    {
        assert(buffer_size >= LZ4F_HEADER_SIZE_MAX);
    }
};

/*
    compression_level: 0: default (fast mode); values > LZ4HC_CLEVEL_MAX count as LZ4HC_CLEVEL_MAX; values < 0 trigger "fast acceleration"
    (according to lz4hc.h LZ4HC_CLEVEL_MAX = 12)
*/
inline lz4_qrk<throw_error_handler> lz4(int compression_level = 0, size_t buffer_size = (std::numeric_limits<size_t>::max)(), LZ4F_blockSizeID_t bszid = LZ4F_default, bool block_linked = true, bool checksum = false, bool block_checksum = false)
{
    return lz4_qrk<throw_error_handler>(compression_level, buffer_size, bszid, block_linked, checksum, block_checksum);
}

}

#include "../detail/compression/lz4_compressor.hpp"
#include "../detail/compression/lz4_extractor.hpp"
