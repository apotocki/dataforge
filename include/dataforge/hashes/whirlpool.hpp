/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

struct whirlpool_qrk : cvt_qrk<void> {};

inline compound_qrk<whirlpool_qrk, int_qrk<8, void>> whirlpool;

}

#include "../detail/hashes/whirlpool.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, whirlpool_qrk>
{
    using type = digest_generic_pusher<whirlpool_detail::whirlpool_impl>;
};

}
