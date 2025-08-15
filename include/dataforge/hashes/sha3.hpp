/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/hashes/sha_qrk.hpp"
#include "../detail/hashes/sha3.hpp"

namespace dataforge {

template <sha3_type type>
struct sha_qrk<sha3_type, type> : cvt_qrk<void>
{
    size_t dlength; // digest length in bits

    explicit sha_qrk(size_t dlength_val = bits())
        : dlength{ dlength_val }
    {}

    static constexpr size_t bits() {
        switch (type) {
        case sha3_type::shake_128: return 128u;
        case sha3_type::sha3_224: return 224u;
        case sha3_type::sha3_256: case sha3_type::shake_256: return 256u;
        case sha3_type::sha3_384: return 384u;
        case sha3_type::sha3_512: default: return 512u;
        }
    }

    static constexpr uint_least8_t pad() {
        switch (type) {
        case sha3_type::shake_128: case sha3_type::shake_256: return 0x1fu;
        default: return 0x6u;
        }
    }
};

inline compound_qrk<sha_qrk<sha3_type, sha3_type::sha3_224>, int_qrk<8, void>> sha3_224{ 224u, nullptr };
inline compound_qrk<sha_qrk<sha3_type, sha3_type::sha3_256>, int_qrk<8, void>> sha3_256{ 256u, nullptr };
inline compound_qrk<sha_qrk<sha3_type, sha3_type::sha3_384>, int_qrk<8, void>> sha3_384{ 384u, nullptr };
inline compound_qrk<sha_qrk<sha3_type, sha3_type::sha3_512>, int_qrk<8, void>> sha3_512{ 512u, nullptr };

inline auto shake_128(size_t d)
{ return compound_qrk<sha_qrk<sha3_type, sha3_type::shake_128>, int_qrk<8, void>>(d, nullptr); }

inline auto shake_256(size_t d)
{ return compound_qrk<sha_qrk<sha3_type, sha3_type::shake_256>, int_qrk<8, void>>(d, nullptr); }

}

#include "../detail/hashes/sha3.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, sha3_type TypeV>
struct cvt_resolver<FromQuarkT, sha_qrk<sha3_type, TypeV>>
{
    using type = digest_generic_pusher<sha3_detail::sha3_impl>;
};

}
