/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "dataforge/detail/quarks.hpp"

namespace dataforge {

enum class crc8_type
{
    DEFAULT, CDMA2000, DARC, DVB_S2, EBU, I_CODE, ITU, MAXIM, ROHC, WCDMA
};

enum class crc16_type
{
    DEFAULT,
    ARC = DEFAULT, // CRC-16, CRC-16/LHA, CRC-IBM
    CCITT_FALSE, AUG_CCITT, BUYPASS, CDMA2000, DDS_110, DECT_R, DECT_X, DNP, EN_13757, GENIBUS, MAXIM, MCRF4XX,
    RIELLO, T10_DIF, TELEDISK, TMS37157, USB, A, KERMIT, MODBUS, X_25, XMODEM
};

enum class crc32_type
{
    DEFAULT, // DEFAULT = ZLIB
    ZLIB = DEFAULT,
    BZIP2, C, D, MPEG2, POSIX, Q, JAMCRC, XFER
};

enum class crc64_type
{
    DEFAULT, WE, XZ
};

template <typename EnumT>
struct crc_qrk : cvt_qrk<void>
{
    EnumT type;
    explicit crc_qrk(EnumT t) : type {t} {}
};

template <size_t BitsV>
struct crc_custom_qrk : cvt_qrk<void>
{
    using crc_t = typename select_int<BitsV>::unsigned_type;
    crc_t polynomial, init, out;
    bool reflectin, reflectout;

    crc_custom_qrk(crc_t polynomial_val, crc_t init_val, crc_t out_val, bool reflectin_val, bool reflectout_val)
        : polynomial {polynomial_val}
        , init {init_val}
        , out {out_val}
        , reflectin {reflectin_val}
        , reflectout {reflectout_val}
    {}
};

inline auto crc(crc8_type t) { return crc_qrk<crc8_type>{ t }; }
inline auto crc(crc16_type t) { return crc_qrk<crc16_type>{ t }; }
inline auto crc(crc32_type t) { return crc_qrk<crc32_type>{ t }; }
inline auto crc(crc64_type t) { return crc_qrk<crc64_type>{ t }; }

}

#include "../detail/crc/bytes_to_crc8_pusher.hpp"
#include "../detail/crc/bytes_to_crc16_pusher.hpp"
#include "../detail/crc/bytes_to_crc32_pusher.hpp"
#include "../detail/crc/bytes_to_crc64_pusher.hpp"
