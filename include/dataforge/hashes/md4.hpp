/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

struct md4_qrk : cvt_qrk<void> {};

inline compound_qrk<md4_qrk, int_qrk<8, void>> md4;

}

#include "../detail/hashes/md4.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, md4_qrk>
{
    using type = digest_generic_pusher<md4_detail::md4_impl>;
};

}
