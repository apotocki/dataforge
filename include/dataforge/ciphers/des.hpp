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
struct des_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;

    std::span<const unsigned char> key;
    std::span<const unsigned char> iv;
    padding_type pt;
    cipher_mode_type cmt;
    int levels;
    bool ed_mode;

    template <Integral<8> KET, size_t EV, Integral<8> IVET, size_t EV2, typename ... EHArgTs>
    des_qrk(int levels_val, std::span<KET, EV> key_val, cipher_mode_type cmt_val, std::span<IVET, EV2> iv_val, padding_type pt_val, bool ed_val, EHArgTs&& ... ehargs)
        : cvt_qrk_t{ std::forward<EHArgTs>(ehargs) ... }
        , key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
        , iv{ reinterpret_cast<const unsigned char*>(iv_val.data()), iv_val.size() }
        , pt { pt_val }
        , cmt { cmt_val }
        , levels { levels_val }
        , ed_mode{ ed_val }
    {}

    des_qrk(
        int levels_val,
        std::span<const unsigned char> key_val,
        cipher_mode_type cmt_val = cipher_mode_type::ECB,
        std::span<const unsigned char> iv_val = {},
        padding_type pt_val = padding_type::zero,
        bool ed_val = true)
            : key{ key_val }
            , iv{ iv_val }
            , pt{ pt_val }
            , cmt{ cmt_val }
            , levels{ levels_val }
            , ed_mode{ ed_val }
    {}
};

}

#include "../detail/ciphers/des.hpp"
#include "../detail/utility/basic_block_cipher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQrkT, typename ErrorHandlerT>
struct cvt_resolver<FromQrkT, des_qrk<ErrorHandlerT>>
{
    using type = generic_block_encrypter<shared_algorithm<basic_block_cipher<des_cipher_type_factory>>, ErrorHandlerT>;
};

template <IntegralBasedQuark<8> DestQrkT, typename ErrorHandlerT>
struct cvt_resolver<des_qrk<ErrorHandlerT>, DestQrkT>
{
    using type = generic_block_decrypter<shared_algorithm<basic_block_cipher<des_cipher_type_factory>>, ErrorHandlerT>;
};

}
