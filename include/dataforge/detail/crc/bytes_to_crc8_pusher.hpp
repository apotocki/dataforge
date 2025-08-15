/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "crc.hpp"

namespace dataforge {

class bytes_to_crc8_pusher 
    : public crc_detail::bytes_to_crc_pusher_base<uint8_t>
    , public generic_pusher<void>
{
public:
    
    template <typename SrcTagT>
    bytes_to_crc8_pusher(SrcTagT const&, crc_custom_qrk<8> const& quark)
    {
        generate_table(quark.polynomial, quark.reflectin, quark.reflectout, quark.init, quark.out);
    }

    template <typename SrcTagT>
    bytes_to_crc8_pusher(SrcTagT const&, crc_qrk<crc8_type> const& quark)
    {
        switch (quark.type) {
        case crc8_type::DEFAULT:
            generate_table(0x7, false, false, 0, 0); break;
        case crc8_type::CDMA2000:
            generate_table(0x9B, false, false, 0xff, 0); break;
        case crc8_type::DARC:
            generate_table(0x39, true, true, 0, 0); break;
        case crc8_type::DVB_S2:
            generate_table(0xD5, false, false, 0, 0); break;
        case crc8_type::EBU:
            generate_table(0x1D, true, true, 0xFF, 0); break;
        case crc8_type::I_CODE:
            generate_table(0x1D, false, false, 0xFD, 0); break;
        case crc8_type::ITU:
            generate_table(0x07, false, false, 0, 0x55); break;
        case crc8_type::MAXIM:
            generate_table(0x31, true, true, 0, 0); break;
        case crc8_type::ROHC:
            generate_table(0x07, true, true, 0xFF, 0); break;
        case crc8_type::WCDMA:
            generate_table(0x9B, true, true, 0, 0); break;
        }
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_qrk<crc8_type>>
{
    using type = bytes_to_crc8_pusher;
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_custom_qrk<8>>
{
    using type = bytes_to_crc8_pusher;
};

}
