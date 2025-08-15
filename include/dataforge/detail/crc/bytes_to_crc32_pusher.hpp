/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "crc.hpp"

namespace dataforge {

class bytes_to_crc32_pusher
    : public crc_detail::bytes_to_crc_pusher_base<uint32_t>
    , public generic_pusher<void>
{
public:
    template <typename SrcTagT>
    bytes_to_crc32_pusher(SrcTagT const&, crc_custom_qrk<32> const& quark)
    {
        generate_table(quark.polynomial, quark.reflectin, quark.reflectout, quark.init, quark.out);
    }

    template <typename SrcTagT>
    bytes_to_crc32_pusher(SrcTagT const&, crc_qrk<crc32_type> const& quark)
    {
        switch (quark.type) {
        case crc32_type::DEFAULT:
            generate_table(0x4C11DB7, true, true, 0xFFFFFFFF, 0xFFFFFFFF); break;
        case crc32_type::BZIP2:
            generate_table(0x4C11DB7, false, false, 0xFFFFFFFF, 0xFFFFFFFF); break;
        case crc32_type::C:
            generate_table(0x1EDC6F41, true, true, 0xFFFFFFFF, 0xFFFFFFFF); break;
        case crc32_type::D:
            generate_table(0xA833982B, true, true, 0xFFFFFFFF, 0xFFFFFFFF); break;
        case crc32_type::MPEG2:
            generate_table(0x4C11DB7, false, false, 0xFFFFFFFF, 0); break;
        case crc32_type::POSIX:
            generate_table(0x4C11DB7, false, false, 0, 0xFFFFFFFF); break;
        case crc32_type::Q:
            generate_table(0x814141AB, false, false, 0, 0); break;
        case crc32_type::JAMCRC:
            generate_table(0x4C11DB7, true, true, 0xFFFFFFFF, 0); break;
        case crc32_type::XFER:
            generate_table(0xAF, false, false, 0, 0); break;
        }
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_qrk<crc32_type>>
{
    using type = bytes_to_crc32_pusher;
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_custom_qrk<32>>
{
    using type = bytes_to_crc32_pusher;
};

}
