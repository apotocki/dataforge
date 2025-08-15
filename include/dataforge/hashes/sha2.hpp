/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/hashes/sha_qrk.hpp"
#include "../detail/hashes/sha2.hpp"

namespace dataforge {

inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha224>, int_qrk<8, void>> sha224;
inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha256>, int_qrk<8, void>> sha256;
inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha384>, int_qrk<8, void>> sha384;
inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha512>, int_qrk<8, void>> sha512;
inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha512_256>, int_qrk<8, void>> sha512_256;
inline compound_qrk<sha_qrk<sha2_type, sha2_type::sha512_224>, int_qrk<8, void>> sha512_224;

}

#include "../detail/hashes/sha2.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, sha2_type TypeV>
struct cvt_resolver<FromQuarkT, sha_qrk<sha2_type, TypeV>>
{
    using type = digest_generic_pusher<sha2_detail::sha2_impl<TypeV>>;
};

}
