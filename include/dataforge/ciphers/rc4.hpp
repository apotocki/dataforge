/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"

#include "../detail/ciphers/blowfish.hpp"

namespace dataforge {

struct rc4_qrk : int_qrk<8, void>
{
    std::span<const unsigned char> key;
    size_t skipsz;
    uint_least8_t n; // size of S block in bits: [1-16] (8 by default)
    
    template <Integral<8> KET, size_t EV>
    inline explicit rc4_qrk(std::span<KET, EV> key_val, uint_least8_t nval = 8, size_t skipsz_val = 0)
        : key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
        , skipsz{ skipsz_val }
        , n { nval }
    {
        assert(nval >= 1 && nval <= 16);
    }

    inline explicit rc4_qrk(std::span<const unsigned char> key_val, uint_least8_t nval = 8, size_t skipsz_val = 0)
        : key{ reinterpret_cast<const unsigned char*>(key_val.data()), key_val.size() }
        , skipsz{ skipsz_val }
        , n{ nval }
    {
        assert(nval >= 1 && nval <= 16);
    }
};

}

#include "../detail/ciphers/rc4_crypter.hpp"
