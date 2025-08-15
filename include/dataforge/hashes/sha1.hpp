/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/hashes/sha_qrk.hpp"
#include "../detail/hashes/sha1.hpp"

namespace dataforge {

using sha1_qrk = sha_qrk<sha1_type, sha1_type::sha1>;
inline compound_qrk<sha1_qrk, int_qrk<8, void>> sha1;

}

#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, sha_qrk<sha1_type, sha1_type::sha1>>
{
    using type = digest_generic_pusher<sha1_detail::sha1_impl>;
};

}
