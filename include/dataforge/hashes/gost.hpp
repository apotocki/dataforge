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

struct gost_qrk : cvt_qrk<void>
{
    bool crypto_pro_sbox;
    explicit gost_qrk(bool crypto_pro_sbox_val)
        : crypto_pro_sbox { crypto_pro_sbox_val }
    {}
};

inline compound_qrk<gost_qrk, int_qrk<8, void >> gost{ false, nullptr };
inline compound_qrk<gost_qrk, int_qrk<8, void >> gost_cpro{ true, nullptr };

}

#include "../detail/hashes/gost.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, gost_qrk>
{
    using type = digest_generic_pusher<gost_detail::gost_impl>;
};

}
