/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "crc.hpp"

namespace dataforge {

class bytes_to_crc16_pusher
    : public crc_detail::bytes_to_crc_pusher_base<uint16_t> 
    , public generic_pusher<void>
{
public:
    template <typename SrcTagT>
    bytes_to_crc16_pusher(SrcTagT const&, crc_custom_qrk<16> const& quark)
    {
        generate_table(quark.polynomial, quark.reflectin, quark.reflectout, quark.init, quark.out);
    }

    template <typename SrcTagT>
    bytes_to_crc16_pusher(SrcTagT const&, crc_qrk<crc16_type> const& quark)
    {
        switch (quark.type) {
        case crc16_type::CCITT_FALSE:
            generate_table(0x1021, false, false, 0xFFFF, 0); break;
        case crc16_type::ARC:
            generate_table(0x8005, true, true, 0, 0); break;
        case crc16_type::AUG_CCITT:
            generate_table(0x1021, false, false, 0x1D0F, 0); break;
        case crc16_type::BUYPASS:
            generate_table(0x8005, false, false, 0, 0); break;
        case crc16_type::CDMA2000:
            generate_table(0xC867, false, false, 0xFFFF, 0); break;
        case crc16_type::DDS_110:
            generate_table(0x8005, false, false, 0x800D, 0); break;
        case crc16_type::DECT_R:
            generate_table(0x0589, false, false, 0, 1); break;
        case crc16_type::DECT_X:
            generate_table(0x0589, false, false, 0, 0); break;
        case crc16_type::DNP:
            generate_table(0x3D65, true, true, 0, 0xFFFF); break;
        case crc16_type::EN_13757:
            generate_table(0x3D65, false, false, 0, 0xFFFF); break;
        case crc16_type::GENIBUS:
            generate_table(0x1021, false, false, 0xFFFF, 0xFFFF); break;
        case crc16_type::MAXIM:
            generate_table(0x8005, true, true, 0, 0xFFFF); break;
        case crc16_type::MCRF4XX:
            generate_table(0x1021, true, true, 0xFFFF, 0); break;
        case crc16_type::RIELLO:
            generate_table(0x1021, true, true, 0xB2AA, 0); break;
        case crc16_type::T10_DIF:
            generate_table(0x8BB7, false, false, 0, 0); break;
        case crc16_type::TELEDISK:
            generate_table(0xA097, false, false, 0, 0); break;
        case crc16_type::TMS37157:
            generate_table(0x1021, true, true, 0x89EC, 0); break;
        case crc16_type::USB:
            generate_table(0x8005, true, true, 0xFFFF, 0xFFFF); break;
        case crc16_type::A:
            generate_table(0x1021, true, true, 0xC6C6, 0); break;
        case crc16_type::KERMIT:
            generate_table(0x1021, true, true, 0, 0); break;
        case crc16_type::MODBUS:
            generate_table(0x8005, true, true, 0xFFFF, 0); break;
        case crc16_type::X_25:
            generate_table(0x1021, true, true, 0xFFFF, 0xFFFF); break;
        case crc16_type::XMODEM:
            generate_table(0x1021, false, false, 0, 0); break;
        }
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_qrk<crc16_type>>
{
    using type = bytes_to_crc16_pusher;
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_custom_qrk<16>>
{
    using type = bytes_to_crc16_pusher;
};

}
