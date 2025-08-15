/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

#include "defs.hpp"

#include "../detail/ciphers/blowfish.hpp"

namespace dataforge {

template <typename ErrorHandlerT = throw_error_handler>
struct blowfish_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;

    cbyte_span_t key;
    cbyte_span_t iv;
    padding_type pt;
    cipher_mode_type cmt;
    bool compat_mode;

    template <SpanOfIntegrals<8> KT, SpanOfIntegrals<8> IVT = cbyte_span_t, typename ... EHArgTs>
    blowfish_qrk(
        bool cm_val,
        KT key_val,
        cipher_mode_type cmt_val = cipher_mode_type::ECB,
        IVT iv_val = {},
        padding_type pt_val = padding_type::zero,
        EHArgTs&& ... ehargs)
            : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }
            , key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
            , iv{ reinterpret_cast<const unsigned char*>(iv_val.data()), iv_val.size() }
            , pt{ pt_val }
            , cmt{ cmt_val }
            , compat_mode{ cm_val }
    {}
};

template <SpanConvertible KT, SpanConvertible IVT = cbyte_span_t>
auto blowfish(
    bool cm,
    KT&& key,
    cipher_mode_type cmt = cipher_mode_type::ECB,
    IVT&& iv = {},
    padding_type pt = padding_type::zero)
{
    return blowfish_qrk<>{ cm, std::span{ key }, cmt, std::span{ iv }, pt };
}

}

#include "../detail/ciphers/blowfish.hpp"
#include "../detail/utility/basic_block_cipher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, typename ErrorHandlerT>
struct cvt_resolver<FromQuarkT, blowfish_qrk<ErrorHandlerT>>
{
    using type = generic_block_encrypter<shared_algorithm<basic_block_cipher<blowfish_cipher_type_factory>>, ErrorHandlerT>;
};

template <IntegralBasedQuark<8> DestTagT, typename ErrorHandlerT>
struct cvt_resolver<blowfish_qrk<ErrorHandlerT>, DestTagT>
{
    using type = generic_block_decrypter<shared_algorithm<basic_block_cipher<blowfish_cipher_type_factory>>, ErrorHandlerT>;
};

}
