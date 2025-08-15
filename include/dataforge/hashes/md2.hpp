/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

struct md2_qrk : cvt_qrk<void> {};

inline compound_qrk<md2_qrk, int_qrk<8, void>> md2;

}

#include "../detail/hashes/md2.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, md2_qrk>
{
    using type = digest_generic_pusher<md2_detail::md2_impl>;
};

}
