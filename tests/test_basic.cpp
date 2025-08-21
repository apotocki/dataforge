/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "test_common.hpp"

#include <list>
#include <string>

#include "dataforge/basic/group.hpp"
#include "dataforge/basic/buffer.hpp"
#include "dataforge/compression/deflate.hpp"
#include "dataforge/compression/bzip2.hpp"
#include "dataforge/compression/lzma.hpp"
#include "dataforge/compression/lz4.hpp"
#include "dataforge/base_xx/base16.hpp"
#include "dataforge/base_xx/base64.hpp"

namespace dataforge {

void group_test()
{
    DATAFORGE_TEST(int8 | grouped_chars(2, "0x"_sp, ""_sp, ", "_sp, 4, ",\n "_sp), "0123456789AABBCCDDEEFF"_bs, "0x01, 0x23, 0x45, 0x67,\n 0x89, 0xAA, 0xBB, 0xCC,\n 0xDD, 0xEE, 0xFF"_bs);
}

void deflate_test()
{
    auto sp0 = "test string"_sp;
    auto sp1 = "one more test string"_sp;
    auto sp = "test stringone more test string"_sp;
    std::vector<char> result;
    auto cvt_it = quark_push_iterator{
        int8 | buffer<uint8_t>(256) / int8 | deflated(false, 256, 8) | inflated(false)
        , std::back_inserter(result)};
    *cvt_it = sp0;
    *cvt_it = sp1;
    cvt_it.finish();

    EXPECT_TRUE(equal_to(result, sp)) << "ERROR in deflate_test";
    
    std::vector<std::span<const char>> spans{ sp0, sp1 };
    DATAFORGE_PULL_TEST(int8 | deflated(false, 256, 8) | int8, spans, sp);
}

void bzip2_test()
{
    auto sp0 = "test string"_sp;
    auto sp1 = "one more test string"_sp;
    auto sp = "test stringone more test string"_sp;

    std::vector<char> result;
    auto cvt_it = quark_push_iterator{
        int8 | bzip2(256) | int8,
        std::back_inserter(result)
    };
    *cvt_it = sp0;
    *cvt_it = sp1;
    cvt_it.finish();

    EXPECT_TRUE(equal_to(result, sp)) << "ERROR in bzip2_test";

    // also testing all pull data provider types
    
    std::vector<std::span<const char>> spans{ sp0, sp1 };
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans, sp);

    std::vector<char> spans2;
    spans2.insert(spans2.end(), sp0.begin(), sp0.end());
    spans2.insert(spans2.end(), sp1.begin(), sp1.end());
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans2, sp);

    std::list<std::span<const char>> spans3;
    spans3.push_back(sp0);
    spans3.push_back(sp1);
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans3, sp);

    std::list<char> spans4;
    spans4.insert(spans4.end(), sp0.begin(), sp0.end());
    spans4.insert(spans4.end(), sp1.begin(), sp1.end());
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans4, sp);

    std::list<std::string> spans5;
    spans5.push_back(std::string(sp0.begin(), sp0.end()));
    spans5.push_back(std::string(sp1.begin(), sp1.end()));
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans5, sp);

    std::string spans6;
    spans6.insert(spans6.end(), sp0.begin(), sp0.end());
    spans6.insert(spans6.end(), sp1.begin(), sp1.end());
    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, spans6, sp);

    DATAFORGE_PULL_TEST(int8 | bzip2(256) | int8, sp, sp);
}

void lzma_test()
{
    auto sp0 = "test string"_sp;
    auto sp1 = "one more test string"_sp;
    auto sp = "test stringone more test string"_sp;

    std::vector<char> result;
    auto cvt_it = quark_push_iterator{
        int8 | lzma2f("lzma2:preset=9e") | int8,
        std::back_inserter(result)
    };
    *cvt_it = sp0;
    *cvt_it = sp1;
    cvt_it.finish();
    EXPECT_TRUE(equal_to(result, sp)) << "ERROR in lzma2_test";

    std::vector<std::span<const char>> spans{ sp0, sp1 };
    DATAFORGE_PULL_TEST(int8 | lzma2f("lzma2:preset=8e") | int8, spans, sp);

    result.clear();
    auto cvt_it2 = quark_push_iterator{
        int8 | lzma1() | int8,
        std::back_inserter(result)
    };
    *cvt_it2 = sp0;
    *cvt_it2 = sp1;
    cvt_it2.finish();
    EXPECT_TRUE(equal_to(result, sp)) << "ERROR in lzma1_test";
}

void lz4_test()
{
    auto sp0 = "test string"_sp;
    auto sp1 = "one more test string"_sp;
    auto sp = "test stringone more test string"_sp;

    std::vector<char> result;
    auto cvt_it = quark_push_iterator{
        int8 | lz4() | int8,
        std::back_inserter(result)
    };
    *cvt_it = sp0;
    *cvt_it = sp1;
    cvt_it.finish();
    EXPECT_TRUE(equal_to(result, sp)) << "ERROR in lzma2_test";
    std::vector<std::span<const char>> spans{ sp0, sp1 };
    DATAFORGE_PULL_TEST(int8 | lz4() | int8, spans, sp);
}

}
