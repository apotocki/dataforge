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
struct kuznyechik_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;

    cbyte_span_t key;
    cbyte_span_t iv;
    padding_type pt;
    cipher_mode_type cmt;

    template <SpanOfIntegrals<8> KT, SpanOfIntegrals<8> IVT = cbyte_span_t, typename ... EHArgTs>
    kuznyechik_qrk(
        KT key_val,
        cipher_mode_type cmt_val = cipher_mode_type::ECB,
        IVT iv_val = {},
        padding_type pt_val = padding_type::zero,
        EHArgTs&& ... ehargs)
            : cvt_qrk_t{ std::forward<EHArgTs>(ehargs) ... }
            , key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
            , iv{ reinterpret_cast<const unsigned char*>(iv_val.data()), iv_val.size() }
            , pt{ pt_val }
            , cmt{ cmt_val }
    {}
};

template <SpanConvertible KT, SpanConvertible IVT = cbyte_span_t>
auto kuznyechik(
    KT&& key,
    cipher_mode_type cmt = cipher_mode_type::ECB,
    IVT&& iv = {},
    padding_type pt = padding_type::zero)
{
    return kuznyechik_qrk<>{ std::span{ key }, cmt, std::span{ iv }, pt };
}

}

#include "../detail/ciphers/kuznyechik.hpp"
#include "../detail/utility/basic_block_cipher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQrkT, typename ErrorHandlerT>
struct cvt_resolver<FromQrkT, kuznyechik_qrk<ErrorHandlerT>>
{
    using type = generic_block_encrypter<shared_algorithm<basic_block_cipher<kuznyechik_cipher_type_factory>>, ErrorHandlerT>;
};

template <IntegralBasedQuark<8> DestQrkT, typename ErrorHandlerT>
struct cvt_resolver<kuznyechik_qrk<ErrorHandlerT>, DestQrkT>
{
    using type = generic_block_decrypter<shared_algorithm<basic_block_cipher<kuznyechik_cipher_type_factory>>, ErrorHandlerT>;
};

}
