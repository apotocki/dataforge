/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

#include "../detail/hashes/ripemd.hpp"

namespace dataforge {

template <ripemd_type Type>
struct ripemd_qrk : cvt_qrk<void> {};

inline compound_qrk<ripemd_qrk<ripemd_type::ripemd128>, int_qrk<8, void>> ripemd128;
inline compound_qrk<ripemd_qrk<ripemd_type::ripemd160>, int_qrk<8, void>> ripemd160;
inline compound_qrk<ripemd_qrk<ripemd_type::ripemd256>, int_qrk<8, void>> ripemd256;
inline compound_qrk<ripemd_qrk<ripemd_type::ripemd320>, int_qrk<8, void>> ripemd320;

}

#include "../detail/hashes/ripemd.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, ripemd_type TypeV>
struct cvt_resolver<FromQuarkT, ripemd_qrk<TypeV>>
{
    using type = digest_generic_pusher<ripemd_detail::ripemd_impl<TypeV>>;
};

}
