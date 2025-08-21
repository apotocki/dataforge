/*=============================================================================
    Copyright (c) 2022 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include <array>

#include "test_common.hpp"
#include "dataforge/checksum/bsd.hpp"
#include "dataforge/checksum/adler32.hpp"
#include "dataforge/checksum/crc.hpp"

namespace dataforge {

void bsd_checksum_test()
{
    std::string example0 = "The quick brown fox jumps over the lazy dog.";
    DATAFORGE_PUSH_TEST(int8 | bsd_checksum, example0, (std::array<uint16_t, 1>{ 0x62E5 }));
    DATAFORGE_PUSH_TEST(int8 | bsd_checksum / int16 | le, example0, (std::array<unsigned char, 2>{ 0xe5, 0x62 }));
}

void adler32_test()
{
    std::string example0 = "The quick brown fox jumps over the lazy dog.";
    DATAFORGE_PUSH_TEST(int8 | adler32, example0, (std::array<uint32_t, 1>{ 0x6be41008 }));
    DATAFORGE_PUSH_TEST(int8 | adler32/ int32 | le, example0, (std::array<unsigned char, 4>{ 0x8, 0x10, 0xe4, 0x6b }));
}

void crc_test()
{
    std::string example0 = "123456789";
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::DEFAULT), example0, (std::array<unsigned char, 1>{ 0xf4 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::CDMA2000), example0, (std::array<unsigned char, 1>{ 0xda }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::DARC), example0, (std::array<unsigned char, 1>{ 0x15 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::DVB_S2), example0, (std::array<unsigned char, 1>{ 0xbc }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::EBU), example0, (std::array<unsigned char, 1>{ 0x97 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::I_CODE), example0, (std::array<unsigned char, 1>{ 0x7e }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::ITU), example0, (std::array<unsigned char, 1>{ 0xa1 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::MAXIM), example0, (std::array<unsigned char, 1>{ 0xa1 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::ROHC), example0, (std::array<unsigned char, 1>{ 0xd0 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc8_type::WCDMA), example0, (std::array<unsigned char, 1>{ 0x25 }));

    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::CCITT_FALSE), example0, (std::array<uint16_t, 1>{ 0x29b1 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::ARC), example0, (std::array<uint16_t, 1>{ 0xbb3d }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::AUG_CCITT), example0, (std::array<uint16_t, 1>{ 0xe5cc }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::BUYPASS), example0, (std::array<uint16_t, 1>{ 0xfee8 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::CDMA2000), example0, (std::array<uint16_t, 1>{ 0x4c06 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::DDS_110), example0, (std::array<uint16_t, 1>{ 0x9ecf }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::DECT_R), example0, (std::array<uint16_t, 1>{ 0x007e }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::DECT_X), example0, (std::array<uint16_t, 1>{ 0x007f }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::DNP), example0, (std::array<uint16_t, 1>{ 0xea82 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::EN_13757), example0, (std::array<uint16_t, 1>{ 0xc2b7 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::GENIBUS), example0, (std::array<uint16_t, 1>{ 0xd64e }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::MAXIM), example0, (std::array<uint16_t, 1>{ 0x44c2 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::MCRF4XX), example0, (std::array<uint16_t, 1>{ 0x6f91 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::RIELLO), example0, (std::array<uint16_t, 1>{ 0x63d0 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::T10_DIF), example0, (std::array<uint16_t, 1>{ 0xd0db }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::TELEDISK), example0, (std::array<uint16_t, 1>{ 0x0fb3 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::TMS37157), example0, (std::array<uint16_t, 1>{ 0x26b1 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::USB), example0, (std::array<uint16_t, 1>{ 0xb4c8 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::A), example0, (std::array<uint16_t, 1>{ 0xbf05 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::KERMIT), example0, (std::array<uint16_t, 1>{ 0x2189 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::MODBUS), example0, (std::array<uint16_t, 1>{ 0x4b37 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::X_25), example0, (std::array<uint16_t, 1>{ 0x906e }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc16_type::XMODEM), example0, (std::array<uint16_t, 1>{ 0x31c3 }));

    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::DEFAULT), example0, (std::array<uint32_t, 1>{ 0xCBF43926 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::BZIP2), example0, (std::array<uint32_t, 1>{ 0xFC891918 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::C), example0, (std::array<uint32_t, 1>{ 0xE3069283 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::D), example0, (std::array<uint32_t, 1>{ 0x87315576 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::MPEG2), example0, (std::array<uint32_t, 1>{ 0x0376E6E7 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::POSIX), example0, (std::array<uint32_t, 1>{ 0x765E7680 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::Q), example0, (std::array<uint32_t, 1>{ 0x3010BF7F }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::JAMCRC), example0, (std::array<uint32_t, 1>{ 0x340BC6D9 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc32_type::XFER), example0, (std::array<uint32_t, 1>{ 0xBD0BE338 }));

    DATAFORGE_PUSH_TEST(int8 | crc(crc64_type::DEFAULT), example0, (std::array<uint64_t, 1>{ 0x6C40DF5F0B497347 }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc64_type::WE), example0, (std::array<uint64_t, 1>{ 0x62EC59E3F1A4F00A }));
    DATAFORGE_PUSH_TEST(int8 | crc(crc64_type::XZ), example0, (std::array<uint64_t, 1>{ 0x995DC9BBDF1939FA }));
}

}
