/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"
#include "../detail/hashes/blake.hpp"

namespace dataforge {

template <typename EnumT, EnumT type>
struct blake_qrk : cvt_qrk<void> {};

struct blake_qrk_32base
{
    std::array<uint_least32_t, 4> salt;
    blake_qrk_32base() { std::fill(salt.begin(), salt.end(), 0u); }
    explicit blake_qrk_32base(std::array<uint_least32_t, 4> s) : salt{s} {}
};

struct blake_qrk_64base
{
    std::array<uint_least64_t, 4> salt;
    blake_qrk_64base() { std::fill(salt.begin(), salt.end(), 0u); }
    explicit blake_qrk_64base(std::array<uint_least64_t, 4> s) : salt{ s } {}
};

template <>
struct blake_qrk<blake_type, blake_type::blake224> : cvt_qrk<void>, blake_qrk_32base
{
    using blake_qrk_32base::blake_qrk_32base;
};

template <>
struct blake_qrk<blake_type, blake_type::blake256> : cvt_qrk<void>, blake_qrk_32base
{
    using blake_qrk_32base::blake_qrk_32base;
};

template <>
struct blake_qrk<blake_type, blake_type::blake384> : cvt_qrk<void>, blake_qrk_64base
{
    using blake_qrk_64base::blake_qrk_64base;
};

template <>
struct blake_qrk<blake_type, blake_type::blake512> : cvt_qrk<void>, blake_qrk_64base
{
    using blake_qrk_64base::blake_qrk_64base;
};

template <blake2_type Type>
struct blake_qrk<blake2_type, Type> : cvt_qrk<void>
{
    cbyte_span_t key;

    blake_qrk() = default;

    template <SpanOfIntegrals<8> KT>
    explicit blake_qrk(KT key_val) : key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() } {}
};

using blake224_t = compound_qrk<blake_qrk<blake_type, blake_type::blake224>, int_qrk<8, void>>;
using blake256_t = compound_qrk<blake_qrk<blake_type, blake_type::blake256>, int_qrk<8, void>>;
using blake384_t = compound_qrk<blake_qrk<blake_type, blake_type::blake384>, int_qrk<8, void>>;
using blake512_t = compound_qrk<blake_qrk<blake_type, blake_type::blake512>, int_qrk<8, void>>;

inline blake224_t blake224;
inline blake256_t blake256;
inline blake384_t blake384;
inline blake512_t blake512;

using blake2s224_t = compound_qrk<blake_qrk<blake2_type, blake2_type::blake2s224>, int_qrk<8, void>>;
using blake2s256_t = compound_qrk<blake_qrk<blake2_type, blake2_type::blake2s256>, int_qrk<8, void>>;
using blake2b384_t = compound_qrk<blake_qrk<blake2_type, blake2_type::blake2b384>, int_qrk<8, void>>;
using blake2b512_t = compound_qrk<blake_qrk<blake2_type, blake2_type::blake2b512>, int_qrk<8, void>>;

inline blake2s224_t blake2s224;
inline blake2s256_t blake2s256;
inline blake2b384_t blake2b384;
inline blake2b512_t blake2b512;

}

#include "../detail/hashes/blake.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, typename EnumT, EnumT TypeV>
struct cvt_resolver<FromQuarkT, blake_qrk<EnumT, TypeV>>
{
    using type = digest_generic_pusher<blake_detail::blake_impl<EnumT, TypeV>>;
};

}
