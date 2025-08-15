/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <cstdint>
#include <type_traits>
#include <cassert>

#include "../detail/quarks.hpp"

namespace dataforge {

template <typename ErrorHandlerT = void>
struct base_qrk : int_qrk<8, ErrorHandlerT>
{
    using cvt_quark_t = int_qrk<8, ErrorHandlerT>;

    std::span<const char> alphabet;
    std::span<const unsigned char> matrix;

    template <size_t E1, Integral<8> MET, size_t E2, typename ... EHArgTs>
    requires (!std::is_void_v<ErrorHandlerT> || 0 == sizeof ... (EHArgTs))
    base_qrk(std::span<const char, E1> a, std::span<const MET, E2> m, EHArgTs&& ... ehargs)
        : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }, alphabet{ a }, matrix{ reinterpret_cast<const unsigned char*>(m.data()), m.size() }
    {
        assert(matrix.empty() || matrix.size() == 256);
    }

protected:
    template <typename ... EHArgTs>
    requires (!std::is_void_v<ErrorHandlerT> || 0 == sizeof ... (EHArgTs))
    explicit base_qrk(EHArgTs&& ... ehargs)
        : cvt_quark_t{ std::forward<EHArgTs>(ehargs) ... }
    {}
};

template <size_t E1>
inline base_qrk<> base(std::span<const char, E1> alphabet)
{
    return base_qrk<>(alphabet, std::span<const uint8_t>{});
}

template <size_t E1, typename MET, size_t E2>
requires (sizeof(MET) == 1 && std::is_integral_v<MET>)
inline base_qrk<> base(std::span<const char, E1> alphabet, std::span<const MET, E2> matrix)
{
    return base_qrk<>(alphabet, matrix);
}

}

#include "../detail/base_xx/bytes_to_generic_base_pusher.hpp"
#include "../detail/base_xx/generic_base_to_bytes_pusher.hpp"
