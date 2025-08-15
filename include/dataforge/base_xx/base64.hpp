/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ErrorHandlerT = void>
struct base64_qrk : int_qrk<8, ErrorHandlerT>
{
    using cvt_quark_t = int_qrk<8, ErrorHandlerT>;

    const char paddingCV;

    template <typename ... EHArgTs>
    explicit base64_qrk(char padding = '=', EHArgTs&& ... ehargs)
        : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }, paddingCV{ padding }
    {}
};

inline base64_qrk<throw_error_handler> base64;

template <typename ErrorHandlerT>
inline base64_qrk<ErrorHandlerT> base64_weh(ErrorHandlerT err) {
    return base64_qrk<ErrorHandlerT>{ '=', std::move(err) };
}

inline base64_qrk<throw_error_handler> base64f(char p)
{
    return base64_qrk<throw_error_handler>(p);
}

}

#include "../detail/base_xx/bytes_to_base64.hpp"
#include "../detail/base_xx/base64_to_bytes.hpp"
