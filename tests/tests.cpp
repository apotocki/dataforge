
#include "test_common.hpp"

#include <unicode/putil.h>
#include <unicode/uclean.h>

#include <iostream>

int main(int argc, char** argv)
{

    // the path of e.g. icudt72l.dat
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
    /*
    utf_test();
    graheme_break_test();
    icu_test();

    generic_base_test();
    base16_test();
    base32_test();
    base58_test();
    base64_test();
    ascii85_test();
    z85_test();

    bsd_checksum_test();
    adler32_test();
    crc_test();

    md2_test();
    md4_test();
    md5_test();
    md6_test();
    ripemd_test();
    sha1_test();
    sha2_test();
    sha3_test();
    tiger_test();
    gost_test();
    streebog_test();
    belt_hash_test();
    whirlpool_test();
    blake_test();

    blowfish_test();
    rc2_test();
    rc4_test();
    rc5_test();
    rc6_test();
    des_test();
    aes_test();
    belt_test();
    magma_test();
    kuznyechik_test();

    group_test();
    deflate_test();
    bzip2_test();
    lzma_test();
    lz4_test();
    */
    
}
