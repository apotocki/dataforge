/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cassert>
#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ET>
struct grouped_qrk : cvt_qrk<void>
{
    size_t group_sz;
    size_t row_sz;
    std::span<const ET> prefix;
    std::span<const ET> suffix;
    std::span<const ET> delim;
    std::span<const ET> rowdelim;

    template <Span PrefixT, Span SuffixT, Span DelimT, Span RowDelimT>
    grouped_qrk(
        size_t gsz,
        PrefixT prefix_value,
        SuffixT suffix_value,
        DelimT delim_value,
        size_t rsz,
        RowDelimT rowdelim_value)
            : group_sz{ gsz }
            , row_sz{ rsz }
            , prefix{ std::span{ reinterpret_cast<const ET*>(prefix_value.data()), prefix_value.size() } }
            , suffix{ std::span{ reinterpret_cast<const ET*>(suffix_value.data()), suffix_value.size() } }
            , delim{ std::span{ reinterpret_cast<const ET*>(delim_value.data()), delim_value.size() } }
            , rowdelim{ std::span{ reinterpret_cast<const ET*>(rowdelim_value.data()), rowdelim_value.size() } }
    {
        assert(gsz);
        assert(row_sz);
    }
};

template <
    SpanConvertible PrefixT = cchar_span_t,
    SpanConvertible SuffixT = cchar_span_t,
    SpanConvertible DelimT = cchar_span_t,
    SpanConvertible RowDelimT = cchar_span_t>
inline grouped_qrk<char> grouped_chars(
    size_t gsz,
    PrefixT && prefix = "0x"_sp,
    SuffixT && suffix = ""_sp,
    DelimT && delim = ", "_sp,
    size_t row_sz = (std::numeric_limits<size_t>::max)(),
    RowDelimT rowdelim = ",\n"_sp)
{
    return grouped_qrk<char>(gsz,
        std::span{ std::forward<PrefixT>(prefix) },
        std::span{ std::forward<SuffixT>(suffix) },
        std::span{ std::forward<DelimT>(delim) },
        row_sz,
        std::span{ std::forward<RowDelimT>(rowdelim) });
}

}

#include "../detail/basic/grouper.hpp"
