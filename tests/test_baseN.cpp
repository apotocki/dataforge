/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "test_common.hpp"

#include "dataforge/base_xx/base.hpp"
#include "dataforge/base_xx/base16.hpp"
#include "dataforge/base_xx/base32.hpp"
#include "dataforge/base_xx/base58.hpp"
#include "dataforge/base_xx/base64.hpp"
#include "dataforge/base_xx/ascii85.hpp"
#include "dataforge/base_xx/z85.hpp"

#include "dataforge/basic/buffer.hpp"

namespace dataforge {

///////////////
struct base32_encode_test_set
{
    const char* testtext[5] = { "foo", "foob", "fooba", "foobar", "foobars" };
    const char* expresults[5] = { "MZXW6===", "MZXW6YQ=", "MZXW6YTB", "MZXW6YTBOI======", "MZXW6YTBOJZQ====" };
};

struct base32_decode_test_set
{
    const char* testtext[5] = { "MZXW6===", "MZXW6YQ=", "MZXW6YTB", "MZXW6YTBOI======", "MZXW6YTBOJZQ====" };
    const char* expresults[5] = { "foo", "foob", "fooba", "foobar", "foobars" };
};

///////////////
struct base32hex_encode_test_set
{
    const char* testtext[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
    const char* expresults[7] = { "", "CO======", "CPNG====", "CPNMU===", "CPNMUOG=", "CPNMUOJ1", "CPNMUOJ1E8======" };
};

struct base32hex_decode_test_set
{
    const char* testtext[7] = { "", "CO======", "CPNG====", "CPNMU===", "CPNMUOG=", "CPNMUOJ1", "CPNMUOJ1E8======" };
    const char* expresults[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
};

///////////////
struct base32cf_encode_test_set
{
    const char* testtext[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
    const char* expresults[7] = { "", "CR", "CSQG", "CSQPY", "CSQPYRG", "CSQPYRK1", "CSQPYRK1E8" };
};

struct base32cf_decode_test_set
{
    const char* testtext[7] = { "", "CR", "CSQG", "CSQPY", "CSQPYRG", "CS-QP-YR-K1", "CSQPY-RK1E-8" };
    const char* expresults[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
};

///////////////
struct zbase32_encode_test_set
{
    const char* testtext[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
    const char* expresults[7] = { "", "ca", "c3zo", "c3zs6", "c3zs6ao", "c3zs6aub", "c3zs6aubqe" };
};

struct zbase32_decode_test_set
{
    const char* testtext[7] = { "", "ca", "c3zo", "c3zs6", "c3zs6ao", "c3zs6aub", "c3zs6aubqe" };
    const char* expresults[7] = { "", "f", "fo", "foo", "foob", "fooba", "foobar" };
};

///////////////

struct base64_encode_test_set
{
    const char* testtext[4] = { "12345678", "123456789", "123456789a", "123456789ab" };
    const char* expresults[4] = { "MTIzNDU2Nzg=", "MTIzNDU2Nzg5", "MTIzNDU2Nzg5YQ==", "MTIzNDU2Nzg5YWI=" };
};

struct base64_decode_test_set
{
    const char* testtext[4] = { "MTIzNDU2Nzg=", "MTIzNDU2Nzg5", "MTIzNDU2Nzg5YQ==", "MTIzNDU2Nzg5YWI=" };
    const char* expresults[4] = { "12345678", "123456789", "123456789a", "123456789ab" };
};

///////////////

struct ascii85_encode_test_set
{
    //const char* testtext[1] = { "D" };
    //const char* expresults[1] = { "6i" };
    const char* testtext[6] = { "!", "!2", "!2Z", "!2Z0", "R!2Z0", "R!2Z    X" };
    const char* expresults[6] = { "+T", "+Yj", "+Ym4", "+Ym4T", ";B[f!0E", ";B[f!+<VdL=9" };
};

struct ascii85_decode_test_set
{
    const char* testtext[6] = { "+T", "+Yj", "+Ym4", "+Ym4T", ";B[f!0E", ";B[f!+<VdL=9" };
    const char* expresults[6] = { "!", "!2", "!2Z", "!2Z0", "R!2Z0", "R!2Z    X" };
};

struct z85_encode_test_set
{
    //const char* testtext[1] = { "\xd0\xa0\xd0\xa1" };
    //const char* expresults[1] = { "^4F:V" };
    const char* testtext[7] = { "\xd0\xa0\xd0\xa1", "!", "!2", "!2Z", "!2Z0", "R!2Z0", "R!2Z    X" };
    const char* expresults[7] = { "^4F:V", "aP", "aU<", "aU)j", "aU)jP", "qxW/0fA", "qxW/0arR^Hso" };
};

struct z85_decode_test_set
{
    //const char* testtext[1] = { "^4F:V" }; 
    //const char* expresults[1] = { "\xd0\xa0\xd0\xa1" };
    const char* testtext[7] = { "^4F:V", "aP", "aU<", "aU)j", "aU)jP", "qxW/0fA", "qxW/0arR^Hso" };
    const char* expresults[7] = { "\xd0\xa0\xd0\xa1", "!", "!2", "!2Z", "!2Z0", "R!2Z0", "R!2Z    X" };
};

/*

}

#include <sstream>
#include <vector>
#include <iostream>
#include "dataforge/group.hpp"

namespace dataforge {

void matrix_builder()
{
    using namespace dataforge;
    const char* alphabet = base32_detail::z_alphabet;

    std::vector<uint8_t> matrix;
    matrix.resize(256, 0xff);
    for (uint8_t i = 0; i < 32; ++i) {
        matrix[alphabet[i]] = i;
    }

    std::ostringstream rss;
    auto cvt_it = quark_push_iterator(
        int8 | base16l / int8 | grouped_chars(2, "0x"_bs, ""_bs, ", "_bs, 16), std::ostreambuf_iterator(rss));
    *cvt_it = std::span{ matrix };
    cvt_it.flush();
    std::cout << rss.str() << "\n";
}

*/


void generic_base_test()
{
    //const char alphabet36[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const char alphabet36[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    const unsigned char in_36_0[] = { 0xff, 0xee };
    std::string strval_36_0 = "1ejy";
    DATAFORGE_PUSH_TEST(int8 | base(std::span{ alphabet36, 36 }), in_36_0, strval_36_0);
    DATAFORGE_PUSH_TEST(base(std::span{ alphabet36, 36 }) | int8, strval_36_0, std::span{ in_36_0 });
}

TEST(DataforgeTest, generic_base)
{
    generic_base_test();
}

void base16_test()
{
    std::string in0 = "0123456789aAbBcCdDeEfF";
    const unsigned char res0[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    DATAFORGE_TEST(base16l | int8, in0, std::span{ res0 });
    DATAFORGE_TEST(int8 | base16l, res0, std::string("0123456789aabbccddeeff"));
    DATAFORGE_TEST(int8 | base16u, res0, std::string("0123456789AABBCCDDEEFF"));
}

TEST(DataforgeTest, base16)
{
    base16_test();
}

void base32_test()
{
    DATAFORGE_PUSH_TEST_SET(int8 | base32, base32_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(base32 | int8, base32_decode_test_set{});
    DATAFORGE_PUSH_TEST_SET(int8 | base32hex, base32hex_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(base32hex | int8, base32hex_decode_test_set{});
    DATAFORGE_PUSH_TEST_SET(int8 | base32cf, base32cf_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(base32cf | int8, base32cf_decode_test_set{});
    DATAFORGE_PUSH_TEST_SET(int8 | zbase32, zbase32_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(zbase32 | int8, zbase32_decode_test_set{});
}

TEST(DataforgeTest, base32)
{
    base32_test();
}

void base58_test()
{
    const unsigned char in[] = { 58 };
    std::string strval = "21";
    DATAFORGE_PUSH_TEST(int8 | base58<false>(base58_type::BITCOIN), in, strval);
    DATAFORGE_PUSH_TEST(base58(base58_type::BITCOIN) | int8, strval, std::span{ in });

    std::vector<char> result;
    auto pconv = dynamic_push_converter<uint8_t> { int8 | base58<false>(base58_type::BITCOIN) / int8 | buffer<uint8_t>(65536), std::back_inserter(result) };
    pconv.push(in[0]);
    pconv.finish();
    EXPECT_TRUE(equal_to(result, strval)) << "ERROR";

    const unsigned char in0[] = { 0, 'C', 'a', 't', 'C', 'a' };
    std::string strval0 = "18bv6TtL";
    DATAFORGE_PUSH_TEST(int8 | base58(base58_type::BITCOIN), in0, strval0);
    DATAFORGE_PUSH_TEST(base58(base58_type::BITCOIN) | int8, strval0, std::span{ in0 });

    std::string strval1 = "StV1DL6CwTryKyV";
    std::string in1 = "hello world";
    DATAFORGE_PUSH_TEST(int8 | base58(base58_type::BITCOIN), in1, strval1);
    DATAFORGE_PUSH_TEST(base58(base58_type::BITCOIN) | int8, strval1, std::span{ in1 });
    

    std::string strval2 = "16ftSEQ4ctQFDtVZiUBusQUjRrGhM3JYwe";
    const unsigned char in2[] = { 0x00, 0x3e, 0x33, 0x5f, 0x0a, 0xb2, 0x09, 0xa2, 0x24, 0x9f, 0xb6, 0x4b, 0xad, 0xf3, 0xc1, 0xe4, 0xb0, 0xa8, 0xc8, 0xea, 0x70, 0x11, 0x7e, 0xd3, 0x85 };
    const unsigned char in2_c[] = { 0x00, 0x3e, 0x33, 0x5f, 0x0a, 0xb2, 0x09, 0xa2, 0x24, 0x9f, 0xb6, 0x4b, 0xad, 0xf3, 0xc1, 0xe4, 0xb0, 0xa8, 0xc8, 0xea, 0x70 };
    DATAFORGE_PUSH_TEST(int8 | base58(base58_type::BITCOIN), in2, strval2);
    DATAFORGE_PUSH_TEST(base58(base58_type::BITCOIN) | int8, strval2, std::span{ in2 });
    DATAFORGE_PUSH_TEST(base58check(base58_type::BITCOIN) | int8, strval2, std::span{ in2_c });
    DATAFORGE_PUSH_TEST(int8 | base58check(base58_type::BITCOIN), in2_c, strval2);

    std::string in3 = "hello world";
    std::string strval3 = "3vQB7B6MrGQZaxCuFg4oh";
    DATAFORGE_PUSH_TEST(int8 | base58check(base58_type::BITCOIN), in3, strval3);
    DATAFORGE_PUSH_TEST(base58check(base58_type::BITCOIN) | int8, strval3, std::span{ in3 });

    std::string strval4_gmp = "1LDlk6QWOejX6rPrJ";
    std::string strval4_bitcoin = "2NEpo7TZRhna7vSvL";
    std::string in4 = "Hello world!";
    DATAFORGE_PUSH_TEST(int8 | base58(base58_type::GMP), in4, strval4_gmp);
    DATAFORGE_PUSH_TEST(base58(base58_type::GMP) | int8, strval4_gmp, std::span{ in4 });
    DATAFORGE_PUSH_TEST(int8 | base58(base58_type::BITCOIN), in4, strval4_bitcoin);
    DATAFORGE_PUSH_TEST(base58(base58_type::BITCOIN) | int8, strval4_bitcoin, std::span{ in4 });
}

TEST(DataforgeTest, base58)
{
    base58_test();
}

void base64_test()
{
    DATAFORGE_TEST_SET(int8 | base64, base64_encode_test_set{});
    DATAFORGE_TEST_SET(base64f('=') | int8, base64_decode_test_set{});
}

TEST(DataforgeTest, base64)
{
    base64_test();
}

void ascii85_test()
{
    DATAFORGE_PUSH_TEST_SET(int8 | ascii85, ascii85_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(ascii85 | int8, ascii85_decode_test_set{});
}

TEST(DataforgeTest, ascii85)
{
    ascii85_test();
}

void z85_test()
{
    DATAFORGE_PUSH_TEST_SET(int8 | z85, z85_encode_test_set{});
    DATAFORGE_PUSH_TEST_SET(z85 | int8, z85_decode_test_set{});
}

TEST(DataforgeTest, z85)
{
    z85_test();
}

}
