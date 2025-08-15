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

struct belt_hash_qrk : cvt_qrk<void>
{
    explicit belt_hash_qrk() {}
};

inline compound_qrk<belt_hash_qrk, int_qrk<8, void >> belt_hash{};

}

#include "../detail/hashes/belt.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, belt_hash_qrk>
{
    using type = digest_generic_pusher<belt_detail::belt_hash_impl>;
};

}
