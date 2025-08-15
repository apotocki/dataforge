/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

#include "defs.hpp"

namespace dataforge {

template <typename ErrorHandlerT = throw_error_handler>
struct aes_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;

    cbyte_span_t key;
    cbyte_span_t iv;
    int blocksize_in_bits;
    padding_type pt;
    cipher_mode_type cmt;

    template <SpanOfIntegrals<8> KT, SpanOfIntegrals<8> IVT, typename ... EHArgTs>
    aes_qrk(int bit_sz_val, KT key_val, cipher_mode_type cmt_val, IVT iv_val, padding_type pt_val, EHArgTs&& ... ehargs)
        : cvt_qrk_t{ std::forward<EHArgTs>(ehargs) ... }
        , key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
        , iv{ reinterpret_cast<const unsigned char*>(iv_val.data()), iv_val.size() }
        , blocksize_in_bits{ bit_sz_val }
        , pt { pt_val }
        , cmt { cmt_val }
    {}
};

template <SpanConvertible KeyT, SpanConvertible IVT = cbyte_span_t>
auto aes(
    int bit_sz,
    KeyT&& key,
    cipher_mode_type cmt = cipher_mode_type::ECB,
    IVT&& iv = {},
    padding_type pt = padding_type::zero)
{
    return aes_qrk<>{ bit_sz, std::span{ std::forward<KeyT>(key) }, cmt, std::span{ std::forward<IVT>(iv) }, pt };
}

}

#include "../detail/ciphers/aes.hpp"
#include "../detail/utility/basic_block_cipher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQrkT, typename ErrorHandlerT>
struct cvt_resolver<FromQrkT, aes_qrk<ErrorHandlerT>>
{
    using type = generic_block_encrypter<shared_algorithm<basic_block_cipher<aes_cipher_type_factory>>, ErrorHandlerT>;
};

template <IntegralBasedQuark<8> DestQrkT, typename ErrorHandlerT>
struct cvt_resolver<aes_qrk<ErrorHandlerT>, DestQrkT>
{
    using type = generic_block_decrypter<shared_algorithm<basic_block_cipher<aes_cipher_type_factory>>, ErrorHandlerT>;
};

}
