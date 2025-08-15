/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ErrorHandlerT = void>
struct base16_qrk : int_qrk<8, ErrorHandlerT>
{
    using cvt_quark_t = int_qrk<8, ErrorHandlerT>;

    bool ulet;

    template <typename ... EHArgTs>
    requires (!std::is_void_v<ErrorHandlerT> || 0 == sizeof ... (EHArgTs))
    explicit base16_qrk(bool is_upper_letters = true, EHArgTs&& ... ehargs)
        : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }, ulet{ is_upper_letters }
    {}
};

inline base16_qrk<> base16u{ true };
inline base16_qrk<> base16l{ false };

template <typename ErrorHandlerT>
inline base16_qrk<ErrorHandlerT> base16(bool is_upper_letters, ErrorHandlerT eh)
{
    return base16_qrk<ErrorHandlerT>(is_upper_letters, std::move(eh));
}

}

#include "../detail/base_xx/bytes_to_base16_pusher.hpp"
#include "../detail/base_xx/base16_to_bytes.hpp"
