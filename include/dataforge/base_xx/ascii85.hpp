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
struct ascii85_qrk : int_qrk<8, ErrorHandlerT>
{
    using cvt_quark_t = int_qrk<8, ErrorHandlerT>;

    bool y_abbr = false;

    template <typename ... EHArgTs>
    explicit ascii85_qrk(bool y_abbr_val = false, EHArgTs && ... ehargs)
        : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }, y_abbr{ y_abbr_val }
    {}
};

inline ascii85_qrk<throw_error_handler> ascii85;
inline ascii85_qrk<throw_error_handler> ascii85_with_y{ true };

template <typename ErrorHandlerT>
inline ascii85_qrk<ErrorHandlerT> ascii85_weh(bool y_abbr, ErrorHandlerT err)
{
    return ascii85_qrk<ErrorHandlerT>{ y_abbr, std::move(err) };
}

}

#include "../detail/base_xx/bytes_to_ascii85_pusher.hpp"
#include "../detail/base_xx/ascii85_to_bytes_pusher.hpp"
