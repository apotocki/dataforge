/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "test_common.hpp"

#include "dataforge/hashes/gost.hpp"
#include "dataforge/hashes/streebog.hpp"
#include "dataforge/hashes/belt.hpp"
#include "dataforge/hashes/tiger.hpp"
#include "dataforge/hashes/sha1.hpp"
#include "dataforge/hashes/sha2.hpp"
#include "dataforge/hashes/sha3.hpp"
#include "dataforge/hashes/md2.hpp"
#include "dataforge/hashes/md4.hpp"
#include "dataforge/hashes/md5.hpp"
#include "dataforge/hashes/md6.hpp"
#include "dataforge/hashes/ripemd.hpp"
#include "dataforge/hashes/whirlpool.hpp"
#include "dataforge/hashes/blake.hpp"
#include "dataforge/base_xx/base16.hpp"

using namespace std::literals::string_view_literals;

namespace dataforge {

void md2_test()
{
    DATAFORGE_TEST(int8 | md2 | base16l, ""sv, "8350e5a3e24c153df2275c9f80692773"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "a"sv, "32ec01ec4a6dac72c0ab96fb34c0b5d1"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "abc"sv, "da853b0d3f88d99b30283a69e6ded6bb"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "message digest"sv, "ab4f496bfb2a530b219ff33031fe06b0"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "abcdefghijklmnopqrstuvwxyz"sv, "4e8ddff3650292ab5a4108c3aa47940b"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"sv, "da33def2a42df13975352846c30338cd"sv);
    DATAFORGE_TEST(int8 | md2 | base16l, "123456789012345678901234567890123456789012345678901234567890123456 78901234567890"sv, "05dbba941443332475b8e3f572f5d148"sv);

    std::vector<uint8_t> example7;
    example7.resize(800);
    for (size_t i = 0; i < example7.size(); ++i) example7[i] = 0x11 * (1 + (i % 7));
    DATAFORGE_TEST(int8 | md2 | base16l, example7, "e2d902d6d1226800983c00acdb47316b"sv);
    
    size_t slice = 10;
    std::string result;
    auto cvt_it = quark_push_iterator{int8 | md2 | base16l, std::back_inserter(result)};
    *cvt_it = std::span{ &example7.front(), slice };
    *cvt_it = std::span{ &example7[slice], 6u };
    *cvt_it = std::span{ &example7[16], example7.size() - 16 };
    cvt_it.finish();
    EXPECT_TRUE(equal_to(result, "e2d902d6d1226800983c00acdb47316b"sv)) << "ranges are different";
}

TEST(DataforgeTest, md2)
{
    md2_test();
}

void md4_test()
{
    DATAFORGE_TEST(int8 | md4 | base16l, "The quick brown fox jumps over the lazy dog"sv, "1bee69a46ba811185c194762abaeae90"sv);
    DATAFORGE_TEST(int8 | md4 | base16l, "The quick brown fox jumps over the lazy cog"sv, "b86e130ce7028da59e672d56ad0113df"sv);
    DATAFORGE_TEST(int8 | md4 | base16l, ""sv, "31d6cfe0d16ae931b73c59d7e0c089c0"sv);
}

TEST(DataforgeTest, md4)
{
    md4_test();
}

void md5_test()
{
    DATAFORGE_TEST(int8 | md5 | base16u, "md5"sv, "1BC29B36F623BA82AAF6724FD3B16718"sv);
    DATAFORGE_TEST(int8 | md5 | base16u, "md4"sv, "C93D3BF7A7C4AFE94B64E30C2CE39F4F"sv);
    DATAFORGE_TEST(int8 | md5 | base16u, ""sv, "D41D8CD98F00B204E9800998ECF8427E"sv);
}

TEST(DataforgeTest, md5)
{
    md5_test();
}

void md6_test()
{
    DATAFORGE_TEST(int8 | md6(128) | base16l, "test"sv, "a133b0efa199156be653427c6ab85d3d"sv);
    DATAFORGE_TEST(int8 | md6(256, 5) | base16l, "abc"sv, "8854c14dc284f840ed71ad7ba542855ce189633e48c797a55121a746be48cec8"sv);

    std::vector<uint8_t> example2;
    example2.resize(600);
    for (size_t i = 0; i < example2.size(); ++i) example2[i] = 0x11 * (1 + (i % 7));
    DATAFORGE_TEST(int8 | md6(224, 5, "abcde12345"_bs) | base16l, example2, "894cf0598ad3288ed4bb5ac5df23eba0ac388a11b7ed2e3dd5ec5131"sv);

    std::vector<uint8_t> example3;
    example3.resize(800);
    for (size_t i = 0; i < example3.size(); ++i) example3[i] = 0x11 * (1 + (i % 7));
    DATAFORGE_TEST(int8 | md6(256, -1, ""_bs, 0) | base16l, example3, "4e78ab5ec8926a3db0dcfa09ed48de6c33a7399e70f01ebfc02abb52767594e2"sv);
}

TEST(DataforgeTest, md6)
{
    md6_test();
}

void ripemd_test()
{
    std::string example1 = "The quick brown fox jumps over the lazy dog";
    std::string example2 = "The quick brown fox jumps over the lazy cog";

    DATAFORGE_TEST(int8 | ripemd128 | base16l, ""sv, "cdf26213a150dc3ecb610f18f6b38b46"sv);
    DATAFORGE_TEST(int8 | ripemd128 | base16l, "a"sv, "86be7afa339d0fc7cfc785e72f578d33"sv);
    DATAFORGE_TEST(int8 | ripemd128 | base16l, "abc"sv, "c14a12199c66e4ba84636b0f69144c77"sv);
    DATAFORGE_TEST(int8 | ripemd128 | base16l, "aaa100"sv, "5b250e8d7ee4fd67f35c3d193c6648c4"sv);
    DATAFORGE_TEST(int8 | ripemd128 | base16l, "aaa101"sv, "e607de9b0ca4fe01be84f87b83d8b5a3"sv);
    
    DATAFORGE_TEST(int8 | ripemd128 | base16l, example1, "3fa9b57f053c053fbe2735b2380db596"sv);
    DATAFORGE_TEST(int8 | ripemd128 | base16l, example2, "3807aaaec58fe336733fa55ed13259d9"sv);
        
    DATAFORGE_TEST(int8 | ripemd160 | base16l, ""sv, "9c1185a5c5e9fc54612808977ee8f548b2258d31"sv);
    DATAFORGE_TEST(int8 | ripemd160 | base16l, example1, "37f332f68db77bd9d7edd4969571ad671cf9dd3b"sv);
    DATAFORGE_TEST(int8 | ripemd160 | base16l, example2, "132072df690933835eb8b6ad0b77e7b6f14acad7"sv);

    DATAFORGE_TEST(int8 | ripemd256 | base16l, ""sv, "02ba4c4e5f8ecd1877fc52d64d30e37a2d9774fb1e5d026380ae0168e3c5522d"sv);
    DATAFORGE_TEST(int8 | ripemd256 | base16l, "a"sv, "f9333e45d857f5d90a91bab70a1eba0cfb1be4b0783c9acfcd883a9134692925"sv);
    DATAFORGE_TEST(int8 | ripemd256 | base16l, example1, "c3b0c2f764ac6d576a6c430fb61a6f2255b4fa833e094b1ba8c1e29b6353036f"sv);
    DATAFORGE_TEST(int8 | ripemd256 | base16l, example2, "b44055d843dea5bcd2151e52b1a0dbc5e8e34493e5fe2f000c0e71f73c3ddcae"sv);

    DATAFORGE_TEST(int8 | ripemd320 | base16l, ""sv, "22d65d5661536cdc75c1fdf5c6de7b41b9f27325ebc61e8557177d705a0ec880151c3a32a00899b8"sv);
    DATAFORGE_TEST(int8 | ripemd320 | base16l, "a"sv, "ce78850638f92658a5a585097579926dda667a5716562cfcf6fbe77f63542f99b04705d6970dff5d"sv);
    DATAFORGE_TEST(int8 | ripemd320 | base16l, example1, "e7660e67549435c62141e51c9ab1dcc3b1ee9f65c0b3e561ae8f58c5dba3d21997781cd1cc6fbc34"sv);
    DATAFORGE_TEST(int8 | ripemd320 | base16l, example2, "393e0df728c4ce3d79e7dcfd357d5c26f5c6d64c6d652dc53b6547b214ea9183e4f61c477ebf5cb0"sv);
}

TEST(DataforgeTest, ripemd)
{
    ripemd_test();
}

void tiger_test()
{
    DATAFORGE_TEST(int8 | tiger192_3 | base16l, ""sv, "3293ac630c13f0245f92bbb1766e16167a4e58492dde73f3"sv);
    DATAFORGE_TEST(int8 | tiger2_192 | base16u, ""sv, "4441BE75F6018773C206C22745374B924AA8313FEF919F41"sv);

    std::string example0 = "The quick brown fox jumps over the lazy dog";
    DATAFORGE_TEST(int8 | tiger128_3 | base16l, example0, "6d12a41e72e644f017b6f0e2f7b44c62"sv);
    DATAFORGE_TEST(int8 | tiger160_3 | base16l, example0, "6d12a41e72e644f017b6f0e2f7b44c6285f06dd5"sv);
    DATAFORGE_TEST(int8 | tiger192_3 | base16l, example0, "6d12a41e72e644f017b6f0e2f7b44c6285f06dd5d2c5b075"sv);
    DATAFORGE_TEST(int8 | tiger128_4 | base16l, example0, "c1f3a704e9f6267e9f75fa47191f83c3"sv);
    DATAFORGE_TEST(int8 | tiger160_4 | base16l, example0, "c1f3a704e9f6267e9f75fa47191f83c354100a04"sv);
    DATAFORGE_TEST(int8 | tiger192_4 | base16l, example0, "c1f3a704e9f6267e9f75fa47191f83c354100a04c4f1dc6f"sv);
    DATAFORGE_TEST(int8 | tiger2_192 | base16l, example0, "976abff8062a2e9dcea3a1ace966ed9c19cb85558b4976d8"sv);

    DATAFORGE_TEST(int8 | tiger2_192 | base16u, std::vector<char>(1), "860F73A57072EFACBBEA35F7DB67D97AA829C53463805093"sv);
    DATAFORGE_TEST(int8 | tiger2_192 | base16u, std::vector<char>(78), "92FA6750D57C08340C98743B2E0BAE08FA3FFEDF93686301"sv);
}

TEST(DataforgeTest, tiger)
{
    tiger_test();
}

void sha1_test()
{
    DATAFORGE_TEST(int8 | sha1 | base16l, "The quick brown fox jumps over the lazy dog"sv, "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12"sv);
    DATAFORGE_TEST(int8 | sha1 | base16l, "sha"sv, "d8f4590320e1343a915b6394170650a8f35d6926"sv);
    DATAFORGE_TEST(int8 | sha1 | base16l, "Sha"sv, "ba79baeb9f10896a46ae74715271b7f586e74640"sv);
    DATAFORGE_TEST(int8 | sha1 | base16l, ""sv, "da39a3ee5e6b4b0d3255bfef95601890afd80709"sv);
}

TEST(DataforgeTest, sha1)
{
    sha1_test();
}

void sha2_test()
{
    std::string example0 = "The quick brown fox jumps over the lazy dog";
    DATAFORGE_TEST(int8 | sha224 | base16u, example0, "730E109BD7A8A32B1CB9D9A09AA2325D2430587DDBC0C38BAD911525"sv);
    DATAFORGE_TEST(int8 | sha256 | base16u, example0, "D7A8FBB307D7809469CA9ABCB0082E4F8D5651E46D3CDB762D02D0BF37C9E592"sv);
    DATAFORGE_TEST(int8 | sha384 | base16u, example0, "CA737F1014A48F4C0B6DD43CB177B0AFD9E5169367544C494011E3317DBF9A509CB1E5DC1E85A941BBEE3D7F2AFBC9B1"sv);
    DATAFORGE_TEST(int8 | sha512 | base16u, example0, "07E547D9586F6A73F73FBAC0435ED76951218FB7D0C8D788A309D785436BBB642E93A252A954F23912547D1E8A3B5ED6E1BFD7097821233FA0538F3DB854FEE6"sv);
    DATAFORGE_TEST(int8 | sha512_224 | base16u, example0, "944CD2847FB54558D4775DB0485A50003111C8E5DAA63FE722C6AA37"sv);
    DATAFORGE_TEST(int8 | sha512_256 | base16u, example0, "DD9D67B371519C339ED8DBD25AF90E976A1EEEFD4AD3D889005E532FC5BEF04D"sv);
}

TEST(DataforgeTest, sha2)
{
    sha2_test();
}

void sha3_test()
{
    DATAFORGE_TEST(int8 | sha3_224 | base16l, ""sv, "6b4e03423667dbb73b6e15454f0eb1abd4597f9a1b078e3f5b5a6bc7"sv);
    DATAFORGE_TEST(int8 | sha3_256 | base16l, ""sv, "a7ffc6f8bf1ed76651c14756a061d662f580ff4de43b49fa82d80a4b80f8434a"sv);
    DATAFORGE_TEST(int8 | sha3_384 | base16l, ""sv, "0c63a75b845e4f7d01107d852e4c2485c51a50aaaa94fc61995e71bbee983a2ac3713831264adb47fb6bd1e058d5f004"sv);
    DATAFORGE_TEST(int8 | sha3_512 | base16l, ""sv, "a69f73cca23a9ac5c8b567dc185a756e97c982164fe25859e0d1dcc1475c80a615b2123af1f5f94c11e3e9402c3ac558f500199d95b6d3e301758586281dcd26"sv);

    std::string example0 = "The quick brown fox jumps over the lazy dog";
    std::string example1 = "The quick brown fox jumps over the lazy dog.";

    DATAFORGE_TEST(int8 | sha3_224 | base16l, example0, "d15dadceaa4d5d7bb3b48f446421d542e08ad8887305e28d58335795"sv);
    DATAFORGE_TEST(int8 | sha3_224 | base16l, example1, "2d0708903833afabdd232a20201176e8b58c5be8a6fe74265ac54db0"sv);

    DATAFORGE_TEST(int8 | sha3_256 | base16l, example0, "69070dda01975c8c120c3aada1b282394e7f032fa9cf32f4cb2259a0897dfc04"sv);
    DATAFORGE_TEST(int8 | sha3_256 | base16l, example1, "a80f839cd4f83f6c3dafc87feae470045e4eb0d366397d5c6ce34ba1739f734d"sv);

    DATAFORGE_TEST(int8 | sha3_384 | base16l, example0, "7063465e08a93bce31cd89d2e3ca8f602498696e253592ed26f07bf7e703cf328581e1471a7ba7ab119b1a9ebdf8be41"sv);
    DATAFORGE_TEST(int8 | sha3_384 | base16l, example1, "1a34d81695b622df178bc74df7124fe12fac0f64ba5250b78b99c1273d4b080168e10652894ecad5f1f4d5b965437fb9"sv);

    DATAFORGE_TEST(int8 | sha3_512 | base16l, example0, "01dedd5de4ef14642445ba5f5b97c15e47b9ad931326e4b0727cd94cefc44fff23f07bf543139939b49128caf436dc1bdee54fcb24023a08d9403f9b4bf0d450"sv);
    DATAFORGE_TEST(int8 | sha3_512 | base16l, example1, "18f4f4bd419603f95538837003d9d254c26c23765565162247483f65c50303597bc9ce4d289f21d1c2f1f458828e33dc442100331b35e7eb031b5d38ba6460f8"sv);

    DATAFORGE_TEST(int8 | shake_128(256) | base16l, ""sv, "7f9c2ba4e88f827d616045507605853ed73b8093f6efbc88eb1a6eacfa66ef26"sv);
    DATAFORGE_TEST(int8 | shake_256(512) | base16l, ""sv, "46b9dd2b0ba88d13233b3feb743eeb243fcd52ea62b81b82b50c27646ed5762fd75dc4ddd8c0f200cb05019d67b592f6fc821c49479ab48640292eacb3b7c4be"sv);

    std::string example2 = "The quick brown fox jumps over the lazy dof";
    DATAFORGE_TEST(int8 | shake_128(256) | base16l, example0, "f4202e3c5852f9182a0430fd8144f0a74b95e7417ecae17db0f8cfeed0e3e66e"sv);
    DATAFORGE_TEST(int8 | shake_128(256) | base16l, example2, "853f4538be0db9621a6cea659a06c1107b1f83f02b13d18297bd39d7411cf10c"sv);

    DATAFORGE_TEST(int8 | shake_256(512) | base16l, example0, "2f671343d9b2e1604dc9dcf0753e5fe15c7c64a0d283cbbf722d411a0e36f6ca1d01d1369a23539cd80f7c054b6e5daf9c962cad5b8ed5bd11998b40d5734442"sv);
    DATAFORGE_TEST(int8 | shake_256(512) | base16l, example2, "46b1ebb2e142c38b9ac9081bef72877fe4723959640fa57119b366ce6899d4013af024f4222921320bee7d3bfaba07a758cd0fde5d27bbd2f8d709f4307d2c34"sv);
}

TEST(DataforgeTest, sha3)
{
    sha3_test();
}

void belt_hash_test()
{
    DATAFORGE_TEST(base16u | int8 | belt_hash | base16u, "B194BAC80A08F53B366D008E58"sv, "ABEF9725D4C5A83597A367D14494CC2542F20F659DDFECC961A3EC550CBA8C75"sv);
    DATAFORGE_TEST(base16u | int8 | belt_hash | base16u, "B194BAC80A08F53B366D008E584A5DE48504FA9D1BB6C7AC252E72C202FDCE0D"sv, "749E4C3653AECE5E48DB4761227742EB6DBE13F4A80F7BEFF1A9CF8D10EE7786"sv);
    DATAFORGE_TEST(base16u | int8 | belt_hash | base16u, "B194BAC80A08F53B366D008E584A5DE48504FA9D1BB6C7AC252E72C202FDCE0D5BE3D61217B96181FE6786AD716B890B"sv, "9D02EE446FB6A29FE5C982D4B13AF9D3E90861BC4CEF27CF306BFB0B174A154A"sv);
}

TEST(DataforgeTest, belt_hash)
{
    belt_hash_test();
}

void gost_test()
{
    DATAFORGE_TEST(int8 | gost | base16l, "The quick brown fox jumps over the lazy dog"sv, "77b7fa410c9ac58a25f49bca7d0468c9296529315eaca76bd1a10f376d1f4294"sv);
    DATAFORGE_TEST(int8 | gost | base16l, "The quick brown fox jumps over the lazy cog"sv, "a3ebc4daaab78b0be131dab5737a7f67e602670d543521319150d2e14eeec445"sv);
    DATAFORGE_TEST(int8 | gost | base16l, "This is message, length=32 bytes"sv, "b1c466d37519b82e8319819ff32595e047a28cb6f83eff1c6916a815a637fffa"sv);
    DATAFORGE_TEST(int8 | gost | base16l, "Suppose the original message has length = 50 bytes"sv, "471aba57a60a770d3a76130635c1fbea4ef14de51f78b4ae57dd893b62f55208"sv);
    DATAFORGE_TEST(int8 | gost | base16l, ""sv, "ce85b99cc46752fffee35cab9a7b0278abb4c2d2055cff685af4912c49490f8d"sv);
    DATAFORGE_TEST(int8 | gost | base16l, "a"sv, "d42c539e367c66e9c88a801f6649349c21871b4344c6a573f849fdce62f314dd"sv);
    DATAFORGE_TEST(int8 | gost | base16l, "message digest"sv, "ad4434ecb18f2c99b60cbe59ec3d2469582b65273f48de72db2fde16a4889a4d"sv);

    std::vector<char> U128(128, 'U');
    DATAFORGE_TEST(int8 | gost | base16l, U128, "53a3a3ed25180cef0c1d85a074273e551c25660a87062a52d926a9e8fe5733a4"sv);

    std::vector<char> a1000000(1000000, 'a');
    DATAFORGE_TEST(int8 | gost | base16l, a1000000, "5c00ccc2734cdd3332d3d4749576e3c1a7dbaf0e7ea74e9fa602413c90a129fa"sv);

    DATAFORGE_TEST(int8 | gost_cpro | base16l, ""sv, "981e5f3ca30c841487830f84fb433e13ac1101569b9c13584ac483234cd656c0"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "a"sv, "e74c52dd282183bf37af0079c9f78055715a103f17e3133ceff1aacf2f403011"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "abc"sv, "b285056dbf18d7392d7677369524dd14747459ed8143997e163b2986f92fd42c"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "message digest"sv, "bc6041dd2aa401ebfa6e9886734174febdb4729aa972d60f549ac39b29721ba0"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "The quick brown fox jumps over the lazy dog"sv, "9004294a361a508c586fe53d1f1b02746765e71b765472786e4770d565830a76"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"sv, "73b70a39497de53a6e08c67b6d4db853540f03e9389299d9b0156ef7e85d0f61"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "12345678901234567890123456789012345678901234567890123456789012345678901234567890"sv, "6bc7b38989b28cf93ae8842bf9d752905910a7528a61e5bce0782de43e610c90"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "This is message, length=32 bytes"sv, "2cefc2f7b7bdc514e18ea57fa74ff357e7fa17d652c75f69cb1be7893ede48eb"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, "Suppose the original message has length = 50 bytes"sv, "c3730c5cbccacf915ac292676f21e8bd4ef75331d9405e5f1a61dc3130a65011"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, U128, "1c4ac7614691bbf427fa2316216be8f10d92edfd37cd1027514c1008f649c4e8"sv);
    DATAFORGE_TEST(int8 | gost_cpro | base16l, a1000000, "8693287aa62f9478f7cb312ec0866b6c4e4a0f11160441e8f4ffcd2715dd554f"sv);
}

TEST(DataforgeTest, gost)
{
    gost_test();
}

void streebog_test()
{
    DATAFORGE_TEST(base16l | int8 | streebog256 | base16l,
        "303132333435363738393031323334353637383930313233343536373839303132333435363738393031323334353637383930313233343536373839303132"sv,
        "9d151eefd8590b89daa6ba6cb74af9275dd051026bb149a452fd84e5e57b5500"sv);

    DATAFORGE_TEST(base16l | int8 | streebog256 | base16l,
        "d1e520e2e5f2f0e82c20d1f2f0e8e1eee6e820e2edf3f6e82c20e2e5fef2fa20f120eceef0ff20f1f2f0e5ebe0ece820ede020f5f0e0e1f0fbff20efebfaeafb20c8e3eef0e5e2fb"sv,
        "9dd2fe4e90409e5da87f53976d7405b0c0cac628fc669a741d50063c557e8f50"sv);

    DATAFORGE_TEST(base16l | int8 | streebog256 | base16l,
        ""sv,
        "3f539a213e97c802cc229d474c6aa32a825a360b2a933a949fd925208d9ce1bb"sv);

    DATAFORGE_TEST(base16l | int8 | streebog256 | base16l,
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"sv,
        "df1fda9ce83191390537358031db2ecaa6aa54cd0eda241dc107105e13636b95"sv);

    DATAFORGE_TEST(base16l | int8 | streebog512 | base16l,
        "303132333435363738393031323334353637383930313233343536373839303132333435363738393031323334353637383930313233343536373839303132"sv,
        "1b54d01a4af5b9d5cc3d86d68d285462b19abc2475222f35c085122be4ba1ffa00ad30f8767b3a82384c6574f024c311e2a481332b08ef7f41797891c1646f48"sv);

    DATAFORGE_TEST(base16l | int8 | streebog512 | base16l,
        "d1e520e2e5f2f0e82c20d1f2f0e8e1eee6e820e2edf3f6e82c20e2e5fef2fa20f120eceef0ff20f1f2f0e5ebe0ece820ede020f5f0e0e1f0fbff20efebfaeafb20c8e3eef0e5e2fb"sv,
        "1e88e62226bfca6f9994f1f2d51569e0daf8475a3b0fe61a5300eee46d961376035fe83549ada2b8620fcd7c496ce5b33f0cb9dddc2b6460143b03dabac9fb28"sv);

    DATAFORGE_TEST(base16l | int8 | streebog512 | base16l,
        ""sv,
        "8e945da209aa869f0455928529bcae4679e9873ab707b55315f56ceb98bef0a7362f715528356ee83cda5f2aac4c6ad2ba3a715c1bcd81cb8e9f90bf4c1c1a8a"sv);

    DATAFORGE_TEST(base16l | int8 | streebog512 | base16l,
        "00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000"sv,
        "b0fd29ac1b0df441769ff3fdb8dc564df67721d6ac06fb28ceffb7bbaa7948c6c014ac999235b58cb26fb60fb112a145d7b4ade9ae566bf2611402c552d20db7"sv);
}

TEST(DataforgeTest, streebog)
{
    streebog_test();
}

void whirlpool_test()
{
    DATAFORGE_TEST(int8 | whirlpool | base16u, "The quick brown fox jumps over the lazy dog"sv, "B97DE512E91E3828B40D2B0FDCE9CEB3C4A71F9BEA8D88E75C4FA854DF36725FD2B52EB6544EDCACD6F8BEDDFEA403CB55AE31F03AD62A5EF54E42EE82C3FB35"sv);
    DATAFORGE_TEST(int8 | whirlpool | base16u, "The quick brown fox jumps over the lazy eog"sv, "C27BA124205F72E6847F3E19834F925CC666D0974167AF915BB462420ED40CC50900D85A1F923219D832357750492D5C143011A76988344C2635E69D06F2D38C"sv);
}

TEST(DataforgeTest, whirlpool)
{
    whirlpool_test();
}

void blake_test()
{
    DATAFORGE_TEST(int8 | blake224 | base16l, ""sv, "7dc5313b1c04512a174bd6503b89607aecbee0903d40a8a569c94eed"sv);
    DATAFORGE_TEST(int8 | blake256 | base16l, ""sv, "716f6e863f744b9ac22c97ec7b76ea5f5908bc5b2f67c61510bfc4751384ea7a"sv);
    DATAFORGE_TEST(int8 | blake384 | base16l, ""sv, "c6cbd89c926ab525c242e6621f2f5fa73aa4afe3d9e24aed727faaadd6af38b620bdb623dd2b4788b1c8086984af8706"sv);
    DATAFORGE_TEST(int8 | blake512 | base16l, ""sv, "a8cfbbd73726062df0c6864dda65defe58ef0cc52a5625090fa17601e1eecd1b628e94f396ae402a00acc9eab77b4d4c2e852aaaa25a636d80af3fc7913ef5b8"sv);

    DATAFORGE_TEST(int8 | blake224 | base16u, "The quick brown fox jumps over the lazy dog"sv, "C8E92D7088EF87C1530AEE2AD44DC720CC10589CC2EC58F95A15E51B"sv);
    DATAFORGE_TEST(int8 | blake512 | base16l, "The quick brown fox jumps over the lazy dog"sv, "1f7e26f63b6ad25a0896fd978fd050a1766391d2fd0471a77afb975e5034b7ad2d9ccf8dfb47abbbe656e1b82fbc634ba42ce186e8dc5e1ce09a885d41f43451"sv);
    DATAFORGE_TEST(int8 | blake512 | base16l, "The quick brown fox jumps over the lazy dof"sv, "a701c2a1f9baabd8b1db6b75aee096900276f0b86dc15d247ecc03937b370324a16a4ffc0c3a85cd63229cfa15c15f4ba6d46ae2e849ed6335e9ff43b764198a"sv);
    
    std::vector<char> x55(55, 'x');
    DATAFORGE_TEST(int8 | blake224 | base16u, x55, "7060733444B73E8E9ABD6CCAE7E58C8136DC476236227AA129CA59EB"sv);
    DATAFORGE_TEST(int8 | blake256 | base16u, x55, "77F6984345B8F65903ACFC64D149CCBE345FF6ADAB371D004905FD0A81AC59FA"sv);

    std::vector<char> x111(111, 'x');
    DATAFORGE_TEST(int8 | blake384 | base16u, x111, "40E3F8D61B698983DD9ED66481D1AB556E8862FB8E351593AE205390AB2A48BC81272D975B09636264DBCC40B723A097"sv);
    DATAFORGE_TEST(int8 | blake512 | base16u, x111, "D3186B3F2954AAA5103713E3B4FA14C83E64F31B5E213B5B3B780067FE17CC7620BF81C0CDC627BFEF0B62C90BAB8708ADCB7C15C81411FDAE88CFB2860D85B6"sv);
    
    std::vector<char> x56(56, 'x');
    DATAFORGE_TEST(int8 | blake224 | base16u, x56, "7761269BB5FB5F911220134AF7336EB6182E3284D90D41970240BBE0"sv);
    DATAFORGE_TEST(int8 | blake256 | base16u, x56, "8B16603C908A4C97164615B28FE15D000BBFD2808B232AEFA5D6095445794C65"sv);

    std::vector<char> x112(112, 'x');
    DATAFORGE_TEST(int8 | blake384 | base16u, x112, "E03C75532FF1207CACBAE023581D531BF7AD85FB225295C6835A5BA41A106F0FE5579EBE65A74913B7DA3CB4791D5E5B"sv);
    DATAFORGE_TEST(int8 | blake512 | base16u, x112, "B54F89C17B84D2961AEA54C68066084C7E301215B3BE6C87B2291BAE28C3F4A099A1BBE4F29D1F9FC739A158C8091AD3AB2D8250F37F3F7BA9282017BFC87ADB"sv);
    
    std::vector<char> x64(64, 'x');
    DATAFORGE_TEST(int8 | blake224 | base16u, x64, "A745F448C389B1E969A3421A6ADF6403EFBA198DC9F104E9EB1C98FD"sv);
    DATAFORGE_TEST(int8 | blake256 | base16u, x64, "1F63FD98806699D22C2F9A3619B60C5DF0207FCA212DC49B692BFDFC3A08466C"sv);

    std::vector<char> x65(65, 'x');
    DATAFORGE_TEST(int8 | blake224 | base16u, x65, "E50E497176D313EDE684B61B39A63AAE7422B89D846F09701C5A2185"sv);
    DATAFORGE_TEST(int8 | blake256 | base16u, x65, "2650B7A48479338F2A503974BAEC72EF3DDAA88C5FDD775FA2463DC196787384"sv);

    std::vector<char> x128(128, 'x');
    DATAFORGE_TEST(int8 | blake384 | base16u, x128, "4FC5278AB9EE95E509EF0AA2BA90890FBB4020183C24586E857F72A3C4E10574354084DA38B782A7A84BB7C8E10B0156"sv);
    DATAFORGE_TEST(int8 | blake512 | base16u, x128, "5DCBD8D43329FFFF0C29330C3CFC177CC7B88C34F391255016637FDA6A41EBB390A819021B130FE131C45013ACD920154A9323D683A5E5C026CEE67B18A98A54"sv);

    std::vector<char> x129(129, 'x');
    DATAFORGE_TEST(int8 | blake384 | base16u, x129, "351B7C198C7BD0366DA2A29588B23E194B582649CCFC539407B7E99E2A2DFFECFA813FD86BB5EB5FEF2995B189174EF5"sv);
    DATAFORGE_TEST(int8 | blake512 | base16u, x129, "DD8390ACAAB891E2D04C82411063186A533FDA6254F787E7E890F851DFC0BC04ACA0EA3FA31145141BEC44184E334FE233CEC9E6893BD60FFBCD5515C8054F31"sv);

    // Blake2 //
    DATAFORGE_TEST(int8 | blake2s224 | base16l, ""sv, "1fa1291e65248b37b3433475b2a0dd63d54a11ecc4e3e034e7bc1ef4"sv);
    DATAFORGE_TEST(int8 | blake2s256 | base16l, ""sv, "69217a3079908094e11121d042354a7c1f55b6482ca1a51e1b250dfd1ed0eef9"sv);
    DATAFORGE_TEST(int8 | blake2b384 | base16l, ""sv, "b32811423377f52d7862286ee1a72ee540524380fda1724a6f25d7978c6fd3244a6caf0498812673c5e05ef583825100"sv);
    DATAFORGE_TEST(int8 | blake2b512 | base16l, ""sv, "786a02f742015903c6c6fd852552d272912f4740e15847618a86e217f71f5419d25e1031afee585313896444934eb04b903a685b1448b755d56f701afe9be2ce"sv);
    DATAFORGE_TEST(int8 | blake2b512 | base16l, "The quick brown fox jumps over the lazy dog"sv, "a8add4bdddfd93e4877d2746e62817b116364a1fa7bc148d95090bc7333b3673f82401cf7aa2e4cb1ecd90296e3f14cb5413f8ed77be73045b13914cdcd6a918"sv);
    DATAFORGE_TEST(int8 | blake2b512 | base16l, "The quick brown fox jumps over the lazy dof"sv, "ab6b007747d8068c02e25a6008db8a77c218d94f3b40d2291a7dc8a62090a744c082ea27af01521a102e42f480a31e9844053f456b4b41e8aa78bbe5c12957bb"sv);
    
    DATAFORGE_TEST(int8 | blake2s224 | base16l, x64, "3b2f687329269c564d4388c95081c6a86e720f4cdded6e7ad1693983"sv);
    DATAFORGE_TEST(int8 | blake2s256 | base16l, x64, "c13eb20b85b1d6a72d52af717429fc54eacc63fecde1295b26f0fa251bdcf40e"sv);
    DATAFORGE_TEST(int8 | blake2s224 | base16l, x65, "cad3030afcf9277abb9557e9739ff46374f48396ed7ad33eb4da7dc6"sv);
    DATAFORGE_TEST(int8 | blake2s256 | base16l, x65, "77f538c970521d7f5b94c5e8c88b8ce35661dfc6d61b5bc837da31e543cc6b81"sv);

    DATAFORGE_TEST(int8 | blake2b384 | base16l, x128, "8eef82704ab5e46be2c0ebec5e5a269155e63da7c0fdd78b5d2175134a4a7e455c2dace1c502d86c68f28bb42d96dfab"sv);
    DATAFORGE_TEST(int8 | blake2b512 | base16l, x128, "082b91ea2e15d1556d2ceefdd5af5d64d31b4e01aff1959724578876293825b236ee8079173a0a38160d7d6685d6bca0bfb62c177b3599b8727d9173e2115b91"sv);

    DATAFORGE_TEST(int8 | blake2b384 | base16l, x129, "b2e78430e3ebda23286f8d3a15456060f87d5b43557ae9e895225980c94a76c366cb2b73cf702e31f612480cbd67a9da"sv);
    DATAFORGE_TEST(int8 | blake2b512 | base16l, x129, "362a53bbe2ec08097b2f358a41d0e153aeed4c132af928400872413650e7bf22f9ae428ff73770170bbd95f935e5dd1953c17de8c7264c72d1f99303bf22dfaa"sv);

    std::vector<char> x63(63, 'x');
    std::vector<char> x1(1, 'x');
    DATAFORGE_TEST(int8 | blake2s224 | base16l, (std::vector{ x63, x1 }), "3b2f687329269c564d4388c95081c6a86e720f4cdded6e7ad1693983"sv);
    DATAFORGE_TEST(int8 | blake2s224 | base16l, (std::vector{ x63, x1, x1 }), "cad3030afcf9277abb9557e9739ff46374f48396ed7ad33eb4da7dc6"sv);
    DATAFORGE_TEST(int8 | blake2s224 | base16l, (std::vector{ x63, x65 }), "51e00eff110c3531414e54cd204aa63af7bad3509b41401242d4d02a"sv);

    DATAFORGE_TEST(int8 | blake2s224_t("1"_bs, nullptr) | base16l, x64, "bbd56c7c818bd42519f86a09de65f82435bba62c7afeb83c48d48cb4"sv);
}

TEST(DataforgeTest, blake)
{
    blake_test();
}

}
