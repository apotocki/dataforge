/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "base.hpp"

namespace dataforge {

enum class base58_type
{
    BITCOIN, IPFS = BITCOIN, RIPPLE, FLICKR, GMP
};

template <bool CheckV, typename ErrorHandlerT = void>
struct base58_qrk : base_qrk<ErrorHandlerT>
{
    using base_t = base_qrk<ErrorHandlerT>;

    template <typename ... EHArgTs>
    base58_qrk(base58_type t, EHArgTs&& ... ehargs)
        : base_t{ get_alphabet(t), std::span<const char>{}, std::forward<EHArgTs>(ehargs) ... }
    { }

    static std::span<const char, 58> get_alphabet(base58_type t)
    {
        using span_t = std::span<const char, 58>;
        switch (t) {
        case base58_type::BITCOIN:
            return span_t{ "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz", 58 };
        case base58_type::RIPPLE:
            return span_t{ "rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz", 58 };
        case base58_type::FLICKR:
            return span_t{ "123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ", 58 };
        case base58_type::GMP:
            return span_t{ "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuv", 58 };
        default:
            throw std::runtime_error("unknown base58 type");
        }
    }
};

template <bool CheckV, typename ErrorHandlerT>
inline base58_qrk<CheckV, ErrorHandlerT> base58(base58_type t, ErrorHandlerT eh)
{
    return base58_qrk<CheckV, ErrorHandlerT>{ t, std::move(eh) };
}

template <bool CheckV>
inline base58_qrk<CheckV, throw_error_handler> base58(base58_type t)
{
    return base58_qrk<CheckV, throw_error_handler>{ t };
}

inline base58_qrk<false, throw_error_handler> base58(base58_type t)
{
    return base58_qrk<false, throw_error_handler>(t);
}

inline base58_qrk<true, throw_error_handler> base58check(base58_type t)
{
    return base58_qrk<true, throw_error_handler>(t);
}

}

#include "../detail/base_xx/bytes_to_base58_pusher.hpp"
#include "../detail/base_xx/base58_to_bytes_pusher.hpp"