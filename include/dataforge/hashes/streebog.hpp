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

struct streebog_qrk : cvt_qrk<void>
{
    uint_least16_t hash_size;

    explicit streebog_qrk(unsigned int hash_size_val)
        : hash_size{ static_cast<uint_least16_t>(hash_size_val) }
    {
        assert(hash_size == 256 || hash_size == 512);
    }
};

inline compound_qrk<streebog_qrk, int_qrk<8, void >> streebog256{ 256u, nullptr };
inline compound_qrk<streebog_qrk, int_qrk<8, void >> streebog512{ 512u, nullptr };

}


#include "../detail/hashes/streebog.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, streebog_qrk>
{
    using type = digest_generic_pusher<streebog_detail::streebog_impl>;
};

}
