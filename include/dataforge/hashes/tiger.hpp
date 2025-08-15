/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

namespace dataforge {

struct tiger_qrk : cvt_qrk<void>
{
    uint_least8_t bits;
    uint_least8_t pad;
    uint_least8_t pc;

    tiger_qrk(uint_least8_t bits_val, uint_least8_t pad_val, uint_least8_t pc_val)
        : bits{ bits_val }, pad{ pad_val }, pc{ pc_val }
    {}
};

inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger128_3{ tiger_qrk{128u, 1, 3}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger160_3{ tiger_qrk{160u, 1, 3}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger192_3{ tiger_qrk{192u, 1, 3}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger128_4{ tiger_qrk{128u, 1, 4}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger160_4{ tiger_qrk{160u, 1, 4}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger192_4{ tiger_qrk{192u, 1, 4}, nullptr };

inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger2_128{ tiger_qrk{128u, 0x80u, 3}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger2_160{ tiger_qrk{160u, 0x80u, 3}, nullptr };
inline compound_qrk<tiger_qrk, int_qrk<8, void>> tiger2_192{ tiger_qrk{192u, 0x80u, 3}, nullptr };

}

#include "../detail/hashes/tiger.hpp"
#include "../detail/hashes/digest_generic_pusher.hpp"

namespace dataforge {

template <IntegralBasedQuark<8> FromQuarkT>
struct cvt_resolver<FromQuarkT, tiger_qrk>
{
    using type = digest_generic_pusher<tiger_detail::tiger_impl>;
};

}
