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
struct md6_qrk : cvt_qrk<ErrorHandlerT>
{
    int bitsize;
    int rounds;
    int l;
    cbyte_span_t key;

    template <SpanOfIntegrals<8> KT = cbyte_span_t>
    explicit md6_qrk(int bitsize_val, int rounds_val = 0, KT key_val = {}, int l_val = -1, ErrorHandlerT const& eh = ErrorHandlerT{})
        : cvt_qrk<ErrorHandlerT>{ eh }, bitsize{ bitsize_val }, rounds{ rounds_val }
        , l{ l_val }, key{ key_val }
    {}
};

template <SpanConvertible KT = cbyte_span_t>
inline auto md6(int bitsize, int rounds = 0, KT && key = {}, int l = -1)
{
    return compound_qrk<md6_qrk<throw_error_handler>, int_qrk<8, void>>(
        md6_qrk<throw_error_handler>{bitsize, rounds, std::span{std::forward<KT>(key)}, l},
        nullptr
    );
}

}

#include "../detail/hashes/bytes_to_md6_pusher.hpp"
