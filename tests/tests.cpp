
#include "test_common.hpp"

#include <unicode/putil.h>
#include <unicode/uclean.h>

#include <iostream>

using namespace dataforge;

TEST(DataforgeTest, generic_base) { generic_base_test(); }
TEST(DataforgeTest, base16) { base16_test(); }
TEST(DataforgeTest, base32) { base32_test(); }
TEST(DataforgeTest, base58) { base58_test(); }
TEST(DataforgeTest, base64) { base64_test(); }
TEST(DataforgeTest, ascii85) { ascii85_test(); }
TEST(DataforgeTest, z85) { z85_test(); }

TEST(DataforgeTest, utf) { utf_test(); }
TEST(DataforgeTest, graheme_break) { grapheme_break_test(); }
TEST(DataforgeTest, icu) { icu_test(); }

TEST(DataforgeTest, bsd_checksum) { bsd_checksum_test(); }
TEST(DataforgeTest, adler32) { adler32_test(); }
TEST(DataforgeTest, crc) { crc_test(); }

TEST(DataforgeTest, md2) { md2_test(); }
TEST(DataforgeTest, md4) { md4_test(); }
TEST(DataforgeTest, md5) { md5_test(); }
TEST(DataforgeTest, md6) { md6_test(); }
TEST(DataforgeTest, ripemd) { ripemd_test(); }
TEST(DataforgeTest, tiger) { tiger_test(); }
TEST(DataforgeTest, sha1) { sha1_test(); }
TEST(DataforgeTest, sha2) { sha2_test(); }
TEST(DataforgeTest, sha3) { sha3_test(); }
TEST(DataforgeTest, belt_hash) { belt_hash_test(); }
TEST(DataforgeTest, gost) { gost_test(); }
TEST(DataforgeTest, streebog) { streebog_test(); }
TEST(DataforgeTest, whirlpool) { whirlpool_test(); }
TEST(DataforgeTest, blake) { blake_test(); }

TEST(DataforgeTest, rc2) { rc2_test(); }
TEST(DataforgeTest, rc4) { rc4_test(); }
TEST(DataforgeTest, rc5) { rc5_test(); }
TEST(DataforgeTest, rc6) { rc6_test(); }
TEST(DataforgeTest, des) { des_test(); }
TEST(DataforgeTest, aes) { aes_test(); }
TEST(DataforgeTest, blowfish) { blowfish_test(); }
TEST(DataforgeTest, belt) { belt_test(); }
TEST(DataforgeTest, magma) { magma_test(); }
//TEST(DataforgeTest, magma) { kuznyechik_test(); }

TEST(DataforgeTest, group) { group_test(); }
TEST(DataforgeTest, deflate) { deflate_test(); }
TEST(DataforgeTest, bzip2) { bzip2_test(); }
TEST(DataforgeTest, lzma) { lzma_test(); }
TEST(DataforgeTest, lz4) { lz4_test(); }

int main(int argc, char** argv)
{
    // the path to e.g. icudt72l.dat
    //u_setDataDirectory("icu/bin64/data");

    UErrorCode errCode = U_ZERO_ERROR;
    u_init(&errCode);
    if (U_FAILURE(errCode)) {
        std::cerr << "can't initialize ICU library, err: " << (int)errCode;
        return 1;
    }

    ///////////////
    ::testing::InitGoogleTest(&argc, argv);
    u_cleanup();

    return RUN_ALL_TESTS();
}
