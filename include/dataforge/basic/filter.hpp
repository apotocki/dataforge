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

template <typename T, typename PredicateT>
struct filter_qrk : cvt_qrk<void>
{
    PredicateT predicate;

    template <typename PredicateArgT>
    explicit filter_qrk(PredicateArgT && p)
        : predicate{ std::forward<PredicateArgT>(p) }
    {}
};

template <std::integral T, typename PredicateT>
compound_qrk<filter_qrk<T, PredicateT>, int_qrk<sizeof(T) * CHAR_BIT, void>>
filter(PredicateT const& p)
{
    return { filter_qrk<T, PredicateT>{ p }, nullptr };
}

}

#include "../detail/basic/filter.hpp"
