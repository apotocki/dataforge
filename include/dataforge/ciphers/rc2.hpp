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

template <size_t MaxEffectiveKeyLengthV = 1024, typename ErrorHandlerT = throw_error_handler>
struct rc2_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;

    static_assert(MaxEffectiveKeyLengthV > 0);
    static constexpr size_t max_effective_keylength = MaxEffectiveKeyLengthV;

    size_t effective_keylength;
    cbyte_span_t key;
    cbyte_span_t iv;
    padding_type pt;
    cipher_mode_type cmt;
    
    template <SpanOfIntegrals<8> KT, SpanOfIntegrals<8> IVT, typename ... EHArgTs>
    rc2_qrk(KT key_val, size_t t1_val, cipher_mode_type cmt_val, IVT iv_val, padding_type pt_val, EHArgTs&& ... ehargs)
        : cvt_qrk_t{ std::forward<EHArgTs>(ehargs) ... }
        , key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
        , iv{ reinterpret_cast<const unsigned char*>(iv_val.data()), iv_val.size() }
        , pt { pt_val }
        , cmt { cmt_val }
        , effective_keylength{ t1_val }
    {
        assert(t1_val <= MaxEffectiveKeyLengthV);
        assert(key.size() <= (MaxEffectiveKeyLengthV + 7) / 8);
    }
};

template <SpanConvertible KeyT, SpanConvertible IVT = cbyte_span_t>
auto rc2(
    KeyT&& key,
    cipher_mode_type cmt = cipher_mode_type::ECB,
    IVT&& iv = {},
    padding_type pt = padding_type::zero)
{
    return rc2_qrk<>{ std::span{ std::forward<KeyT>(key) }, std::size(key) * 8, cmt, std::span{ std::forward<IVT>(iv) }, pt };
}

}

#include "../detail/ciphers/rc2.hpp"
#include "../detail/utility/basic_block_cipher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT, size_t MaxEffectiveKeyLengthV, typename ErrorHandlerT>
struct cvt_resolver<FromQuarkT, rc2_qrk<MaxEffectiveKeyLengthV, ErrorHandlerT>>
{
    using type = generic_block_encrypter<shared_algorithm<basic_block_cipher<rc2_cipher_type_factory>>, ErrorHandlerT>;
};

template <IntegralBasedQuark<8> DestTagT, size_t MaxEffectiveKeyLengthV, typename ErrorHandlerT>
struct cvt_resolver<rc2_qrk<MaxEffectiveKeyLengthV, ErrorHandlerT>, DestTagT>
{
    using type = generic_block_decrypter<shared_algorithm<basic_block_cipher<rc2_cipher_type_factory>>, ErrorHandlerT>;
};

}
