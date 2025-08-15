/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "crc.hpp"

namespace dataforge {

class bytes_to_crc64_pusher 
    : public crc_detail::bytes_to_crc_pusher_base<uint64_t>
    , public generic_pusher<void>
{
public:
    template <typename SrcTagT>
    bytes_to_crc64_pusher(SrcTagT const&, crc_custom_qrk<64> const& quark)
    {
        generate_table(quark.polynomial, quark.reflectin, quark.reflectout, quark.init, quark.out);
    }

    template <typename SrcTagT>
    bytes_to_crc64_pusher(SrcTagT const&, crc_qrk<crc64_type> const& quark)
    {
        switch (quark.type) {
        case crc64_type::DEFAULT:
            generate_table(0x42F0E1EBA9EA3693, false, false, 0, 0); break;
        case crc64_type::WE:
            generate_table(0x42F0E1EBA9EA3693, false, false, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull); break;
        case crc64_type::XZ:
            generate_table(0x42F0E1EBA9EA3693, true, true, 0xFFFFFFFFFFFFFFFFull, 0xFFFFFFFFFFFFFFFFull); break;
        }
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_qrk<crc64_type>>
{
    using type = bytes_to_crc64_pusher;
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, crc_custom_qrk<64>>
{
    using type = bytes_to_crc64_pusher;
};

}
