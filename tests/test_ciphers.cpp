/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "test_common.hpp"

#include "dataforge/ciphers/kuznyechik.hpp"
#include "dataforge/ciphers/magma.hpp"
#include "dataforge/ciphers/belt.hpp"
#include "dataforge/ciphers/blowfish.hpp"
#include "dataforge/ciphers/rc2.hpp"
#include "dataforge/ciphers/rc4.hpp"
#include "dataforge/ciphers/rc5.hpp"
#include "dataforge/ciphers/rc6.hpp"
#include "dataforge/ciphers/des.hpp"
#include "dataforge/ciphers/aes.hpp"
#include "dataforge/base_xx/base16.hpp"
#include "dataforge/basic/filter.hpp"
#include "dataforge/basic/group.hpp"

#include <array>

namespace dataforge {

using namespace std::literals::string_view_literals;

void rc2_test()
{
#if 1
    // zero input to encrypt -> zero output for zero & none paddings
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CFB, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CFB, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::OFB, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::OFB, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CTR, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::CTR, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::zero) / int8 | base16l, ""sv, ""sv);
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::none) / int8 | base16l, ""sv, ""sv);

    // zero input to deccrypt -> zero output for zero & none paddings
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::none) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CFB, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CFB, ""_bs, padding_type::none) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::OFB, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::OFB, ""_bs, padding_type::none) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CTR, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::CTR, ""_bs, padding_type::none) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::zero) | int8, ""sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::none) | int8, ""sv, ""sv);

    // zero input to decrypt -> exception for pkcs paddings
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CFB, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::OFB, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CTR, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::pkcs) | int8, ""sv, "insufficient input data");

    // insufficient input to decrypt -> exception
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::none) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::zero) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::CBC, ""_bs, padding_type::pkcs) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::none) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::zero) | int8, "\0"sv, "insufficient input data");
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::PCBC, ""_bs, padding_type::pkcs) | int8, "\0"sv, "insufficient input data");

    // empty string encode for pkcs
    DATAFORGE_TEST(int8 | rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16l, ""sv, "dae7dbb8804e1f42"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, "dae7dbb8804e1f42"sv, ""sv);
    DATAFORGE_TEST(base16l | int8 / rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "dae7dbb8804e1f42"sv, "\x8\x8\x8\x8\x8\x8\x8\x8"_bs);

    // wrong pkcs padding test
    CONV_EXCEPTION_TEST(rc2("\0"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, "\0\0\0\0\0\0\0\0"sv, "wrong padding");

    // half block to encode, padding none
    CONV_EXCEPTION_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::none), "1234"sv, "Not enough input data");
    CONV_EXCEPTION_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CBC, ""_bs, padding_type::none), "1234"sv, "Not enough input data");
    CONV_EXCEPTION_TEST(int8 | rc2("12345678"sv, cipher_mode_type::PCBC, ""_bs, padding_type::none), "1234"sv, "Not enough input data");

    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::OFB, "12345678"_bs, padding_type::none) / int8 | base16l, "1234"sv, "5076f3a7"sv);
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CFB, "12345678"_bs, padding_type::none) / int8 | base16l, "1234"sv, "5076f3a7"sv);
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CTR, "12345678"_bs, padding_type::none) / int8 | base16l, "1234"sv, "5076f3a7"sv);

    // half block to decode, padding none
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::OFB, "12345678"_bs, padding_type::none) | int8, "5076f3a7"sv, "1234"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::CFB, "12345678"_bs, padding_type::none) | int8, "5076f3a7"sv, "1234"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::CTR, "12345678"_bs, padding_type::none) | int8, "5076f3a7"sv, "1234"sv);

    // one and half block, none padding
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "9434378879e1fade3f9b15a3"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::ECB, "12345678"_bs, padding_type::none) | int8, "9434378879e1fade3f9b15a3"sv, "123412345678"sv);
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CBC, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "d0270ed44d40d1d39ba78834"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::CBC, "12345678"_bs, padding_type::none) | int8, "d0270ed44d40d1d39ba78834"sv, "123412345678"sv);
    //DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::PCBC, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "8e585f1ee15e67489ba78834"sv);
    //DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::PCBC, "12345678"_bs, padding_type::none) | int8, "8e585f1ee15e67489ba78834"sv, "123412345678"sv);

    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::OFB, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "5076f3a7e1b2045a23eed042"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::OFB, "12345678"_bs, padding_type::none) | int8, "5076f3a7e1b2045a23eed042"sv, "123412345678"sv);
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CFB, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "5076f3a7e1b2045a662938a3"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::CFB, "12345678"_bs, padding_type::none) | int8, "5076f3a7e1b2045a662938a3"sv, "123412345678"sv);
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::CTR, "12345678"_bs, padding_type::none) / int8 | base16l, "123412345678"sv, "5076f3a7e1b2045ab458345d"sv);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::CTR, "12345678"_bs, padding_type::none) | int8, "5076f3a7e1b2045ab458345d"sv, "123412345678"sv);

    //DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16l, "1234"sv, "dae7dbb8804e1f42"sv);

    // standard
    auto example0 = "The quick brown fox jumps over the lazy dog....."sv;
    auto result0 = "1504be48efdec0bef425685c1f4f628a089e6920f6a2d779ee38c7fc2a901698622ddb42fe9a729575359e3dd3475c3f5959a55150ef79bd"sv;
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16l, example0, result0);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, result0, example0);

    auto example1 = "The quick brown fox jumps over the lazy dog."sv;
    auto result1 = "1504be48efdec0bef425685c1f4f628a089e6920f6a2d779ee38c7fc2a901698622ddb42fe9a72950d0e3ce4aa58106e"sv;
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16l, example1, result1);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, result1, example1);

    auto result1_0 = "1504be48efdec0bef425685c1f4f628a089e6920f6a2d779ee38c7fc2a9016986a5960cace43dde4622ddb42"sv;
    DATAFORGE_TEST(int8 | rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16l, example1, result1_0);
    DATAFORGE_TEST(base16l | int8 / rc2("12345678"sv, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8, result1_0, example1);
#endif
}

TEST(DataforgeTest, rc2)
{
    rc2_test();
}

void blowfish_test()
{
    auto example0 = "The quick brown fox jumps over the lazy dog."sv;
    auto padded0_example0 = "The quick brown fox jumps over the lazy dog.\x0\x0\x0\x0"sv;

    DATAFORGE_TEST(int8 | blowfish_qrk(false, ""_bs, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16l, example0, "54859a6a7ed1951f06cbe2b040784c707b4d8aae12594e5c100ab3755b5958c7606afd3395f65249f783c66c1b2d30cf"sv);
    DATAFORGE_TEST(base16l | int8 / blowfish_qrk(false, ""_bs, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, "54859a6a7ed1951f06cbe2b040784c707b4d8aae12594e5c100ab3755b5958c7606afd3395f65249f783c66c1b2d30cf"sv, example0);

    std::array<unsigned char, 8> key64 = {
        0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
    };

    DATAFORGE_TEST(int8 | blowfish(true, key64) / int8 | base16u, example0, "22BBA7DED59E0D7588C2C9FE1BFB5C477DFD22948491303545F4A66D58F4F84CA9E68A5AB44CB44F9821668D74D9B362"sv);
    DATAFORGE_TEST(base16u | int8 / blowfish(true, key64) | int8, "22BBA7DED59E0D7588C2C9FE1BFB5C477DFD22948491303545F4A66D58F4F84CA9E68A5AB44CB44F9821668D74D9B362"sv, padded0_example0);
    
    DATAFORGE_TEST(int8 | blowfish(false, key64) / int8 | base16u, example0, "EB69EC828DC72E4CF298A158B067719D5F7F0729BAEC1DDD1D0317860506582C8FB720D36E2890481646C1AF9A36AAC1"sv);
    DATAFORGE_TEST(base16u | int8 / blowfish(false, key64) | int8, "EB69EC828DC72E4CF298A158B067719D5F7F0729BAEC1DDD1D0317860506582C8FB720D36E2890481646C1AF9A36AAC1"sv, padded0_example0);
    
    std::array<unsigned char, 8> iv = {
        0xe3, 0x8b, 0xfb, 0x0c, 0xd0, 0xb9, 0x30, 0xcb
    };

    DATAFORGE_TEST(int8 | blowfish(true, key64, cipher_mode_type::CBC, iv) / int8 | base16u, example0, "9137A028FFFE9BF102DADDF75895A2A038DD6F7F8CF0FB4E0AD172ADE383DEAB46748AD94681CF3D67A823860A455993"sv);
    DATAFORGE_TEST(base16u | int8 / blowfish(true, key64, cipher_mode_type::CBC, iv) | int8, "9137A028FFFE9BF102DADDF75895A2A038DD6F7F8CF0FB4E0AD172ADE383DEAB46748AD94681CF3D67A823860A455993"sv, padded0_example0);
    
    DATAFORGE_TEST(int8 | blowfish(false, key64, cipher_mode_type::CBC, iv) / int8 | base16u, example0, "D8DD92D65BEF17EFA9890B00CD49027A5D396924483DE77D887C4416B4B4F01C5F78D7C954458B7A7C2F8360814D1372"sv);
    DATAFORGE_TEST(base16u | int8 / blowfish(false, key64, cipher_mode_type::CBC, iv) | int8, "D8DD92D65BEF17EFA9890B00CD49027A5D396924483DE77D887C4416B4B4F01C5F78D7C954458B7A7C2F8360814D1372"sv, padded0_example0);
}

TEST(DataforgeTest, blowfish)
{
    blowfish_test();
}

void rc4_test()
{
    auto example0 = "The quick brown fox jumps over the lazy dog."sv;
    std::array<unsigned char, 8> key64 = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };

    DATAFORGE_TEST(int8 | rc4_qrk(key64, 8u, 0u) / int8 | base16u, example0, "C3C3EF3B81DAD00259D2940037AD7B88E40C97FB2FB1CCF6F7CFE890D4EC7B7DAE95CAC0707E5C9DD28D41BA"sv);
    DATAFORGE_TEST(base16u | int8 / rc4_qrk(key64, 8u, 0u) | int8, "C3C3EF3B81DAD00259D2940037AD7B88E40C97FB2FB1CCF6F7CFE890D4EC7B7DAE95CAC0707E5C9DD28D41BA"sv, example0);

    DATAFORGE_TEST(int8 | rc4_qrk(key64, 8u, 1u) / int8 | base16u, example0, "9F3DA02D89A2B5537259194B439A6474A7DE8FCD48973D33B1D72C853DBD8D700B1D55BA69F8EB7E3F1E74E4"sv);
    DATAFORGE_TEST(base16u | int8 / rc4_qrk(key64, 8u, 1u) | int8, "9F3DA02D89A2B5537259194B439A6474A7DE8FCD48973D33B1D72C853DBD8D700B1D55BA69F8EB7E3F1E74E4"sv, example0);
}

TEST(DataforgeTest, rc4)
{
    rc4_test();
}

void rc5_test()
{
   auto example5 = "The unknown message is: The RC5-32/12/5-test contest's plaintext"sv;
    std::array<unsigned char, 5> key40 = { 0x27, 0xd8, 0x6d, 0xd2, 0x43 };
    std::array<unsigned char, 8> iv5 = { 0xf6, 0x75, 0x17, 0x1a, 0x59, 0xb7, 0xea, 0xd0 };
    
    unsigned char rc5_32_12_5_cipher[] = {
        0xb4, 0x0a, 0x53, 0x88, 0xb1, 0x38, 0x82, 0xad, 0x93, 0x0d, 0xa8, 0xb6, 0x1e, 0x51, 0xa3, 0xb1,
        0xff, 0xae, 0xe1, 0xd8, 0x14, 0xa5, 0xfa, 0x15, 0x40, 0x0b, 0x66, 0x3d, 0x58, 0x13, 0x8a, 0x33,
        0x19, 0x60, 0x3c, 0x85, 0x10, 0xc7, 0xb0, 0xd4, 0x87, 0x91, 0x8e, 0x32, 0xf8, 0x34, 0x77, 0x5b,
        0x20, 0x65, 0xe9, 0x74, 0x33, 0xb6, 0xea, 0xda, 0x54, 0x8a, 0xd9, 0xd7, 0xf7, 0x08, 0xf6, 0x45,
        0xc2, 0x33, 0xce, 0x8a, 0x83, 0xb2, 0x76, 0xe0
    };
    std::span r5{ rc5_32_12_5_cipher, sizeof(rc5_32_12_5_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key40, cipher_mode_type::CBC, iv5, padding_type::pkcs) / int8, example5, r5);
    DATAFORGE_TEST(rc5_qrk<32>(12, key40, cipher_mode_type::CBC, iv5, padding_type::pkcs) | int8, r5, example5);

    const std::string example6 = "The unknown message is: The RC5-32/12/6-test contest's plaintext";
    std::array<unsigned char, 6> key48 = { 0x28, 0x26, 0x04, 0x78, 0x06, 0x51 };
    std::array<unsigned char, 8> iv6 = { 0x23, 0x8c, 0x3c, 0xc9, 0xb0, 0xfd, 0x9f, 0x9e };
    
    unsigned char rc5_32_12_6_cipher[] = {
        0xbf, 0x88, 0x86, 0xd2, 0xa9, 0x50, 0x84, 0x1c, 0xb8, 0x83, 0xf3, 0xd4, 0xcf, 0xd9, 0x55, 0x1f,
        0x98, 0xab, 0xd2, 0xe6, 0xbf, 0x40, 0xd4, 0x92, 0x91, 0x85, 0x2f, 0x28, 0xd0, 0x89, 0xfb, 0xb4,
        0xf0, 0xf9, 0xc3, 0x27, 0xa6, 0xc5, 0xaa, 0xa2, 0xdf, 0x4f, 0x36, 0x3f, 0x17, 0xfb, 0xcf, 0x1e,
        0xfc, 0x6e, 0x65, 0x85, 0x30, 0xf8, 0x5e, 0x02, 0x00, 0x58, 0x31, 0xb1, 0xe0, 0x67, 0x2e, 0x49,
        0x50, 0xcb, 0xea, 0x3c, 0x95, 0x1d, 0x46, 0x31
    };
    std::span r6{ rc5_32_12_6_cipher, sizeof(rc5_32_12_6_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key48, cipher_mode_type::CBC, iv6, padding_type::pkcs) / int8, example6, r6);
    DATAFORGE_TEST(rc5_qrk<32>(12, key48, cipher_mode_type::CBC, iv6, padding_type::pkcs) | int8, r6, example6);

    const std::string example7 = "The unknown message is: The RC5-32/12/7-test contest's plaintext";
    unsigned char key56[] = { 0x14, 0x11, 0xb3, 0x5a, 0x9a, 0x98, 0xda };
    unsigned char iv7[] = { 0xfb, 0xb2, 0xe9, 0xe4, 0x12, 0xd0, 0xec, 0xbc };
    
    unsigned char rc5_32_12_7_cipher[] = {
        0x39, 0xde, 0x1b, 0x56, 0x3f, 0x1d, 0x5c, 0x25, 0xe7, 0xe8, 0xce, 0xbb, 0xe7, 0x2a, 0xec, 0x22,
        0x6c, 0xb1, 0x90, 0x34, 0xc1, 0xef, 0x0f, 0xd3, 0x84, 0x77, 0xd9, 0x6d, 0x52, 0xd9, 0x34, 0x50,
        0xf8, 0x08, 0xe8, 0xb2, 0xf5, 0x6a, 0xdc, 0xcb, 0xe5, 0xdf, 0x16, 0x01, 0xe2, 0xc0, 0x0a, 0x25,
        0x23, 0xdf, 0x22, 0x67, 0xd6, 0x60, 0x93, 0x4a, 0x39, 0x68, 0xc1, 0x42, 0x6c, 0x69, 0x73, 0x2f,
        0x3b, 0x4a, 0x4e, 0x25, 0x35, 0xc2, 0xce, 0xc9
    };
    std::span r7{ rc5_32_12_7_cipher, sizeof(rc5_32_12_7_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key56, cipher_mode_type::CBC, iv7, padding_type::pkcs) / int8, example7, r7);
    DATAFORGE_TEST(rc5_qrk<32>(12, key56, cipher_mode_type::CBC, iv7, padding_type::pkcs) | int8, r7, example7);

    const std::string example8 = "The unknown message is: The RC5-32/12/8-test contest's plaintext";
    unsigned char key64[] = { 0x82, 0xe5, 0x1b, 0x9f, 0x9c, 0xc7, 0x18, 0xf9 };
    unsigned char iv8[] = { 0xd9, 0xa5, 0x39, 0xf8, 0xc1, 0x78, 0x1f, 0xc4 };
    
    unsigned char rc5_32_12_8_cipher[] = {
        0x41, 0xe0, 0x4b, 0xb7, 0x29, 0xef, 0x6d, 0x49, 0xb9, 0x85, 0x72, 0x64, 0x52, 0xfe, 0x90, 0x11,
        0xb0, 0xbf, 0xa4, 0xb2, 0x5b, 0xae, 0x2a, 0x48, 0x0c, 0xa4, 0x81, 0x7a, 0xe8, 0x09, 0x0f, 0xba,
        0x5a, 0x13, 0x41, 0xe0, 0xbc, 0xc3, 0x8a, 0xe8, 0x16, 0x87, 0x4e, 0xad, 0xc6, 0xdf, 0x53, 0x87,
        0xd2, 0x37, 0x4a, 0x82, 0x14, 0x65, 0xcc, 0xb8, 0x2c, 0x92, 0x92, 0xd1, 0x28, 0xf3, 0xba, 0xef,
        0x10, 0x19, 0x2c, 0x0f, 0x39, 0xc7, 0xa7, 0x6d
    };
    std::span r8{ rc5_32_12_8_cipher, sizeof(rc5_32_12_8_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key64, cipher_mode_type::CBC, iv8, padding_type::pkcs) / int8, example8, r8);
    DATAFORGE_TEST(rc5_qrk<32>(12, key64, cipher_mode_type::CBC, iv8, padding_type::pkcs) | int8, r8, example8);

    const std::string example9 = "The unknown message is: The RC5-32/12/9-test contest's plaintext";
    unsigned char key72[] = { 0xc9, 0x0c, 0x03, 0x53, 0xc0, 0xd4, 0xe1, 0xfe, 0x85 };
    unsigned char iv9[] = { 0x07, 0xce, 0x59, 0x1f, 0x86, 0x14, 0x9a, 0x41 };
    
    unsigned char rc5_32_12_9_cipher[] = {
        0x5a, 0x28, 0x2d, 0x56, 0x2a, 0x85, 0xb7, 0x2f, 0xef, 0xae, 0x30, 0xba, 0x93, 0x54, 0xda, 0x15,
        0xac, 0xba, 0x79, 0x1a, 0xa1, 0xcb, 0x0e, 0x85, 0x9a, 0xc3, 0x31, 0xa5, 0x36, 0x19, 0x85, 0xe3,
        0x77, 0xdf, 0xdd, 0x07, 0x34, 0x3b, 0xfa, 0xed, 0x84, 0x84, 0x41, 0x90, 0x9d, 0x7b, 0x6f, 0x17,
        0x2d, 0x51, 0xea, 0xb2, 0x1c, 0x5e, 0x45, 0x1c, 0xe9, 0x6a, 0x34, 0xde, 0xa5, 0xf5, 0xb9, 0x56,
        0x22, 0x16, 0xe2, 0x60, 0x15, 0xe1, 0x7e, 0xb2
    };
    std::span r9{ rc5_32_12_9_cipher, sizeof(rc5_32_12_9_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key72, cipher_mode_type::CBC, iv9, padding_type::pkcs) / int8, example9, r9);
    DATAFORGE_TEST(rc5_qrk<32>(12, key72, cipher_mode_type::CBC, iv9, padding_type::pkcs) | int8, r9, example9);

    const std::string example10 = "The unknown message is: The RC5-32/12/10-test contest's plaintext";
    unsigned char key80[] = { 0xa4, 0xac, 0x9b, 0xaf, 0xac, 0xb1, 0x27, 0x5f, 0xb9, 0x6f };
    unsigned char iv10[] = { 0x31, 0xef, 0x08, 0x46, 0xe1, 0x43, 0xdd, 0xa7 };
    
    unsigned char rc5_32_12_10_cipher[] = {
        0x4d, 0xf8, 0x76, 0xd7, 0xe6, 0x49, 0x1a, 0xc7, 0xd6, 0x05, 0xe9, 0x37, 0xb5, 0xc2, 0x13, 0xb9,
        0x29, 0x6f, 0x6c, 0xc1, 0x20, 0x18, 0x57, 0x97, 0x6a, 0x37, 0xf1, 0x96, 0x63, 0xb0, 0x7a, 0x7d,
        0x4f, 0x69, 0x96, 0x97, 0x9a, 0xa9, 0x6d, 0x62, 0x2e, 0xd2, 0xca, 0x83, 0x07, 0x74, 0x43, 0xfc,
        0xbe, 0x23, 0x4e, 0x6d, 0xcf, 0x2f, 0x93, 0x06, 0xa3, 0x26, 0x67, 0xaa, 0x61, 0x0c, 0xd8, 0x1d,
        0x60, 0xca, 0xe8, 0xf5, 0xb1, 0xd9, 0x74, 0x49
    };
    std::span r10{ rc5_32_12_10_cipher, sizeof(rc5_32_12_10_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key80, cipher_mode_type::CBC, iv10, padding_type::pkcs) / int8, example10, r10);
    DATAFORGE_TEST(rc5_qrk<32>(12, key80, cipher_mode_type::CBC, iv10, padding_type::pkcs) | int8, r10, example10);

    const std::string example11 = "The unknown message is: The RC5-32/12/11-test contest's plaintext";
    unsigned char key88[] = { 0x65, 0x82, 0x8d, 0xe0, 0x89, 0x1d, 0x6a, 0x88, 0xbe, 0xc9, 0xcd };
    unsigned char iv11[] = { 0x09, 0xfb, 0x2f, 0x69, 0x87, 0xb2, 0xf2, 0xfd };
    
    unsigned char rc5_32_12_11_cipher[] = {
        0x76, 0x85, 0x6d, 0xdd, 0x78, 0x66, 0x9a, 0x9a, 0xa2, 0x57, 0xb5, 0x3c, 0xf9, 0x65, 0x7a, 0x44,
        0x2a, 0x2c, 0xf4, 0x8b, 0x8e, 0xd8, 0x7a, 0x68, 0x36, 0xc2, 0xcb, 0x6f, 0xed, 0x11, 0x65, 0x01,
        0xd9, 0x3e, 0x3a, 0xcc, 0xdb, 0x29, 0xaa, 0xd2, 0x82, 0x5a, 0xa1, 0x9a, 0x9b, 0xb4, 0xdf, 0x79,
        0xe5, 0xa1, 0x35, 0xd7, 0xd5, 0x62, 0x3e, 0x59, 0x4e, 0xb8, 0xb1, 0x1e, 0x56, 0xe0, 0xfd, 0xa0,
        0x1a, 0x0a, 0xd6, 0xc5, 0x1f, 0x6c, 0xdc, 0x62
    };
    std::span r11{ rc5_32_12_11_cipher, sizeof(rc5_32_12_11_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key88, cipher_mode_type::CBC, iv11, padding_type::pkcs) / int8, example11, r11);
    DATAFORGE_TEST(rc5_qrk<32>(12, key88, cipher_mode_type::CBC, iv11, padding_type::pkcs) | int8, r11, example11);

    const std::string example12 = "The unknown message is: The RC5-32/12/12-test contest's plaintext";
    unsigned char key96[] = { 0x67, 0x40, 0x23, 0x1b, 0x6d, 0x1a, 0x7e, 0x0c, 0x92, 0x96, 0x05, 0xc2 };
    unsigned char iv12[] = { 0xf0, 0xed, 0xe8, 0x46, 0x43, 0x48, 0xbd, 0x62 };
    
    unsigned char rc5_32_12_12_cipher[] = {
        0xae, 0x0e, 0xe8, 0x11, 0xdf, 0xfd, 0xc8, 0xd2, 0x06, 0xe9, 0x2e, 0xb4, 0xda, 0x75, 0x04, 0xfc,
        0x33, 0x17, 0x47, 0x91, 0x98, 0x50, 0x0f, 0xf5, 0x2e, 0x70, 0x5b, 0xf7, 0x38, 0x8b, 0x24, 0x53,
        0x34, 0x86, 0x40, 0x95, 0x44, 0x46, 0xd5, 0x0a, 0x67, 0x1a, 0x2b, 0xc0, 0x2f, 0x84, 0xdf, 0x92,
        0x64, 0x74, 0x8e, 0xda, 0x1a, 0x4b, 0xa6, 0xe3, 0x1a, 0xc8, 0x67, 0xd3, 0x52, 0x58, 0xa4, 0xb7,
        0x54, 0x7f, 0xc4, 0x83, 0xbf, 0xad, 0x1f, 0xfe
    };
    std::span r12{ rc5_32_12_12_cipher, sizeof(rc5_32_12_12_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key96, cipher_mode_type::CBC, iv12, padding_type::pkcs) / int8, example12, r12);
    DATAFORGE_TEST(rc5_qrk<32>(12, key96, cipher_mode_type::CBC, iv12, padding_type::pkcs) | int8, r12, example12);

    const std::string example13 = "The unknown message is: The RC5-32/12/13-test contest's plaintext";
    unsigned char key104[] = { 0x32, 0x32, 0xd5, 0x0c, 0xc6, 0x46, 0x8b, 0xf4, 0x3e, 0xb9, 0xcb, 0xba, 0x82 };
    unsigned char iv13[] = { 0xdb, 0xce, 0x3b, 0x13, 0xf0, 0x81, 0xaf, 0x9e };
    
    unsigned char rc5_32_12_13_cipher[] = {
        0x50, 0x63, 0x53, 0x2e, 0xdc, 0xb0, 0x5a, 0xc4, 0xf6, 0xe0, 0x13, 0x7e, 0xbb, 0x0b, 0xf1, 0xe4,
        0x9b, 0x0c, 0x2b, 0xed, 0xc3, 0xc1, 0xf8, 0x34, 0x6a, 0x09, 0x63, 0xad, 0xca, 0x8f, 0xa3, 0x9b,
        0x30, 0xec, 0xfc, 0xba, 0x21, 0x63, 0x5d, 0xd8, 0xdf, 0x37, 0xa0, 0xbc, 0x7c, 0x24, 0xd6, 0xb0,
        0xea, 0xe1, 0x05, 0x04, 0x56, 0xce, 0x35, 0x6b, 0x4f, 0xb7, 0xa7, 0x23, 0x08, 0xd1, 0xa3, 0x5b,
        0xf3, 0x3d, 0x7f, 0x9a, 0x57, 0x96, 0xb4, 0x50
    };
    std::span r13{ rc5_32_12_13_cipher, sizeof(rc5_32_12_13_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key104, cipher_mode_type::CBC, iv13, padding_type::pkcs) / int8, example13, r13);
    DATAFORGE_TEST(rc5_qrk<32>(12, key104, cipher_mode_type::CBC, iv13, padding_type::pkcs) | int8, r13, example13);

    const std::string example14 = "The unknown message is: The RC5-32/12/14-test contest's plaintext";
    unsigned char key112[] = { 0xb5, 0x6e, 0x61, 0x11, 0x16, 0x02, 0xc4, 0x46, 0xfd, 0x45, 0x2b, 0xa8, 0x15, 0x9c };
    unsigned char iv14[] = { 0x64, 0xc0, 0xb2, 0xff, 0x53, 0x23, 0x51, 0xb3 };
    
    unsigned char rc5_32_12_14_cipher[] = {
        0xdf, 0x0b, 0x2e, 0x7a, 0x13, 0x29, 0x63, 0x2b, 0xd6, 0x81, 0xef, 0xf8, 0x64, 0x76, 0x58, 0x64,
        0xaa, 0x48, 0xac, 0x91, 0xf8, 0xbd, 0x9c, 0xf8, 0xea, 0xb8, 0x2c, 0xe6, 0xc3, 0xfb, 0xac, 0x54,
        0x88, 0x2e, 0xd1, 0xa1, 0x22, 0xd8, 0x1b, 0x0e, 0x5e, 0xd3, 0x5b, 0x0f, 0x07, 0x3d, 0x17, 0xfc,
        0xd2, 0xe5, 0xf9, 0x37, 0x35, 0xaf, 0xfc, 0xae, 0x00, 0x09, 0x19, 0x29, 0x7f, 0x3b, 0xf1, 0x5f,
        0x5d, 0xf3, 0xa2, 0x8d, 0x9e, 0xf9, 0x9d, 0x4f
    };
    std::span r14{ rc5_32_12_14_cipher, sizeof(rc5_32_12_14_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key112, cipher_mode_type::CBC, iv14, padding_type::pkcs) / int8, example14, r14);
    DATAFORGE_TEST(rc5_qrk<32>(12, key112, cipher_mode_type::CBC, iv14, padding_type::pkcs) | int8, r14, example14);

    const std::string example15 = "The unknown message is: The RC5-32/12/15-test contest's plaintext";
    unsigned char key120[] = { 0x4f, 0x74, 0xb5, 0xf4, 0x4d, 0x24, 0x4e, 0xce, 0xb4, 0x24, 0xa8, 0x31, 0x5e, 0xc7, 0x0f };
    unsigned char iv15[] = { 0xfa, 0xc9, 0x86, 0xa0, 0x90, 0x48, 0x30, 0xa2 };
    
    unsigned char rc5_32_12_15_cipher[] = {
        0x8e, 0xd7, 0x83, 0xd4, 0x97, 0x00, 0x67, 0x0b, 0x98, 0xac, 0x58, 0x18, 0xc1, 0xdb, 0x75, 0xbf,
        0xe0, 0x3c, 0x0a, 0x6e, 0x1c, 0x5b, 0xcc, 0xb9, 0xa5, 0xa9, 0x07, 0xbc, 0xdf, 0xc6, 0xf9, 0x6a,
        0x80, 0x1f, 0x3c, 0x98, 0x78, 0x21, 0x6f, 0xe3, 0x0d, 0x2e, 0x9b, 0x8d, 0xb3, 0x9c, 0x85, 0x2a,
        0x97, 0x03, 0x61, 0x30, 0x2b, 0xce, 0xbb, 0x55, 0xce, 0x19, 0x62, 0x7f, 0xd9, 0x69, 0xb5, 0x55,
        0x5e, 0xb4, 0x63, 0xde, 0xd7, 0xf8, 0x1e, 0xa3
    };
    std::span r15{ rc5_32_12_15_cipher, sizeof(rc5_32_12_15_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key120, cipher_mode_type::CBC, iv15, padding_type::pkcs) / int8, example15, r15);
    DATAFORGE_TEST(rc5_qrk<32>(12, key120, cipher_mode_type::CBC, iv15, padding_type::pkcs) | int8, r15, example15);

    const std::string example16 = "The unknown message is: The RC5-32/12/16-test contest's plaintext";
    unsigned char key128[] = { 0xb1, 0x6c, 0x2a, 0xb9, 0x74, 0xfb, 0x5e, 0x48, 0x7f, 0x8a, 0x65, 0xd7, 0xf9, 0xe4, 0x0c, 0x62 };
    unsigned char iv16[] = { 0x0b, 0x3c, 0x9d, 0x50, 0xfd, 0x06, 0xf9, 0xf1 };
    
    unsigned char rc5_32_12_16_cipher[] = {
        0xde, 0x4e, 0xae, 0xe9, 0x6c, 0xf5, 0x85, 0xc2, 0x36, 0xff, 0x6f, 0xb6, 0x9c, 0x50, 0xda, 0xd5,
        0x6a, 0x57, 0xfe, 0xb6, 0xd2, 0x41, 0x28, 0x60, 0x73, 0x04, 0x89, 0xca, 0x36, 0xc8, 0x4f, 0x7c,
        0xaa, 0xb0, 0x35, 0x3c, 0x63, 0xd0, 0xcb, 0xe0, 0xc4, 0xe3, 0x0d, 0x79, 0x77, 0xe9, 0xc6, 0x48,
        0x7f, 0x13, 0xa0, 0xda, 0x11, 0xee, 0x38, 0xb9, 0x97, 0x2e, 0xa5, 0x3d, 0x3b, 0x48, 0x4f, 0x3e,
        0x9a, 0x93, 0x37, 0x18, 0x87, 0x7c, 0x37, 0xac
    };
    std::span r16{ rc5_32_12_16_cipher, sizeof(rc5_32_12_16_cipher) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key128, cipher_mode_type::CBC, iv16, padding_type::pkcs) / int8, example16, r16);
    DATAFORGE_TEST(rc5_qrk<32>(12, key128, cipher_mode_type::CBC, iv16, padding_type::pkcs) | int8, r16, example16);

    const std::string example8b = "The unknown message is: Some things are better left unread.";
    unsigned char key64b[] = { 0x63, 0xDE, 0x7D, 0xC1, 0x54, 0xF4, 0xD0, 0x39 };
    unsigned char iv8b[] = { 0x79, 0xce, 0xd5, 0xd5, 0x50, 0x75, 0xea, 0xfc };
    
    unsigned char rc5_32_12_8_cipher_b[] = {
        0xbf, 0x55, 0x01, 0x55, 0xdc, 0x26, 0xf2, 0x4b, 0x26, 0xe4, 0x85, 0x4d, 0xf9, 0x0a, 0xd6, 0x79,
        0x66, 0x93, 0xab, 0x92, 0x3c, 0x72, 0xf1, 0x37, 0xc8, 0xb7, 0x0d, 0x1f, 0x60, 0x11, 0x0c, 0x92,
        0xae, 0x2e, 0xcd, 0xfd, 0x70, 0xd3, 0xfd, 0x17, 0xdf, 0xb0, 0x42, 0x12, 0xb9, 0x7d, 0xcf, 0x22,
        0x18, 0x6b, 0xa7, 0x15, 0xce, 0x2c, 0x84, 0xbf, 0xce, 0x0d, 0xd0, 0x4d, 0x00, 0x6b, 0xe1, 0x46
    };
    std::span r8b{ rc5_32_12_8_cipher_b, sizeof(rc5_32_12_8_cipher_b) };
    DATAFORGE_TEST(int8 | rc5_qrk<32>(12, key64b, cipher_mode_type::CBC, iv8b, padding_type::pkcs) / int8, example8b, r8b);
    DATAFORGE_TEST(rc5_qrk<32>(12, key64b, cipher_mode_type::CBC, iv8b, padding_type::pkcs) | int8, r8b, example8b);

    ////
    unsigned char iv_16[] = { 0x79, 0xce, 0xd5, 0xd5, 0x50, 0x75, 0xea, 0xfc, 0x79, 0xce, 0xd5, 0xd5, 0x50, 0x75, 0xea, 0xfc };
    auto result64 = "93871C5C66E18BFE4F3B86CC2D0975C1D67866F403EAB84C9DD55CC42D47C6D31D9B787E6F9FB93574612203B720CF0F771B779F6DB0EFB430F70DFCC37D1ECF"sv;
    DATAFORGE_TEST(int8 | rc5_qrk<64>(18, key64b, cipher_mode_type::CBC, iv_16, padding_type::pkcs) / int8 | base16u, example8b, result64);
    DATAFORGE_TEST(base16u | int8 / rc5_qrk<64>(18, key64b, cipher_mode_type::CBC, iv_16, padding_type::pkcs) | int8, result64, example8b);

    ////
    //const std::string example8b = "The unknown message is: Some things are better left unread.";
    //unsigned char key64b[] = { 0x63, 0xDE, 0x7D, 0xC1, 0x54, 0xF4, 0xD0, 0x39 };
    unsigned char iv_4[] = { 0x79, 0xce, 0xd5, 0xd5 };
    auto result16 = "7EDEE5506CC0938449E20B90414F5F853CEDE45F753F0985E55858F79E392DC7C5E685487419250684D5650ADE59AB8A8BD7F321E2BA4FC3A6687532"sv;
    DATAFORGE_TEST(int8 | rc5_qrk<16>(18, key64b, cipher_mode_type::CBC, iv_4, padding_type::pkcs) / int8 | base16u, example8b, result16);
    DATAFORGE_TEST(base16u | int8 / rc5_qrk<16>(18, key64b, cipher_mode_type::CBC, iv_4, padding_type::pkcs) | int8, result16, example8b);
}

TEST(DataforgeTest, rc5)
{
    rc5_test();
}

void rc6_test()
{
    std::array<unsigned char, 16> example0 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<unsigned char, 16> key0 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key0, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example0, "8fc3a53656b1f778c129df4e9848a41e"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key0, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "8fc3a53656b1f778c129df4e9848a41e"sv, example0);
    
    std::array<unsigned char, 16> example1 = { 0x02, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0x8a, 0x9b, 0xac, 0xbd, 0xce, 0xdf, 0xe0, 0xf1 };
    std::array<unsigned char, 16> key1 = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78 };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key1, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example1, "524e192f4715c6231f51f6367ea43f18"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key1, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "524e192f4715c6231f51f6367ea43f18"sv, example1);
    
    std::array<unsigned char, 16> example2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<unsigned char, 24> key2 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key2, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example2, "6cd61bcb190b30384e8a3f168690ae82"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key2, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "6cd61bcb190b30384e8a3f168690ae82"sv, example2);

    std::array<unsigned char, 16> example3 = { 0x02, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0x8a, 0x9b, 0xac, 0xbd, 0xce, 0xdf, 0xe0, 0xf1 };
    std::array<unsigned char, 24> key3 = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0 };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key3, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example3, "688329d019e505041e52e92af95291d4"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key3, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "688329d019e505041e52e92af95291d4"sv, example3);

    std::array<unsigned char, 16> example4 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    std::array<unsigned char, 32> key4 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key4, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example4, "8f5fbd0510d15fa893fa3fda6e857ec2"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key4, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "8f5fbd0510d15fa893fa3fda6e857ec2"sv, example4);

    std::array<unsigned char, 16> example5 = { 0x02, 0x13, 0x24, 0x35, 0x46, 0x57, 0x68, 0x79, 0x8a, 0x9b, 0xac, 0xbd, 0xce, 0xdf, 0xe0, 0xf1 };
    std::array<unsigned char, 32> key5 = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x12, 0x23, 0x34, 0x45, 0x56, 0x67, 0x78, 0x89, 0x9a, 0xab, 0xbc, 0xcd, 0xde, 0xef, 0xf0, 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe };
    DATAFORGE_TEST(int8 | rc6_qrk<32>(20, key5, cipher_mode_type::ECB, ""_bs, padding_type::zero) / int8 | base16l, example5, "c8241816f0d7e48920ad16a1674e5d48"sv);
    DATAFORGE_TEST(base16l | int8 / rc6_qrk<32>(20, key5, cipher_mode_type::ECB, ""_bs, padding_type::zero) | int8, "c8241816f0d7e48920ad16a1674e5d48"sv, example5);

    auto example6 = "The unknown message is: Some things are better left unread."sv;
    unsigned char key6[] = { 0x63, 0xDE, 0x7D, 0xC1, 0x54, 0xF4, 0xD0, 0x39 };
    unsigned char iv6[] = { 0x64, 0xc0, 0xb2, 0xff, 0x53, 0x23, 0x51, 0xb3 };
    auto r6 = "0140C2AB63EEF23CFA1E9DDE78807478E77C7BE781619C369D16E02EBCF3B8EB0B241A532303179483D8B2C78C285D90B071CFDEB6512665441BC1B35E72ADB4"sv;
    DATAFORGE_TEST(int8 | rc6_qrk<16>(20, key6, cipher_mode_type::CBC, iv6, padding_type::pkcs) / int8 | base16u, example6, r6);
    DATAFORGE_TEST(base16u | int8 / rc6_qrk<16>(20, key6, cipher_mode_type::CBC, iv6, padding_type::pkcs) | int8, r6, example6);

    auto r7 = "9C542D60F1F52EE61964BD93A9ACDEC5243E5F07FB1242298F5638A20BC862E7B1A56BCA2C3E7086264D7ACAD908E8584F642A1A6F87C6BF65BD63B746410791"sv;
    DATAFORGE_TEST(int8 | rc6_qrk<64>(20, key6, cipher_mode_type::CBC, iv6, padding_type::pkcs) / int8 | base16u, example6, r7);
    DATAFORGE_TEST(base16u | int8 / rc6_qrk<64>(20, key6, cipher_mode_type::CBC, iv6, padding_type::pkcs) | int8, r7, example6);
}

TEST(DataforgeTest, rc6)
{
    rc6_test();
}

void des_test()
{
    auto example0 = "The quick brown fox jumps over the lazy dog."sv;
    std::array<unsigned char, 8> key64 = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };

    auto result = "2E1CC3F24E6912B8BECEB54FB51C60934C982444E3CDFCED8C82B277FE0BDC17A59353174D896A426ABD3F5059DAEAC0"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64) | int8, result, "The quick brown fox jumps over the lazy dog.\x0\x0\x0\x0"sv);

    std::array<unsigned char, 8> iv = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

    result = "E1295492CFAA94CA573CBE3446BAA5AB926B76C972CDEE80FF6D5A201A1A106EF746F86E32098430DBA4B21F4E27E436"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::CBC, iv) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::CBC, iv) | int8, result, "The quick brown fox jumps over the lazy dog.\x0\x0\x0\x0"sv);

    result = "E1295492CFAA94CA573CBE3446BAA5AB926B76C972CDEE80FF6D5A201A1A106EF746F86E32098430CD3F7023A4C22B1F"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::CBC, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::CBC, iv, padding_type::pkcs) | int8, result, example0);

    result = "D6372DEC8C1D4093FE2B242C7EE74A6DD70BDC5F7177DA6747DE2D753260437767C0C6A3B8DB958844CE4478EBEF721B"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::CFB, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::CFB, iv, padding_type::pkcs) | int8, result, example0);

    result = "D6372DEC8C1D40932BA83D1BC1DEB4D35461BC1E6B1839B0C1BC0E7D27DFA9373B14A920E57A7D500A910F2FF8DF6C3D"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::OFB, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::OFB, iv, padding_type::pkcs) | int8, result, example0);

    result = "D6372DEC8C1D409372101BE339713CA0C83648E3433121EECC74E26F4DD551FCDCABE17AC1859EF7E2AA0C8037D4D0F6"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::CTR, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::CTR, iv, padding_type::pkcs) | int8, result, example0);

    result = "E1295492CFAA94CAE26CB0F0FB8C960090571F8F56D09F92083E41B64F9B6E3F455628C26FA2793BD787C54EC13A5E59"sv;
    DATAFORGE_TEST(int8 | des_qrk(1u, key64, cipher_mode_type::PCBC, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(1u, key64, cipher_mode_type::PCBC, iv, padding_type::pkcs) | int8, result, example0);

    std::array<unsigned char, 24> key192 = {
       0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
       0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
       0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80 };

    result = "160A2DF48C4769DEB288C27270062AC0C1B5CCAC172E5827D9C8A941CFFB82BD760EEC01569D0FF9909398995CB0A1D6"sv;
    DATAFORGE_TEST(int8 | des_qrk(3u, key192) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / des_qrk(3u, key192) | int8, result, "The quick brown fox jumps over the lazy dog.\x0\x0\x0\x0"sv);
}

TEST(DataforgeTest, des)
{
    des_test();
}

void aes_test()
{
    auto example0 = "The quick brown fox jumps over the lazy dog."sv;
    auto key128 = "123456789abcdef0"_bs;

    auto result = "375CC68B56C49292847046AC5BAA10BE0B6DBA6C6E934BC7758ECAD6FB365070835D737344930915348D143F036B0B7A"sv;
    DATAFORGE_TEST(int8 | aes(128, key128, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key128, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, result, example0);
    
    auto key192 = "123456789abcdef012345678"_bs;
    result = "A2B7C53E42715F085AE7443584FA50D21A52E102FE9B432682150E6C425E8CE3FCE69D1401402EA996C56C8E91BFDE47"sv;
    DATAFORGE_TEST(int8 | aes(128, key192, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key192, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, result, example0);

    auto key256 = "123456789abcdef0123456789abcdef0"_bs;
    result = "CF74EDED2B95B0A6E6AF608E61AEB92FECF001C0C118F4C6E6F16E5B6CB44B53010D228C037AC8AA47879275BF086573"sv;
    DATAFORGE_TEST(int8 | aes(128, key256, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key256, cipher_mode_type::ECB, ""_bs, padding_type::pkcs) | int8, result, example0);

    std::vector<uint8_t> iv(16, 0x31);
    result = "A5D4B09435E308DF4CDEE63AC7A8E66FFEB0A3573A2B031C8F9584ED55DE42006CC8682F65EF603F96502B44B2119886"sv;
    DATAFORGE_TEST(int8 | aes(128, key128, cipher_mode_type::CBC, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key128, cipher_mode_type::CBC, iv, padding_type::pkcs) | int8, result, example0);

    result = "64618D5C216C28A389D434882D27032FA0C2544BCE8D3E79298869C3AD9AA8B30209CD623F183670089E36CDA494B32B"sv;
    DATAFORGE_TEST(int8 | aes(128, key192, cipher_mode_type::CBC, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key192, cipher_mode_type::CBC, iv, padding_type::pkcs) | int8, result, example0);

    result = "E53462932E513B64061E548A807971F3DB4829B5EF8E9AF8F3C956F181E264471E52B27DCFEA7F0D9EAA4DDF3C9E4424"sv;
    DATAFORGE_TEST(int8 | aes(128, key256, cipher_mode_type::CBC, iv, padding_type::pkcs) / int8 | base16u, example0, result);
    DATAFORGE_TEST(base16u | int8 / aes(128, key256, cipher_mode_type::CBC, iv, padding_type::pkcs) | int8, result, example0);
}

TEST(DataforgeTest, aes)
{
    aes_test();
}

void belt_test()
{
    auto flt = filter<char>([](char c) { return c != ' '; });
    auto grp = grouped_chars(8, ""_sp, ""_sp, " "_sp);

    // encrypt/decrypt one block
    std::vector<char> key;
    (quark_push_iterator{ int8 | flt / base16u | int8, std::back_inserter(key) } << "E9DEE72C 8F0C0FA6 2DDB49F4 6F739647 06075316 ED247A37 39CBA383 03A98BF6"sv).finish();

    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, "B194BAC8 0A08F53B 366D008E 584A5DE4"sv, "69CCA1C9 3557C9E3 D66BC3E0 FA88FA6E"sv);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, "69CCA1C9 3557C9E3 D66BC3E0 FA88FA6E"sv, "B194BAC8 0A08F53B 366D008E 584A5DE4"sv);
    
    std::vector<char> key1;
    (quark_push_iterator{ int8 | flt / base16u | int8, std::back_inserter(key1) } << "92BD9B1C E5D14101 5445FBC9 5E4D0EF2 682080AA 227D642F 2687F934 90405511"sv).finish();
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, "0DC53006 00CAB840 B38448E5 E993F421"sv, "E12BDC1A E28257EC 703FCCF0 95EE8DF1"sv);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, "E12BDC1A E28257EC 703FCCF0 95EE8DF1"sv, "0DC53006 00CAB840 B38448E5 E993F421"sv);

    // encrypt/decrypt by belt-ecb
    auto in0 = "B194BAC8 0A08F53B 366D008E 584A5DE4 8504FA9D 1BB6C7AC 252E72C2 02FDCE0D 5BE3D612 17B96181 FE6786AD 716B890B"sv;
    auto out0 = "69CCA1C9 3557C9E3 D66BC3E0 FA88FA6E 5F23102E F1097107 75017F73 806DA9DC 46FB2ED2 CE771F26 DCB5E5D1 569F9AB0"sv;

    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, in0, out0);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, out0, in0);

    auto in1 = "B194BAC8 0A08F53B 366D008E 584A5DE4 8504FA9D 1BB6C7AC 252E72C2 02FDCE0D 5BE3D612 17B96181 FE6786AD 716B89"sv;
    auto out1 = "69CCA1C9 3557C9E3 D66BC3E0 FA88FA6E 36F00CFE D6D1CA14 98C12798 F4BEB207 5F23102E F1097107 75017F73 806DA9"sv;

    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, in1, out1);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, out1, in1);

    auto out2 = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B 637C306A DD4EA779 9EB23D31"sv;
    auto in2 = "0DC53006 00CAB840 B38448E5 E993F421 E55A239F 2AB5C5D5 FDB6E81B 40938E2A 54120CA3 E6E19C7A D750FC35 31DAEAB7"sv;

    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, in2, out2);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, out2, in2);

    auto out3 = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B"sv;
    auto in3 = "0DC53006 00CAB840 B38448E5 E993F421 5780A6E2 B69EAFBB 258726D7 B6718523 E55A239F"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16u | grp, in3, out3);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16u | grp, out3, in3);

    std::vector<char> iv;
    (quark_push_iterator{ std::move(int8 | flt / base16u | int8), std::back_inserter(iv) } << "BE329713 43FC9A48 A02A885F 194B09A1"sv).finish();

    auto out0_cbc = "10116EFA E6AD58EE 14852E11 DA1B8A74 5CF2480E 8D03F1C1 9492E53E D3A70F60 657C1EE8 C0E0AE5B 58388BF8 A68E3309"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::CBC, iv, padding_type::none) / int8 | base16u | grp, in0, out0_cbc);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::CBC, iv, padding_type::none) | int8 | base16u | grp, out0_cbc, in0);

    auto in4_cbc = "B194BAC8 0A08F53B 366D008E 584A5DE4 8504FA9D 1BB6C7AC 252E72C2 02FDCE0D 5BE3D612"sv;
    auto out4_cbc = "10116EFA E6AD58EE 14852E11 DA1B8A74 6A9BBADC AF73F968 F875DEDC 0A44F6B1 5CF2480E"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::CBC, iv, padding_type::none) / int8 | base16u | grp, in4_cbc, out4_cbc);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::CBC, iv, padding_type::none) | int8 | base16u | grp, out4_cbc, in4_cbc);

    std::vector<char> iv1;
    (quark_push_iterator{ std::move(int8 | flt / base16u | int8), std::back_inserter(iv1) } << "7ECDA4D0 1544AF8C A58450BF 66D2E88A"sv).finish();

    auto in5_cbc = "730894D6 158E17CC 1600185A 8F411CAB 0471FF85 C8379239 8D8924EB D57D03DB 95B97A9B 7907E4B0 20960455 E46176F8"sv;
    auto out5_cbc = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B 637C306A DD4EA779 9EB23D31"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::CBC, iv1, padding_type::none) / int8 | base16u | grp, in5_cbc, out5_cbc);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::CBC, iv1, padding_type::none) | int8 | base16u | grp, out5_cbc, in5_cbc);

    auto in6_cbc = "730894D6 158E17CC 1600185A 8F411CAB B6AB7AF8 541CF857 55B8EA27 239F08D2 166646E4"sv;
    auto out6_cbc = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::CBC, iv1, padding_type::none) / int8 | base16u | grp, in6_cbc, out6_cbc);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::CBC, iv1, padding_type::none) | int8 | base16u | grp, out6_cbc, in6_cbc);

    // cfb
    auto in0_cfb = "B194BAC8 0A08F53B 366D008E 584A5DE4 8504FA9D 1BB6C7AC 252E72C2 02FDCE0D 5BE3D612 17B96181 FE6786AD 716B890B"sv;
    auto out0_cfb = "C31E490A 90EFA374 626CC99E 4B7B8540 A6E48685 464A5A06 849C9CA7 69A1B0AE 55C2CC59 39303EC8 32DD2FE1 6C8E5A1B"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::CFB, iv, padding_type::none) / int8 | base16u | grp, in0_cfb, out0_cfb);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::CFB, iv, padding_type::none) | int8 | base16u | grp, out0_cfb, in0_cfb);

    auto in1_cfb = "FA9D107A 86F375EE 65CD1DB8 81224BD0 16AFF814 938ED39B 3361ABB0 BF0851B6 52244EB0 6842DD4C 94AA4500 774E40BB"sv;
    auto out1_cfb = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B 637C306A DD4EA779 9EB23D31"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::CFB, iv1, padding_type::none) / int8 | base16u | grp, in1_cfb, out1_cfb);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::CFB, iv1, padding_type::none) | int8 | base16u | grp, out1_cfb, in1_cfb);

    // ctr
    auto in0_ctr = "B194BAC8 0A08F53B 366D008E 584A5DE4 8504FA9D 1BB6C7AC 252E72C2 02FDCE0D 5BE3D612 17B96181 FE6786AD 716B890B"sv;
    auto out0_ctr = "52C9AF96 FF50F644 35FC43DE F56BD797 D5B5B1FF 79FB4125 7AB9CDF6 E63E81F8 F0034147 3EAE4098 33622DE0 5213773A"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key, cipher_mode_type::CTR, iv, padding_type::none) / int8 | base16u | grp, in0_ctr, out0_ctr);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key, cipher_mode_type::CTR, iv, padding_type::none) | int8 | base16u | grp, out0_ctr, in0_ctr);

    auto in1_ctr = "DF181ED0 08A20F43 DCBBB936 50DAD34B 389CDEE5 826D40E2 D4BD80F4 9A93F5D2 12F63331 66456F16 9043CC5F"sv;
    auto out1_ctr = "E12BDC1A E28257EC 703FCCF0 95EE8DF1 C1AB7638 9FE678CA F7C6F860 D5BB9C4F F33C657B 637C306A DD4EA779"sv;
    DATAFORGE_TEST(int8 | flt / base16u | int8 | belt(key1, cipher_mode_type::CTR, iv1, padding_type::none) / int8 | base16u | grp, in1_ctr, out1_ctr);
    DATAFORGE_TEST(int8 | flt / base16u | int8 / belt(key1, cipher_mode_type::CTR, iv1, padding_type::none) | int8 | base16u | grp, out1_ctr, in1_ctr);
}

TEST(DataforgeTest, belt)
{
    belt_test();
}

void magma_test()
{
    //auto flt = filter<char>([](char c) { return c != ' '; });
    std::vector<char> key;
    (quark_push_iterator{ base16l | int8, std::back_inserter(key) } << "ffeeddccbbaa99887766554433221100f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff"sv).finish();

    DATAFORGE_TEST(base16u | int8 | magma(key, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16l, "fedcba9876543210"sv, "4ee901e5c2d8ca3d"sv);
    DATAFORGE_TEST(base16u | int8 / magma(key, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16l, "4ee901e5c2d8ca3d"sv, "fedcba9876543210"sv);
    DATAFORGE_TEST(base16u | int8 | magma(key, cipher_mode_type::ECB, ""_bs, padding_type::none) / int8 | base16l, "92def06b3c130a59"sv, "2b073f0494f372a0"sv);
    DATAFORGE_TEST(base16u | int8 / magma(key, cipher_mode_type::ECB, ""_bs, padding_type::none) | int8 | base16l, "2b073f0494f372a0"sv, "92def06b3c130a59"sv);
}

TEST(DataforgeTest, magma)
{
    magma_test();
}

//void kuznyechik_test()
//{
//
//}

}
