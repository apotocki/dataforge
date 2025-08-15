/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"
#include "../detail/compression/lzma.hpp"

namespace dataforge {

enum class lzma_type
{
    lzma1,
    lzma2
};

template <lzma_type TV, typename ErrorHandlerT>
struct lzma_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;
    uint32_t preset;
    uint_least32_t buffer_size;
    lzma_check check; // LZMA_CHECK_NONE or LZMA_CHECK_CRC32 or LZMA_CHECK_CRC64(default) or LZMA_CHECK_SHA256
    const char* filterscfg;
    explicit lzma_qrk(const char* f, lzma_check c, uint_least32_t bsz, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , preset(LZMA_PRESET_DEFAULT)
        , filterscfg{ f }
        , check{ c }
        , buffer_size{ bsz }
    {
    }

    explicit lzma_qrk(uint32_t p, lzma_check c, uint_least32_t bsz, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , preset{ p }
        , check{ c }
        , buffer_size{ bsz }
        , filterscfg{ nullptr }
    {
    }
};

template <typename ErrorHandlerT>
struct lzma_qrk<lzma_type::lzma1, ErrorHandlerT> : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;
    uint32_t preset; // 0 is the fastest and 9 is the slowest, or LZMA_PRESET_EXTREME
    uint_least32_t buffer_size;
    explicit lzma_qrk(uint32_t p, uint_least32_t bsz, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_quark_t{ eh }
        , preset{ p }
        , buffer_size{ bsz }
    { }
};

inline lzma_qrk<lzma_type::lzma2, throw_error_handler> lzma2(uint32_t preset = LZMA_PRESET_DEFAULT, lzma_check check = LZMA_CHECK_CRC64, uint_least32_t buffer_size = 65536)
{
    return lzma_qrk<lzma_type::lzma2, throw_error_handler>(preset, check, buffer_size);
}

inline lzma_qrk<lzma_type::lzma2, throw_error_handler> lzma2f(const char* f = "lzma2:preset=6", lzma_check check = LZMA_CHECK_CRC64, uint_least32_t buffer_size = 65536)
{
    return lzma_qrk<lzma_type::lzma2, throw_error_handler>(f, check, buffer_size);
}

inline lzma_qrk<lzma_type::lzma1, throw_error_handler> lzma1(uint32_t preset = LZMA_PRESET_DEFAULT, uint_least32_t buffer_size = 65536)
{
    return lzma_qrk<lzma_type::lzma1, throw_error_handler>(preset, buffer_size);
}

}

#include "../detail/compression/lzma_compressor.hpp"
#include "../detail/compression/lzma_extractor.hpp"


