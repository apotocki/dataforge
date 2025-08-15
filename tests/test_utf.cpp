/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include "test_common.hpp"
#include "dataforge/unicode/utf.hpp"
#include "dataforge/unicode/icu.hpp"
#include "dataforge/base_xx/base16.hpp"
#include "dataforge/base_xx/base64.hpp"
#include "dataforge/basic/mapper.hpp"

namespace dataforge {

const char8_t utf8text[] = {
    //0xef, 0xbb, 0xbf,
    0x65, 0x6e, 0x67, 0x6c, 0x69, 0x73, 0x68, 0x3a, 0x20, 0x61, 0x62, 0x63, 0x64,
    0x65, 0x66, 0x67, 0x68, 0x2c, 0x20, 0xd1, 0x80, 0xd1, 0x83, 0xd1, 0x81, 0xd1, 0x81, 0xd0, 0xba,
    0xd0, 0xb8, 0xd0, 0xb9, 0x3a, 0x20, 0xd0, 0xb0, 0xd0, 0xb1, 0xd0, 0xb2, 0xd0, 0xb3, 0xd0, 0xb4,
    0xd0, 0xb5, 0xd1, 0x91, 0xd0, 0xb6, 0xd0, 0xb7, 0xd0, 0xb8, 0xd0, 0x99, 0xd0, 0x9a, 0xd0, 0x9b,
    0xd0, 0x9c, 0xd0, 0x9d, 0xd0, 0x9e, 0xd0, 0x9f, 0xd0, 0xa0, 0xd0, 0xa1, 0xd0, 0xa2, 0xd0, 0xa3,
    0xd0, 0xa4, 0xd0, 0xa5, 0xd0, 0xa6, 0xd0, 0xa7, 0xd0, 0xa8, 0xd0, 0xa9, 0xd0, 0xaa, 0xd0, 0xab,
    0xd1, 0x8c, 0xd0, 0xad, 0xd0, 0xae, 0xd0, 0xaf,
    0xe6, 0x8c, 0x99,			//U+6319
    0xee, 0x8f, 0x99,			//U+e3d9
    0xf4, 0x8f, 0x95, 0x95,		//U+10f555
    0xf0, 0xA0, 0x80, 0xA7,     //U+20027
    0xf0, 0xA0, 0xA4, 0xA9      //U+20929
};

const char32_t utf32text[] = {
    //0xfeff,
    0x065, 0x06e, 0x067, 0x06c, 0x069, 0x073, 0x068, 0x03a,
    0x020, 0x061, 0x062, 0x063, 0x064, 0x065, 0x066, 0x067,
    0x068, 0x02c, 0x020, 0x440, 0x443, 0x441, 0x441, 0x43a,
    0x438, 0x439, 0x03a, 0x020, 0x430, 0x431, 0x432, 0x433,
    0x434, 0x435, 0x451, 0x436, 0x437, 0x438, 0x419, 0x41a,
    0x41b, 0x41c, 0x41d, 0x41e, 0x41f, 0x420, 0x421, 0x422,
    0x423, 0x424, 0x425, 0x426, 0x427, 0x428, 0x429, 0x42a,
    0x42b, 0x44c, 0x42d, 0x42e, 0x42f,
    0x6319, //U+6319
    0xe3d9, //U+e3d9
    0x10f555,
    0x20027,
    0x20929
};

const char16_t utf16text[] = {
    //0xfeff,
    0x065, 0x06e, 0x067, 0x06c, 0x069, 0x073, 0x068, 0x03a,
    0x020, 0x061, 0x062, 0x063, 0x064, 0x065, 0x066, 0x067,
    0x068, 0x02c, 0x020, 0x440, 0x443, 0x441, 0x441, 0x43a,
    0x438, 0x439, 0x03a, 0x020, 0x430, 0x431, 0x432, 0x433,
    0x434, 0x435, 0x451, 0x436, 0x437, 0x438, 0x419, 0x41a,
    0x41b, 0x41c, 0x41d, 0x41e, 0x41f, 0x420, 0x421, 0x422,
    0x423, 0x424, 0x425, 0x426, 0x427, 0x428, 0x429, 0x42a,
    0x42b, 0x44c, 0x42d, 0x42e, 0x42f,
    0x6319, //U+6319
    0xe3d9, //U+e3d9
    0xdbfd, 0xdd55, //U+10f555
    0xd840, 0xdc27, //U+20027
    0xd842, 0xdd29  //U+20929
};

const unsigned char utf16le[] = {
    //0xfe, 0xff,
    0x65, 0x0, 0x6e, 0x0, 0x67, 0x0, 0x6c, 0x0, 0x69, 0x0, 0x73, 0x0, 0x68, 0x0,
    0x3a, 0x0, 0x20, 0x0, 0x61, 0x0, 0x62, 0x0, 0x63, 0x0, 0x64, 0x0, 0x65, 0x0, 0x66,
    0x0, 0x67, 0x0, 0x68, 0x0, 0x2c, 0x0, 0x20, 0x0, 0x40, 0x4, 0x43, 0x4, 0x41, 0x4,
    0x41, 0x4, 0x3a, 0x4, 0x38, 0x4, 0x39, 0x4, 0x3a, 0x0, 0x20, 0x0, 0x30, 0x4, 0x31,
    0x4, 0x32, 0x4, 0x33, 0x4, 0x34, 0x4, 0x35, 0x4, 0x51, 0x4, 0x36, 0x4, 0x37, 0x4,
    0x38, 0x4, 0x19, 0x4, 0x1a, 0x4, 0x1b, 0x4, 0x1c, 0x4, 0x1d, 0x4, 0x1e, 0x4, 0x1f,
    0x4, 0x20, 0x4, 0x21, 0x4, 0x22, 0x4, 0x23, 0x4, 0x24, 0x4, 0x25, 0x4, 0x26, 0x4,
    0x27, 0x4, 0x28, 0x4, 0x29, 0x4, 0x2a, 0x4, 0x2b, 0x4, 0x4c, 0x4, 0x2d, 0x4, 0x2e, 0x4, 0x2f, 0x4,
    0x19, 0x63, //U+6319
    0xd9, 0xe3, //U+e3d9
    0xfd, 0xdb, 0x55, 0xdd, //U+10f555
    0x40, 0xd8, 0x27, 0xdc, //U+20027
    0x42, 0xd8, 0x29, 0xdd  //U+20929
};

const unsigned char utf32le[] = {
    //0xff, 0xfe, 0x00, 0x0,
    0x65, 0x00, 0x00, 0x0, 0x6e, 0x0, 0x0, 0x0, 0x67, 0x0, 0x0, 0x0, 0x6c, 0x0, 0x0, 0x0,
    0x69, 0x00, 0x00, 0x0, 0x73, 0x0, 0x0, 0x0, 0x68, 0x0, 0x0, 0x0, 0x3a, 0x0, 0x0, 0x0,
    0x20, 0x00, 0x00, 0x0, 0x61, 0x0, 0x0, 0x0, 0x62, 0x0, 0x0, 0x0, 0x63, 0x0, 0x0, 0x0,
    0x64, 0x00, 0x00, 0x0, 0x65, 0x0, 0x0, 0x0, 0x66, 0x0, 0x0, 0x0, 0x67, 0x0, 0x0, 0x0,
    0x68, 0x00, 0x00, 0x0, 0x2c, 0x0, 0x0, 0x0, 0x20, 0x0, 0x0, 0x0, 0x40, 0x4, 0x0, 0x0,
    0x43, 0x04, 0x00, 0x0, 0x41, 0x4, 0x0, 0x0, 0x41, 0x4, 0x0, 0x0, 0x3a, 0x4, 0x0, 0x0,
    0x38, 0x04, 0x00, 0x0, 0x39, 0x4, 0x0, 0x0, 0x3a, 0x0, 0x0, 0x0, 0x20, 0x0, 0x0, 0x0,
    0x30, 0x04, 0x00, 0x0, 0x31, 0x4, 0x0, 0x0, 0x32, 0x4, 0x0, 0x0, 0x33, 0x4, 0x0, 0x0,
    0x34, 0x04, 0x00, 0x0, 0x35, 0x4, 0x0, 0x0, 0x51, 0x4, 0x0, 0x0, 0x36, 0x4, 0x0, 0x0,
    0x37, 0x04, 0x00, 0x0, 0x38, 0x4, 0x0, 0x0, 0x19, 0x4, 0x0, 0x0, 0x1a, 0x4, 0x0, 0x0,
    0x1b, 0x04, 0x00, 0x0, 0x1c, 0x4, 0x0, 0x0, 0x1d, 0x4, 0x0, 0x0, 0x1e, 0x4, 0x0, 0x0,
    0x1f, 0x04, 0x00, 0x0, 0x20, 0x4, 0x0, 0x0, 0x21, 0x4, 0x0, 0x0, 0x22, 0x4, 0x0, 0x0,
    0x23, 0x04, 0x00, 0x0, 0x24, 0x4, 0x0, 0x0, 0x25, 0x4, 0x0, 0x0, 0x26, 0x4, 0x0, 0x0,
    0x27, 0x04, 0x00, 0x0, 0x28, 0x4, 0x0, 0x0, 0x29, 0x4, 0x0, 0x0, 0x2a, 0x4, 0x0, 0x0,
    0x2b, 0x04, 0x00, 0x0, 0x4c, 0x4, 0x0, 0x0, 0x2d, 0x4, 0x0, 0x0, 0x2e, 0x4, 0x0, 0x0,
    0x2f, 0x04, 0x00, 0x0,
    0x19, 0x63, 0x00, 0x0,
    0xd9, 0xe3, 0x00, 0x0,
    0x55, 0xf5, 0x10, 0x0,
    0x27, 0x00, 0x02, 0x0,
    0x29, 0x09, 0x02, 0x0
};

const std::string utf7text = "english: abcdefgh, +BEAEQwRBBEEEOgQ4BDk-: +BDAEMQQyBDMENAQ1BFEENgQ3BDgEGQQaBBsEH"
    "AQdBB4EHwQgBCEEIgQjBCQEJQQmBCcEKAQpBCoEKwRMBC0ELgQvYxnj2dv93VXYQNwn2ELdKQ-";

void utf_test()
{
    DATAFORGE_TEST(utf8 | utf32, std::span{ utf8text }, std::span{ utf32text });
    

    DATAFORGE_TEST(utf8 | utf32 | utf16, std::span{ utf8text }, std::span{ utf16text });
    DATAFORGE_TEST(utf16 | utf32 | utf8, std::span{ utf16text }, std::span{ utf8text });
    DATAFORGE_TEST(utf8 | utf16, std::span{ utf8text }, std::span{ utf16text });
    DATAFORGE_TEST(utf16 | utf8, std::span{ utf16text }, std::span{ utf8text });

    DATAFORGE_TEST(utf16 | le, std::span{ utf16text }, std::span{ utf16le });
    DATAFORGE_TEST(le | utf16, std::span{ utf16le }, std::span{ utf16text });

    // preparing utf16be, utf32be
    std::vector<char> utf16be;
    for (size_t i = 0; i < sizeof(utf16le); i += 2)
    {
        utf16be.push_back(utf16le[i + 1]);
        utf16be.push_back(utf16le[i]);
    }
    std::vector<char> utf32be;
    for (size_t i = 0; i < sizeof(utf32le); i += 4)
    {
        utf32be.push_back(utf32le[i + 3]);
        utf32be.push_back(utf32le[i + 2]);
        utf32be.push_back(utf32le[i + 1]);
        utf32be.push_back(utf32le[i]);
    }

    DATAFORGE_TEST(utf16 | be, std::span{ utf16text }, utf16be);
    DATAFORGE_TEST(be | utf16, utf16be, std::span{ utf16text });
    DATAFORGE_TEST(utf16 | utf32 | le, std::span{ utf16text }, std::span{ utf32le });
    DATAFORGE_TEST(le | utf32 | utf16, std::span{ utf32le }, std::span{ utf16text });
    DATAFORGE_TEST(utf16 | utf32 | be, std::span{ utf16text }, utf32be);
    DATAFORGE_TEST(be | utf32 | utf16, utf32be, std::span{ utf16text });

    DATAFORGE_TEST(utf16 | utf7, std::span{ utf16text }, utf7text);
    DATAFORGE_TEST(utf7 | utf16, utf7text, std::span{ utf16text });

    DATAFORGE_TEST(be | utf16 | utf32 | le/int8 | base64 | int8/le | utf32 | utf16, utf16be, std::span{ utf16text });
}

TEST(DataforgeTest, utf)
{
    utf_test();
}

struct grapheme_accum
{
    std::vector<std::vector<unsigned char>> graphemes;

    inline grapheme_accum& operator*() { return *this; }
    inline void operator++() {}

    template <typename CharT>
    grapheme_accum& operator=(std::pair<size_t, CharT> fragment)
    {
        this->operator=(std::pair{ fragment.first, std::span{&fragment.second, 1} });
        return *this;
    }

    template <typename CharT, size_t E>
    grapheme_accum& operator=(std::pair<size_t, std::span<const CharT, E>> fragment)
    {
        if (graphemes.size() <= fragment.first) {
            graphemes.resize(fragment.first + 1);
        }
        auto& graphem = graphemes[fragment.first];
        graphem.insert(graphem.end(), fragment.second.begin(), fragment.second.end());
        return *this;
    }
};

void check_breaks(cbyte_span_t in, std::vector<cbyte_span_t> grahemes)
{
    grapheme_accum result;
    auto it = quark_push_iterator(be | utf16 | utf32 | enumerated_graphemes | seq_mapper(utf32 | utf16 | be), std::ref(result));
    *it = in;
    it.finish();

    if (result.graphemes.size() != grahemes.size()) {
        EXPECT_TRUE(result.graphemes.size() != grahemes.size()) << "found wrong graheme count";
    }
    for (size_t i = 0; i < result.graphemes.size(); ++i) {
        bool test_result = equal_to(result.graphemes[i], grahemes[i]);
        if (!test_result) {
            EXPECT_TRUE(test_result) << "ERROR: ranges are different";
        }
    }
}

void grapheme_break_test()
{
    check_breaks("\x00\x20\x00\x20"_bs, { "\x00\x20"_bs, "\x00\x20"_bs }); // ÷ 0020 ÷ 0020 ÷	
    check_breaks("\x00\x20\x03\x08\x00\x20"_bs, { "\x00\x20\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0020 × 0308 ÷ 0020 ÷	
    check_breaks("\x00\x20\x00\x0D"_bs, { "\x00\x20"_bs, "\x00\x0D"_bs }); // ÷ 0020 ÷ 000D ÷	
    check_breaks("\x00\x20\x03\x08\x00\x0D"_bs, { "\x00\x20\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0020 × 0308 ÷ 000D ÷	
    check_breaks("\x00\x20\x00\x0A"_bs, { "\x00\x20"_bs, "\x00\x0A"_bs }); // ÷ 0020 ÷ 000A ÷	
    check_breaks("\x00\x20\x03\x08\x00\x0A"_bs, { "\x00\x20\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0020 × 0308 ÷ 000A ÷	
    check_breaks("\x00\x20\x00\x01"_bs, { "\x00\x20"_bs, "\x00\x01"_bs }); // ÷ 0020 ÷ 0001 ÷	
    check_breaks("\x00\x20\x03\x08\x00\x01"_bs, { "\x00\x20\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0020 × 0308 ÷ 0001 ÷	
    check_breaks("\x00\x20\x03\x4F"_bs, { "\x00\x20\x03\x4F"_bs }); // ÷ 0020 × 034F ÷	
    check_breaks("\x00\x20\x03\x08\x03\x4F"_bs, { "\x00\x20\x03\x08\x03\x4F"_bs }); // ÷ 0020 × 0308 × 034F ÷	
    check_breaks("\x00\x20\xD8\x3C\xDD\xE6"_bs, { "\x00\x20"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0020 ÷ 1F1E6 ÷	
    check_breaks("\x00\x20\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x00\x20\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0020 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x00\x20\x06\x00"_bs, { "\x00\x20"_bs, "\x06\x00"_bs }); // ÷ 0020 ÷ 0600 ÷	
    check_breaks("\x00\x20\x03\x08\x06\x00"_bs, { "\x00\x20\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0020 × 0308 ÷ 0600 ÷	
    check_breaks("\x00\x20\x09\x03"_bs, { "\x00\x20\x09\x03"_bs }); // ÷ 0020 × 0903 ÷	
    check_breaks("\x00\x20\x03\x08\x09\x03"_bs, { "\x00\x20\x03\x08\x09\x03"_bs }); // ÷ 0020 × 0308 × 0903 ÷	
    check_breaks("\x00\x20\x11\x00"_bs, { "\x00\x20"_bs, "\x11\x00"_bs }); // ÷ 0020 ÷ 1100 ÷	
    check_breaks("\x00\x20\x03\x08\x11\x00"_bs, { "\x00\x20\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0020 × 0308 ÷ 1100 ÷	
    check_breaks("\x00\x20\x11\x60"_bs, { "\x00\x20"_bs, "\x11\x60"_bs }); // ÷ 0020 ÷ 1160 ÷	
    check_breaks("\x00\x20\x03\x08\x11\x60"_bs, { "\x00\x20\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0020 × 0308 ÷ 1160 ÷	
    check_breaks("\x00\x20\x11\xA8"_bs, { "\x00\x20"_bs, "\x11\xA8"_bs }); // ÷ 0020 ÷ 11A8 ÷	
    check_breaks("\x00\x20\x03\x08\x11\xA8"_bs, { "\x00\x20\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0020 × 0308 ÷ 11A8 ÷	
    check_breaks("\x00\x20\xAC\x00"_bs, { "\x00\x20"_bs, "\xAC\x00"_bs }); // ÷ 0020 ÷ AC00 ÷	
    check_breaks("\x00\x20\x03\x08\xAC\x00"_bs, { "\x00\x20\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0020 × 0308 ÷ AC00 ÷	
    check_breaks("\x00\x20\xAC\x01"_bs, { "\x00\x20"_bs, "\xAC\x01"_bs }); // ÷ 0020 ÷ AC01 ÷	
    check_breaks("\x00\x20\x03\x08\xAC\x01"_bs, { "\x00\x20\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0020 × 0308 ÷ AC01 ÷	
    check_breaks("\x00\x20\x23\x1A"_bs, { "\x00\x20"_bs, "\x23\x1A"_bs }); // ÷ 0020 ÷ 231A ÷	
    check_breaks("\x00\x20\x03\x08\x23\x1A"_bs, { "\x00\x20\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0020 × 0308 ÷ 231A ÷	
    check_breaks("\x00\x20\x03\x00"_bs, { "\x00\x20\x03\x00"_bs }); // ÷ 0020 × 0300 ÷	
    check_breaks("\x00\x20\x03\x08\x03\x00"_bs, { "\x00\x20\x03\x08\x03\x00"_bs }); // ÷ 0020 × 0308 × 0300 ÷	
    check_breaks("\x00\x20\x20\x0D"_bs, { "\x00\x20\x20\x0D"_bs }); // ÷ 0020 × 200D ÷	
    check_breaks("\x00\x20\x03\x08\x20\x0D"_bs, { "\x00\x20\x03\x08\x20\x0D"_bs }); // ÷ 0020 × 0308 × 200D ÷	
    check_breaks("\x00\x20\x03\x78"_bs, { "\x00\x20"_bs, "\x03\x78"_bs }); // ÷ 0020 ÷ 0378 ÷	
    check_breaks("\x00\x20\x03\x08\x03\x78"_bs, { "\x00\x20\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0020 × 0308 ÷ 0378 ÷	
    check_breaks("\x00\x0D\x00\x20"_bs, { "\x00\x0D"_bs, "\x00\x20"_bs }); // ÷ 000D ÷ 0020 ÷	
    check_breaks("\x00\x0D\x03\x08\x00\x20"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 000D ÷ 0308 ÷ 0020 ÷	
    check_breaks("\x00\x0D\x00\x0D"_bs, { "\x00\x0D"_bs, "\x00\x0D"_bs }); // ÷ 000D ÷ 000D ÷	
    check_breaks("\x00\x0D\x03\x08\x00\x0D"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 000D ÷ 0308 ÷ 000D ÷	
    check_breaks("\x00\x0D\x00\x0A"_bs, { "\x00\x0D\x00\x0A"_bs }); // ÷ 000D × 000A ÷	
    check_breaks("\x00\x0D\x03\x08\x00\x0A"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 000D ÷ 0308 ÷ 000A ÷	
    check_breaks("\x00\x0D\x00\x01"_bs, { "\x00\x0D"_bs, "\x00\x01"_bs }); // ÷ 000D ÷ 0001 ÷	
    check_breaks("\x00\x0D\x03\x08\x00\x01"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 000D ÷ 0308 ÷ 0001 ÷	
    check_breaks("\x00\x0D\x03\x4F"_bs, { "\x00\x0D"_bs, "\x03\x4F"_bs }); // ÷ 000D ÷ 034F ÷	
    check_breaks("\x00\x0D\x03\x08\x03\x4F"_bs, { "\x00\x0D"_bs, "\x03\x08\x03\x4F"_bs }); // ÷ 000D ÷ 0308 × 034F ÷	
    check_breaks("\x00\x0D\xD8\x3C\xDD\xE6"_bs, { "\x00\x0D"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 000D ÷ 1F1E6 ÷	
    check_breaks("\x00\x0D\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 000D ÷ 0308 ÷ 1F1E6 ÷	
    check_breaks("\x00\x0D\x06\x00"_bs, { "\x00\x0D"_bs, "\x06\x00"_bs }); // ÷ 000D ÷ 0600 ÷	
    check_breaks("\x00\x0D\x03\x08\x06\x00"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 000D ÷ 0308 ÷ 0600 ÷	
    check_breaks("\x00\x0D\x09\x03"_bs, { "\x00\x0D"_bs, "\x09\x03"_bs }); // ÷ 000D ÷ 0903 ÷	
    check_breaks("\x00\x0D\x03\x08\x09\x03"_bs, { "\x00\x0D"_bs, "\x03\x08\x09\x03"_bs }); // ÷ 000D ÷ 0308 × 0903 ÷	
    check_breaks("\x00\x0D\x11\x00"_bs, { "\x00\x0D"_bs, "\x11\x00"_bs }); // ÷ 000D ÷ 1100 ÷	
    check_breaks("\x00\x0D\x03\x08\x11\x00"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 000D ÷ 0308 ÷ 1100 ÷	
    check_breaks("\x00\x0D\x11\x60"_bs, { "\x00\x0D"_bs, "\x11\x60"_bs }); // ÷ 000D ÷ 1160 ÷	
    check_breaks("\x00\x0D\x03\x08\x11\x60"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 000D ÷ 0308 ÷ 1160 ÷	
    check_breaks("\x00\x0D\x11\xA8"_bs, { "\x00\x0D"_bs, "\x11\xA8"_bs }); // ÷ 000D ÷ 11A8 ÷	
    check_breaks("\x00\x0D\x03\x08\x11\xA8"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 000D ÷ 0308 ÷ 11A8 ÷	
    check_breaks("\x00\x0D\xAC\x00"_bs, { "\x00\x0D"_bs, "\xAC\x00"_bs }); // ÷ 000D ÷ AC00 ÷	
    check_breaks("\x00\x0D\x03\x08\xAC\x00"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 000D ÷ 0308 ÷ AC00 ÷	
    check_breaks("\x00\x0D\xAC\x01"_bs, { "\x00\x0D"_bs, "\xAC\x01"_bs }); // ÷ 000D ÷ AC01 ÷	
    check_breaks("\x00\x0D\x03\x08\xAC\x01"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 000D ÷ 0308 ÷ AC01 ÷	
    check_breaks("\x00\x0D\x23\x1A"_bs, { "\x00\x0D"_bs, "\x23\x1A"_bs }); // ÷ 000D ÷ 231A ÷	
    check_breaks("\x00\x0D\x03\x08\x23\x1A"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 000D ÷ 0308 ÷ 231A ÷	
    check_breaks("\x00\x0D\x03\x00"_bs, { "\x00\x0D"_bs, "\x03\x00"_bs }); // ÷ 000D ÷ 0300 ÷	
    check_breaks("\x00\x0D\x03\x08\x03\x00"_bs, { "\x00\x0D"_bs, "\x03\x08\x03\x00"_bs }); // ÷ 000D ÷ 0308 × 0300 ÷	
    check_breaks("\x00\x0D\x20\x0D"_bs, { "\x00\x0D"_bs, "\x20\x0D"_bs }); // ÷ 000D ÷ 200D ÷	
    check_breaks("\x00\x0D\x03\x08\x20\x0D"_bs, { "\x00\x0D"_bs, "\x03\x08\x20\x0D"_bs }); // ÷ 000D ÷ 0308 × 200D ÷	
    check_breaks("\x00\x0D\x03\x78"_bs, { "\x00\x0D"_bs, "\x03\x78"_bs }); // ÷ 000D ÷ 0378 ÷	
    check_breaks("\x00\x0D\x03\x08\x03\x78"_bs, { "\x00\x0D"_bs, "\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 000D ÷ 0308 ÷ 0378 ÷	
    check_breaks("\x00\x0A\x00\x20"_bs, { "\x00\x0A"_bs, "\x00\x20"_bs }); // ÷ 000A ÷ 0020 ÷	
    check_breaks("\x00\x0A\x03\x08\x00\x20"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 000A ÷ 0308 ÷ 0020 ÷	
    check_breaks("\x00\x0A\x00\x0D"_bs, { "\x00\x0A"_bs, "\x00\x0D"_bs }); // ÷ 000A ÷ 000D ÷	
    check_breaks("\x00\x0A\x03\x08\x00\x0D"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 000A ÷ 0308 ÷ 000D ÷	
    check_breaks("\x00\x0A\x00\x0A"_bs, { "\x00\x0A"_bs, "\x00\x0A"_bs }); // ÷ 000A ÷ 000A ÷	
    check_breaks("\x00\x0A\x03\x08\x00\x0A"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 000A ÷ 0308 ÷ 000A ÷	
    check_breaks("\x00\x0A\x00\x01"_bs, { "\x00\x0A"_bs, "\x00\x01"_bs }); // ÷ 000A ÷ 0001 ÷	
    check_breaks("\x00\x0A\x03\x08\x00\x01"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 000A ÷ 0308 ÷ 0001 ÷	
    check_breaks("\x00\x0A\x03\x4F"_bs, { "\x00\x0A"_bs, "\x03\x4F"_bs }); // ÷ 000A ÷ 034F ÷	
    check_breaks("\x00\x0A\x03\x08\x03\x4F"_bs, { "\x00\x0A"_bs, "\x03\x08\x03\x4F"_bs }); // ÷ 000A ÷ 0308 × 034F ÷	
    check_breaks("\x00\x0A\xD8\x3C\xDD\xE6"_bs, { "\x00\x0A"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 000A ÷ 1F1E6 ÷	
    check_breaks("\x00\x0A\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 000A ÷ 0308 ÷ 1F1E6 ÷	
    check_breaks("\x00\x0A\x06\x00"_bs, { "\x00\x0A"_bs, "\x06\x00"_bs }); // ÷ 000A ÷ 0600 ÷	
    check_breaks("\x00\x0A\x03\x08\x06\x00"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 000A ÷ 0308 ÷ 0600 ÷	
    check_breaks("\x00\x0A\x09\x03"_bs, { "\x00\x0A"_bs, "\x09\x03"_bs }); // ÷ 000A ÷ 0903 ÷	
    check_breaks("\x00\x0A\x03\x08\x09\x03"_bs, { "\x00\x0A"_bs, "\x03\x08\x09\x03"_bs }); // ÷ 000A ÷ 0308 × 0903 ÷	
    check_breaks("\x00\x0A\x11\x00"_bs, { "\x00\x0A"_bs, "\x11\x00"_bs }); // ÷ 000A ÷ 1100 ÷	
    check_breaks("\x00\x0A\x03\x08\x11\x00"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 000A ÷ 0308 ÷ 1100 ÷	
    check_breaks("\x00\x0A\x11\x60"_bs, { "\x00\x0A"_bs, "\x11\x60"_bs }); // ÷ 000A ÷ 1160 ÷	
    check_breaks("\x00\x0A\x03\x08\x11\x60"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 000A ÷ 0308 ÷ 1160 ÷	
    check_breaks("\x00\x0A\x11\xA8"_bs, { "\x00\x0A"_bs, "\x11\xA8"_bs }); // ÷ 000A ÷ 11A8 ÷	
    check_breaks("\x00\x0A\x03\x08\x11\xA8"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 000A ÷ 0308 ÷ 11A8 ÷	
    check_breaks("\x00\x0A\xAC\x00"_bs, { "\x00\x0A"_bs, "\xAC\x00"_bs }); // ÷ 000A ÷ AC00 ÷	
    check_breaks("\x00\x0A\x03\x08\xAC\x00"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 000A ÷ 0308 ÷ AC00 ÷	
    check_breaks("\x00\x0A\xAC\x01"_bs, { "\x00\x0A"_bs, "\xAC\x01"_bs }); // ÷ 000A ÷ AC01 ÷	
    check_breaks("\x00\x0A\x03\x08\xAC\x01"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 000A ÷ 0308 ÷ AC01 ÷	
    check_breaks("\x00\x0A\x23\x1A"_bs, { "\x00\x0A"_bs, "\x23\x1A"_bs }); // ÷ 000A ÷ 231A ÷	
    check_breaks("\x00\x0A\x03\x08\x23\x1A"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 000A ÷ 0308 ÷ 231A ÷	
    check_breaks("\x00\x0A\x03\x00"_bs, { "\x00\x0A"_bs, "\x03\x00"_bs }); // ÷ 000A ÷ 0300 ÷	
    check_breaks("\x00\x0A\x03\x08\x03\x00"_bs, { "\x00\x0A"_bs, "\x03\x08\x03\x00"_bs }); // ÷ 000A ÷ 0308 × 0300 ÷	
    check_breaks("\x00\x0A\x20\x0D"_bs, { "\x00\x0A"_bs, "\x20\x0D"_bs }); // ÷ 000A ÷ 200D ÷	
    check_breaks("\x00\x0A\x03\x08\x20\x0D"_bs, { "\x00\x0A"_bs, "\x03\x08\x20\x0D"_bs }); // ÷ 000A ÷ 0308 × 200D ÷	
    check_breaks("\x00\x0A\x03\x78"_bs, { "\x00\x0A"_bs, "\x03\x78"_bs }); // ÷ 000A ÷ 0378 ÷	
    check_breaks("\x00\x0A\x03\x08\x03\x78"_bs, { "\x00\x0A"_bs, "\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 000A ÷ 0308 ÷ 0378 ÷	
    check_breaks("\x00\x01\x00\x20"_bs, { "\x00\x01"_bs, "\x00\x20"_bs }); // ÷ 0001 ÷ 0020 ÷	
    check_breaks("\x00\x01\x03\x08\x00\x20"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0001 ÷ 0308 ÷ 0020 ÷	
    check_breaks("\x00\x01\x00\x0D"_bs, { "\x00\x01"_bs, "\x00\x0D"_bs }); // ÷ 0001 ÷ 000D ÷	
    check_breaks("\x00\x01\x03\x08\x00\x0D"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0001 ÷ 0308 ÷ 000D ÷	
    check_breaks("\x00\x01\x00\x0A"_bs, { "\x00\x01"_bs, "\x00\x0A"_bs }); // ÷ 0001 ÷ 000A ÷	
    check_breaks("\x00\x01\x03\x08\x00\x0A"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0001 ÷ 0308 ÷ 000A ÷	
    check_breaks("\x00\x01\x00\x01"_bs, { "\x00\x01"_bs, "\x00\x01"_bs }); // ÷ 0001 ÷ 0001 ÷	
    check_breaks("\x00\x01\x03\x08\x00\x01"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0001 ÷ 0308 ÷ 0001 ÷	
    check_breaks("\x00\x01\x03\x4F"_bs, { "\x00\x01"_bs, "\x03\x4F"_bs }); // ÷ 0001 ÷ 034F ÷	
    check_breaks("\x00\x01\x03\x08\x03\x4F"_bs, { "\x00\x01"_bs, "\x03\x08\x03\x4F"_bs }); // ÷ 0001 ÷ 0308 × 034F ÷	
    check_breaks("\x00\x01\xD8\x3C\xDD\xE6"_bs, { "\x00\x01"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0001 ÷ 1F1E6 ÷	
    check_breaks("\x00\x01\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0001 ÷ 0308 ÷ 1F1E6 ÷	
    check_breaks("\x00\x01\x06\x00"_bs, { "\x00\x01"_bs, "\x06\x00"_bs }); // ÷ 0001 ÷ 0600 ÷	
    check_breaks("\x00\x01\x03\x08\x06\x00"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0001 ÷ 0308 ÷ 0600 ÷	
    check_breaks("\x00\x01\x09\x03"_bs, { "\x00\x01"_bs, "\x09\x03"_bs }); // ÷ 0001 ÷ 0903 ÷	
    check_breaks("\x00\x01\x03\x08\x09\x03"_bs, { "\x00\x01"_bs, "\x03\x08\x09\x03"_bs }); // ÷ 0001 ÷ 0308 × 0903 ÷	
    check_breaks("\x00\x01\x11\x00"_bs, { "\x00\x01"_bs, "\x11\x00"_bs }); // ÷ 0001 ÷ 1100 ÷	
    check_breaks("\x00\x01\x03\x08\x11\x00"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0001 ÷ 0308 ÷ 1100 ÷	
    check_breaks("\x00\x01\x11\x60"_bs, { "\x00\x01"_bs, "\x11\x60"_bs }); // ÷ 0001 ÷ 1160 ÷	
    check_breaks("\x00\x01\x03\x08\x11\x60"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0001 ÷ 0308 ÷ 1160 ÷	
    check_breaks("\x00\x01\x11\xA8"_bs, { "\x00\x01"_bs, "\x11\xA8"_bs }); // ÷ 0001 ÷ 11A8 ÷	
    check_breaks("\x00\x01\x03\x08\x11\xA8"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0001 ÷ 0308 ÷ 11A8 ÷	
    check_breaks("\x00\x01\xAC\x00"_bs, { "\x00\x01"_bs, "\xAC\x00"_bs }); // ÷ 0001 ÷ AC00 ÷	
    check_breaks("\x00\x01\x03\x08\xAC\x00"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0001 ÷ 0308 ÷ AC00 ÷	
    check_breaks("\x00\x01\xAC\x01"_bs, { "\x00\x01"_bs, "\xAC\x01"_bs }); // ÷ 0001 ÷ AC01 ÷	
    check_breaks("\x00\x01\x03\x08\xAC\x01"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0001 ÷ 0308 ÷ AC01 ÷	
    check_breaks("\x00\x01\x23\x1A"_bs, { "\x00\x01"_bs, "\x23\x1A"_bs }); // ÷ 0001 ÷ 231A ÷	
    check_breaks("\x00\x01\x03\x08\x23\x1A"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0001 ÷ 0308 ÷ 231A ÷	
    check_breaks("\x00\x01\x03\x00"_bs, { "\x00\x01"_bs, "\x03\x00"_bs }); // ÷ 0001 ÷ 0300 ÷	
    check_breaks("\x00\x01\x03\x08\x03\x00"_bs, { "\x00\x01"_bs, "\x03\x08\x03\x00"_bs }); // ÷ 0001 ÷ 0308 × 0300 ÷	
    check_breaks("\x00\x01\x20\x0D"_bs, { "\x00\x01"_bs, "\x20\x0D"_bs }); // ÷ 0001 ÷ 200D ÷	
    check_breaks("\x00\x01\x03\x08\x20\x0D"_bs, { "\x00\x01"_bs, "\x03\x08\x20\x0D"_bs }); // ÷ 0001 ÷ 0308 × 200D ÷	
    check_breaks("\x00\x01\x03\x78"_bs, { "\x00\x01"_bs, "\x03\x78"_bs }); // ÷ 0001 ÷ 0378 ÷	
    check_breaks("\x00\x01\x03\x08\x03\x78"_bs, { "\x00\x01"_bs, "\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0001 ÷ 0308 ÷ 0378 ÷	
    check_breaks("\x03\x4F\x00\x20"_bs, { "\x03\x4F"_bs, "\x00\x20"_bs }); // ÷ 034F ÷ 0020 ÷	
    check_breaks("\x03\x4F\x03\x08\x00\x20"_bs, { "\x03\x4F\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 034F × 0308 ÷ 0020 ÷	
    check_breaks("\x03\x4F\x00\x0D"_bs, { "\x03\x4F"_bs, "\x00\x0D"_bs }); // ÷ 034F ÷ 000D ÷	
    check_breaks("\x03\x4F\x03\x08\x00\x0D"_bs, { "\x03\x4F\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 034F × 0308 ÷ 000D ÷	
    check_breaks("\x03\x4F\x00\x0A"_bs, { "\x03\x4F"_bs, "\x00\x0A"_bs }); // ÷ 034F ÷ 000A ÷	
    check_breaks("\x03\x4F\x03\x08\x00\x0A"_bs, { "\x03\x4F\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 034F × 0308 ÷ 000A ÷	
    check_breaks("\x03\x4F\x00\x01"_bs, { "\x03\x4F"_bs, "\x00\x01"_bs }); // ÷ 034F ÷ 0001 ÷	
    check_breaks("\x03\x4F\x03\x08\x00\x01"_bs, { "\x03\x4F\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 034F × 0308 ÷ 0001 ÷	
    check_breaks("\x03\x4F\x03\x4F"_bs, { "\x03\x4F\x03\x4F"_bs }); // ÷ 034F × 034F ÷	
    check_breaks("\x03\x4F\x03\x08\x03\x4F"_bs, { "\x03\x4F\x03\x08\x03\x4F"_bs }); // ÷ 034F × 0308 × 034F ÷	
    check_breaks("\x03\x4F\xD8\x3C\xDD\xE6"_bs, { "\x03\x4F"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 034F ÷ 1F1E6 ÷	
    check_breaks("\x03\x4F\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x03\x4F\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 034F × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x03\x4F\x06\x00"_bs, { "\x03\x4F"_bs, "\x06\x00"_bs }); // ÷ 034F ÷ 0600 ÷	
    check_breaks("\x03\x4F\x03\x08\x06\x00"_bs, { "\x03\x4F\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 034F × 0308 ÷ 0600 ÷	
    check_breaks("\x03\x4F\x09\x03"_bs, { "\x03\x4F\x09\x03"_bs }); // ÷ 034F × 0903 ÷	
    check_breaks("\x03\x4F\x03\x08\x09\x03"_bs, { "\x03\x4F\x03\x08\x09\x03"_bs }); // ÷ 034F × 0308 × 0903 ÷	
    check_breaks("\x03\x4F\x11\x00"_bs, { "\x03\x4F"_bs, "\x11\x00"_bs }); // ÷ 034F ÷ 1100 ÷	
    check_breaks("\x03\x4F\x03\x08\x11\x00"_bs, { "\x03\x4F\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 034F × 0308 ÷ 1100 ÷	
    check_breaks("\x03\x4F\x11\x60"_bs, { "\x03\x4F"_bs, "\x11\x60"_bs }); // ÷ 034F ÷ 1160 ÷	
    check_breaks("\x03\x4F\x03\x08\x11\x60"_bs, { "\x03\x4F\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 034F × 0308 ÷ 1160 ÷	
    check_breaks("\x03\x4F\x11\xA8"_bs, { "\x03\x4F"_bs, "\x11\xA8"_bs }); // ÷ 034F ÷ 11A8 ÷	
    check_breaks("\x03\x4F\x03\x08\x11\xA8"_bs, { "\x03\x4F\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 034F × 0308 ÷ 11A8 ÷	
    check_breaks("\x03\x4F\xAC\x00"_bs, { "\x03\x4F"_bs, "\xAC\x00"_bs }); // ÷ 034F ÷ AC00 ÷	
    check_breaks("\x03\x4F\x03\x08\xAC\x00"_bs, { "\x03\x4F\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 034F × 0308 ÷ AC00 ÷	
    check_breaks("\x03\x4F\xAC\x01"_bs, { "\x03\x4F"_bs, "\xAC\x01"_bs }); // ÷ 034F ÷ AC01 ÷	
    check_breaks("\x03\x4F\x03\x08\xAC\x01"_bs, { "\x03\x4F\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 034F × 0308 ÷ AC01 ÷	
    check_breaks("\x03\x4F\x23\x1A"_bs, { "\x03\x4F"_bs, "\x23\x1A"_bs }); // ÷ 034F ÷ 231A ÷	
    check_breaks("\x03\x4F\x03\x08\x23\x1A"_bs, { "\x03\x4F\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 034F × 0308 ÷ 231A ÷	
    check_breaks("\x03\x4F\x03\x00"_bs, { "\x03\x4F\x03\x00"_bs }); // ÷ 034F × 0300 ÷	
    check_breaks("\x03\x4F\x03\x08\x03\x00"_bs, { "\x03\x4F\x03\x08\x03\x00"_bs }); // ÷ 034F × 0308 × 0300 ÷	
    check_breaks("\x03\x4F\x20\x0D"_bs, { "\x03\x4F\x20\x0D"_bs }); // ÷ 034F × 200D ÷	
    check_breaks("\x03\x4F\x03\x08\x20\x0D"_bs, { "\x03\x4F\x03\x08\x20\x0D"_bs }); // ÷ 034F × 0308 × 200D ÷	
    check_breaks("\x03\x4F\x03\x78"_bs, { "\x03\x4F"_bs, "\x03\x78"_bs }); // ÷ 034F ÷ 0378 ÷	
    check_breaks("\x03\x4F\x03\x08\x03\x78"_bs, { "\x03\x4F\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 034F × 0308 ÷ 0378 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x00\x20"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x00\x20"_bs }); // ÷ 1F1E6 ÷ 0020 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x00\x20"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 1F1E6 × 0308 ÷ 0020 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x00\x0D"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x00\x0D"_bs }); // ÷ 1F1E6 ÷ 000D ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x00\x0D"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 1F1E6 × 0308 ÷ 000D ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x00\x0A"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x00\x0A"_bs }); // ÷ 1F1E6 ÷ 000A ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x00\x0A"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 1F1E6 × 0308 ÷ 000A ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x00\x01"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x00\x01"_bs }); // ÷ 1F1E6 ÷ 0001 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x00\x01"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 1F1E6 × 0308 ÷ 0001 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x4F"_bs, { "\xD8\x3C\xDD\xE6\x03\x4F"_bs }); // ÷ 1F1E6 × 034F ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x03\x4F"_bs, { "\xD8\x3C\xDD\xE6\x03\x08\x03\x4F"_bs }); // ÷ 1F1E6 × 0308 × 034F ÷	
    check_breaks("\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE6"_bs, { "\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE6"_bs }); // ÷ 1F1E6 × 1F1E6 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 1F1E6 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x06\x00"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x06\x00"_bs }); // ÷ 1F1E6 ÷ 0600 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x06\x00"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 1F1E6 × 0308 ÷ 0600 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x09\x03"_bs, { "\xD8\x3C\xDD\xE6\x09\x03"_bs }); // ÷ 1F1E6 × 0903 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x09\x03"_bs, { "\xD8\x3C\xDD\xE6\x03\x08\x09\x03"_bs }); // ÷ 1F1E6 × 0308 × 0903 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x11\x00"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x11\x00"_bs }); // ÷ 1F1E6 ÷ 1100 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x11\x00"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 1F1E6 × 0308 ÷ 1100 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x11\x60"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x11\x60"_bs }); // ÷ 1F1E6 ÷ 1160 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x11\x60"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 1F1E6 × 0308 ÷ 1160 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x11\xA8"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x11\xA8"_bs }); // ÷ 1F1E6 ÷ 11A8 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x11\xA8"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 1F1E6 × 0308 ÷ 11A8 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\xAC\x00"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\xAC\x00"_bs }); // ÷ 1F1E6 ÷ AC00 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\xAC\x00"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 1F1E6 × 0308 ÷ AC00 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\xAC\x01"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\xAC\x01"_bs }); // ÷ 1F1E6 ÷ AC01 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\xAC\x01"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 1F1E6 × 0308 ÷ AC01 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x23\x1A"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x23\x1A"_bs }); // ÷ 1F1E6 ÷ 231A ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x23\x1A"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 1F1E6 × 0308 ÷ 231A ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x00"_bs, { "\xD8\x3C\xDD\xE6\x03\x00"_bs }); // ÷ 1F1E6 × 0300 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x03\x00"_bs, { "\xD8\x3C\xDD\xE6\x03\x08\x03\x00"_bs }); // ÷ 1F1E6 × 0308 × 0300 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x20\x0D"_bs, { "\xD8\x3C\xDD\xE6\x20\x0D"_bs }); // ÷ 1F1E6 × 200D ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x20\x0D"_bs, { "\xD8\x3C\xDD\xE6\x03\x08\x20\x0D"_bs }); // ÷ 1F1E6 × 0308 × 200D ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x78"_bs, { "\xD8\x3C\xDD\xE6"_bs, "\x03\x78"_bs }); // ÷ 1F1E6 ÷ 0378 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\x03\x08\x03\x78"_bs, { "\xD8\x3C\xDD\xE6\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 1F1E6 × 0308 ÷ 0378 ÷	
    check_breaks("\x06\x00\x00\x20"_bs, { "\x06\x00\x00\x20"_bs }); // ÷ 0600 × 0020 ÷	
    check_breaks("\x06\x00\x03\x08\x00\x20"_bs, { "\x06\x00\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0600 × 0308 ÷ 0020 ÷	
    check_breaks("\x06\x00\x00\x0D"_bs, { "\x06\x00"_bs, "\x00\x0D"_bs }); // ÷ 0600 ÷ 000D ÷	
    check_breaks("\x06\x00\x03\x08\x00\x0D"_bs, { "\x06\x00\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0600 × 0308 ÷ 000D ÷	
    check_breaks("\x06\x00\x00\x0A"_bs, { "\x06\x00"_bs, "\x00\x0A"_bs }); // ÷ 0600 ÷ 000A ÷	
    check_breaks("\x06\x00\x03\x08\x00\x0A"_bs, { "\x06\x00\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0600 × 0308 ÷ 000A ÷	
    check_breaks("\x06\x00\x00\x01"_bs, { "\x06\x00"_bs, "\x00\x01"_bs }); // ÷ 0600 ÷ 0001 ÷	
    check_breaks("\x06\x00\x03\x08\x00\x01"_bs, { "\x06\x00\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0600 × 0308 ÷ 0001 ÷	
    check_breaks("\x06\x00\x03\x4F"_bs, { "\x06\x00\x03\x4F"_bs }); // ÷ 0600 × 034F ÷	
    check_breaks("\x06\x00\x03\x08\x03\x4F"_bs, { "\x06\x00\x03\x08\x03\x4F"_bs }); // ÷ 0600 × 0308 × 034F ÷	
    check_breaks("\x06\x00\xD8\x3C\xDD\xE6"_bs, { "\x06\x00\xD8\x3C\xDD\xE6"_bs }); // ÷ 0600 × 1F1E6 ÷	
    check_breaks("\x06\x00\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x06\x00\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0600 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x06\x00\x06\x00"_bs, { "\x06\x00\x06\x00"_bs }); // ÷ 0600 × 0600 ÷	
    check_breaks("\x06\x00\x03\x08\x06\x00"_bs, { "\x06\x00\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0600 × 0308 ÷ 0600 ÷	
    check_breaks("\x06\x00\x09\x03"_bs, { "\x06\x00\x09\x03"_bs }); // ÷ 0600 × 0903 ÷	
    check_breaks("\x06\x00\x03\x08\x09\x03"_bs, { "\x06\x00\x03\x08\x09\x03"_bs }); // ÷ 0600 × 0308 × 0903 ÷	
    check_breaks("\x06\x00\x11\x00"_bs, { "\x06\x00\x11\x00"_bs }); // ÷ 0600 × 1100 ÷	
    check_breaks("\x06\x00\x03\x08\x11\x00"_bs, { "\x06\x00\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0600 × 0308 ÷ 1100 ÷	
    check_breaks("\x06\x00\x11\x60"_bs, { "\x06\x00\x11\x60"_bs }); // ÷ 0600 × 1160 ÷	
    check_breaks("\x06\x00\x03\x08\x11\x60"_bs, { "\x06\x00\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0600 × 0308 ÷ 1160 ÷	
    check_breaks("\x06\x00\x11\xA8"_bs, { "\x06\x00\x11\xA8"_bs }); // ÷ 0600 × 11A8 ÷	
    check_breaks("\x06\x00\x03\x08\x11\xA8"_bs, { "\x06\x00\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0600 × 0308 ÷ 11A8 ÷	
    check_breaks("\x06\x00\xAC\x00"_bs, { "\x06\x00\xAC\x00"_bs }); // ÷ 0600 × AC00 ÷	
    check_breaks("\x06\x00\x03\x08\xAC\x00"_bs, { "\x06\x00\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0600 × 0308 ÷ AC00 ÷	
    check_breaks("\x06\x00\xAC\x01"_bs, { "\x06\x00\xAC\x01"_bs }); // ÷ 0600 × AC01 ÷	
    check_breaks("\x06\x00\x03\x08\xAC\x01"_bs, { "\x06\x00\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0600 × 0308 ÷ AC01 ÷	
    check_breaks("\x06\x00\x23\x1A"_bs, { "\x06\x00\x23\x1A"_bs }); // ÷ 0600 × 231A ÷	
    check_breaks("\x06\x00\x03\x08\x23\x1A"_bs, { "\x06\x00\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0600 × 0308 ÷ 231A ÷	
    check_breaks("\x06\x00\x03\x00"_bs, { "\x06\x00\x03\x00"_bs }); // ÷ 0600 × 0300 ÷	
    check_breaks("\x06\x00\x03\x08\x03\x00"_bs, { "\x06\x00\x03\x08\x03\x00"_bs }); // ÷ 0600 × 0308 × 0300 ÷	
    check_breaks("\x06\x00\x20\x0D"_bs, { "\x06\x00\x20\x0D"_bs }); // ÷ 0600 × 200D ÷	
    check_breaks("\x06\x00\x03\x08\x20\x0D"_bs, { "\x06\x00\x03\x08\x20\x0D"_bs }); // ÷ 0600 × 0308 × 200D ÷	
    check_breaks("\x06\x00\x03\x78"_bs, { "\x06\x00\x03\x78"_bs }); // ÷ 0600 × 0378 ÷	
    check_breaks("\x06\x00\x03\x08\x03\x78"_bs, { "\x06\x00\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0600 × 0308 ÷ 0378 ÷	
    check_breaks("\x09\x03\x00\x20"_bs, { "\x09\x03"_bs, "\x00\x20"_bs }); // ÷ 0903 ÷ 0020 ÷	
    check_breaks("\x09\x03\x03\x08\x00\x20"_bs, { "\x09\x03\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0903 × 0308 ÷ 0020 ÷	
    check_breaks("\x09\x03\x00\x0D"_bs, { "\x09\x03"_bs, "\x00\x0D"_bs }); // ÷ 0903 ÷ 000D ÷	
    check_breaks("\x09\x03\x03\x08\x00\x0D"_bs, { "\x09\x03\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0903 × 0308 ÷ 000D ÷	
    check_breaks("\x09\x03\x00\x0A"_bs, { "\x09\x03"_bs, "\x00\x0A"_bs }); // ÷ 0903 ÷ 000A ÷	
    check_breaks("\x09\x03\x03\x08\x00\x0A"_bs, { "\x09\x03\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0903 × 0308 ÷ 000A ÷	
    check_breaks("\x09\x03\x00\x01"_bs, { "\x09\x03"_bs, "\x00\x01"_bs }); // ÷ 0903 ÷ 0001 ÷	
    check_breaks("\x09\x03\x03\x08\x00\x01"_bs, { "\x09\x03\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0903 × 0308 ÷ 0001 ÷	
    check_breaks("\x09\x03\x03\x4F"_bs, { "\x09\x03\x03\x4F"_bs }); // ÷ 0903 × 034F ÷	
    check_breaks("\x09\x03\x03\x08\x03\x4F"_bs, { "\x09\x03\x03\x08\x03\x4F"_bs }); // ÷ 0903 × 0308 × 034F ÷	
    check_breaks("\x09\x03\xD8\x3C\xDD\xE6"_bs, { "\x09\x03"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0903 ÷ 1F1E6 ÷	
    check_breaks("\x09\x03\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x09\x03\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0903 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x09\x03\x06\x00"_bs, { "\x09\x03"_bs, "\x06\x00"_bs }); // ÷ 0903 ÷ 0600 ÷	
    check_breaks("\x09\x03\x03\x08\x06\x00"_bs, { "\x09\x03\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0903 × 0308 ÷ 0600 ÷	
    check_breaks("\x09\x03\x09\x03"_bs, { "\x09\x03\x09\x03"_bs }); // ÷ 0903 × 0903 ÷	
    check_breaks("\x09\x03\x03\x08\x09\x03"_bs, { "\x09\x03\x03\x08\x09\x03"_bs }); // ÷ 0903 × 0308 × 0903 ÷	
    check_breaks("\x09\x03\x11\x00"_bs, { "\x09\x03"_bs, "\x11\x00"_bs }); // ÷ 0903 ÷ 1100 ÷	
    check_breaks("\x09\x03\x03\x08\x11\x00"_bs, { "\x09\x03\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0903 × 0308 ÷ 1100 ÷	
    check_breaks("\x09\x03\x11\x60"_bs, { "\x09\x03"_bs, "\x11\x60"_bs }); // ÷ 0903 ÷ 1160 ÷	
    check_breaks("\x09\x03\x03\x08\x11\x60"_bs, { "\x09\x03\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0903 × 0308 ÷ 1160 ÷	
    check_breaks("\x09\x03\x11\xA8"_bs, { "\x09\x03"_bs, "\x11\xA8"_bs }); // ÷ 0903 ÷ 11A8 ÷	
    check_breaks("\x09\x03\x03\x08\x11\xA8"_bs, { "\x09\x03\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0903 × 0308 ÷ 11A8 ÷	
    check_breaks("\x09\x03\xAC\x00"_bs, { "\x09\x03"_bs, "\xAC\x00"_bs }); // ÷ 0903 ÷ AC00 ÷	
    check_breaks("\x09\x03\x03\x08\xAC\x00"_bs, { "\x09\x03\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0903 × 0308 ÷ AC00 ÷	
    check_breaks("\x09\x03\xAC\x01"_bs, { "\x09\x03"_bs, "\xAC\x01"_bs }); // ÷ 0903 ÷ AC01 ÷	
    check_breaks("\x09\x03\x03\x08\xAC\x01"_bs, { "\x09\x03\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0903 × 0308 ÷ AC01 ÷	
    check_breaks("\x09\x03\x23\x1A"_bs, { "\x09\x03"_bs, "\x23\x1A"_bs }); // ÷ 0903 ÷ 231A ÷	
    check_breaks("\x09\x03\x03\x08\x23\x1A"_bs, { "\x09\x03\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0903 × 0308 ÷ 231A ÷	
    check_breaks("\x09\x03\x03\x00"_bs, { "\x09\x03\x03\x00"_bs }); // ÷ 0903 × 0300 ÷	
    check_breaks("\x09\x03\x03\x08\x03\x00"_bs, { "\x09\x03\x03\x08\x03\x00"_bs }); // ÷ 0903 × 0308 × 0300 ÷	
    check_breaks("\x09\x03\x20\x0D"_bs, { "\x09\x03\x20\x0D"_bs }); // ÷ 0903 × 200D ÷	
    check_breaks("\x09\x03\x03\x08\x20\x0D"_bs, { "\x09\x03\x03\x08\x20\x0D"_bs }); // ÷ 0903 × 0308 × 200D ÷	
    check_breaks("\x09\x03\x03\x78"_bs, { "\x09\x03"_bs, "\x03\x78"_bs }); // ÷ 0903 ÷ 0378 ÷	
    check_breaks("\x09\x03\x03\x08\x03\x78"_bs, { "\x09\x03\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0903 × 0308 ÷ 0378 ÷	
    check_breaks("\x11\x00\x00\x20"_bs, { "\x11\x00"_bs, "\x00\x20"_bs }); // ÷ 1100 ÷ 0020 ÷	
    check_breaks("\x11\x00\x03\x08\x00\x20"_bs, { "\x11\x00\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 1100 × 0308 ÷ 0020 ÷	
    check_breaks("\x11\x00\x00\x0D"_bs, { "\x11\x00"_bs, "\x00\x0D"_bs }); // ÷ 1100 ÷ 000D ÷	
    check_breaks("\x11\x00\x03\x08\x00\x0D"_bs, { "\x11\x00\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 1100 × 0308 ÷ 000D ÷	
    check_breaks("\x11\x00\x00\x0A"_bs, { "\x11\x00"_bs, "\x00\x0A"_bs }); // ÷ 1100 ÷ 000A ÷	
    check_breaks("\x11\x00\x03\x08\x00\x0A"_bs, { "\x11\x00\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 1100 × 0308 ÷ 000A ÷	
    check_breaks("\x11\x00\x00\x01"_bs, { "\x11\x00"_bs, "\x00\x01"_bs }); // ÷ 1100 ÷ 0001 ÷	
    check_breaks("\x11\x00\x03\x08\x00\x01"_bs, { "\x11\x00\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 1100 × 0308 ÷ 0001 ÷	
    check_breaks("\x11\x00\x03\x4F"_bs, { "\x11\x00\x03\x4F"_bs }); // ÷ 1100 × 034F ÷	
    check_breaks("\x11\x00\x03\x08\x03\x4F"_bs, { "\x11\x00\x03\x08\x03\x4F"_bs }); // ÷ 1100 × 0308 × 034F ÷	
    check_breaks("\x11\x00\xD8\x3C\xDD\xE6"_bs, { "\x11\x00"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 1100 ÷ 1F1E6 ÷	
    check_breaks("\x11\x00\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x11\x00\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 1100 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x11\x00\x06\x00"_bs, { "\x11\x00"_bs, "\x06\x00"_bs }); // ÷ 1100 ÷ 0600 ÷	
    check_breaks("\x11\x00\x03\x08\x06\x00"_bs, { "\x11\x00\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 1100 × 0308 ÷ 0600 ÷	
    check_breaks("\x11\x00\x09\x03"_bs, { "\x11\x00\x09\x03"_bs }); // ÷ 1100 × 0903 ÷	
    check_breaks("\x11\x00\x03\x08\x09\x03"_bs, { "\x11\x00\x03\x08\x09\x03"_bs }); // ÷ 1100 × 0308 × 0903 ÷	
    check_breaks("\x11\x00\x11\x00"_bs, { "\x11\x00\x11\x00"_bs }); // ÷ 1100 × 1100 ÷	
    check_breaks("\x11\x00\x03\x08\x11\x00"_bs, { "\x11\x00\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 1100 × 0308 ÷ 1100 ÷	
    check_breaks("\x11\x00\x11\x60"_bs, { "\x11\x00\x11\x60"_bs }); // ÷ 1100 × 1160 ÷	
    check_breaks("\x11\x00\x03\x08\x11\x60"_bs, { "\x11\x00\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 1100 × 0308 ÷ 1160 ÷	
    check_breaks("\x11\x00\x11\xA8"_bs, { "\x11\x00"_bs, "\x11\xA8"_bs }); // ÷ 1100 ÷ 11A8 ÷	
    check_breaks("\x11\x00\x03\x08\x11\xA8"_bs, { "\x11\x00\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 1100 × 0308 ÷ 11A8 ÷	
    check_breaks("\x11\x00\xAC\x00"_bs, { "\x11\x00\xAC\x00"_bs }); // ÷ 1100 × AC00 ÷	
    check_breaks("\x11\x00\x03\x08\xAC\x00"_bs, { "\x11\x00\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 1100 × 0308 ÷ AC00 ÷	
    check_breaks("\x11\x00\xAC\x01"_bs, { "\x11\x00\xAC\x01"_bs }); // ÷ 1100 × AC01 ÷	
    check_breaks("\x11\x00\x03\x08\xAC\x01"_bs, { "\x11\x00\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 1100 × 0308 ÷ AC01 ÷	
    check_breaks("\x11\x00\x23\x1A"_bs, { "\x11\x00"_bs, "\x23\x1A"_bs }); // ÷ 1100 ÷ 231A ÷	
    check_breaks("\x11\x00\x03\x08\x23\x1A"_bs, { "\x11\x00\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 1100 × 0308 ÷ 231A ÷	
    check_breaks("\x11\x00\x03\x00"_bs, { "\x11\x00\x03\x00"_bs }); // ÷ 1100 × 0300 ÷	
    check_breaks("\x11\x00\x03\x08\x03\x00"_bs, { "\x11\x00\x03\x08\x03\x00"_bs }); // ÷ 1100 × 0308 × 0300 ÷	
    check_breaks("\x11\x00\x20\x0D"_bs, { "\x11\x00\x20\x0D"_bs }); // ÷ 1100 × 200D ÷	
    check_breaks("\x11\x00\x03\x08\x20\x0D"_bs, { "\x11\x00\x03\x08\x20\x0D"_bs }); // ÷ 1100 × 0308 × 200D ÷	
    check_breaks("\x11\x00\x03\x78"_bs, { "\x11\x00"_bs, "\x03\x78"_bs }); // ÷ 1100 ÷ 0378 ÷	
    check_breaks("\x11\x00\x03\x08\x03\x78"_bs, { "\x11\x00\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 1100 × 0308 ÷ 0378 ÷	
    check_breaks("\x11\x60\x00\x20"_bs, { "\x11\x60"_bs, "\x00\x20"_bs }); // ÷ 1160 ÷ 0020 ÷	
    check_breaks("\x11\x60\x03\x08\x00\x20"_bs, { "\x11\x60\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 1160 × 0308 ÷ 0020 ÷	
    check_breaks("\x11\x60\x00\x0D"_bs, { "\x11\x60"_bs, "\x00\x0D"_bs }); // ÷ 1160 ÷ 000D ÷	
    check_breaks("\x11\x60\x03\x08\x00\x0D"_bs, { "\x11\x60\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 1160 × 0308 ÷ 000D ÷	
    check_breaks("\x11\x60\x00\x0A"_bs, { "\x11\x60"_bs, "\x00\x0A"_bs }); // ÷ 1160 ÷ 000A ÷	
    check_breaks("\x11\x60\x03\x08\x00\x0A"_bs, { "\x11\x60\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 1160 × 0308 ÷ 000A ÷	
    check_breaks("\x11\x60\x00\x01"_bs, { "\x11\x60"_bs, "\x00\x01"_bs }); // ÷ 1160 ÷ 0001 ÷	
    check_breaks("\x11\x60\x03\x08\x00\x01"_bs, { "\x11\x60\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 1160 × 0308 ÷ 0001 ÷	
    check_breaks("\x11\x60\x03\x4F"_bs, { "\x11\x60\x03\x4F"_bs }); // ÷ 1160 × 034F ÷	
    check_breaks("\x11\x60\x03\x08\x03\x4F"_bs, { "\x11\x60\x03\x08\x03\x4F"_bs }); // ÷ 1160 × 0308 × 034F ÷	
    check_breaks("\x11\x60\xD8\x3C\xDD\xE6"_bs, { "\x11\x60"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 1160 ÷ 1F1E6 ÷	
    check_breaks("\x11\x60\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x11\x60\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 1160 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x11\x60\x06\x00"_bs, { "\x11\x60"_bs, "\x06\x00"_bs }); // ÷ 1160 ÷ 0600 ÷	
    check_breaks("\x11\x60\x03\x08\x06\x00"_bs, { "\x11\x60\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 1160 × 0308 ÷ 0600 ÷	
    check_breaks("\x11\x60\x09\x03"_bs, { "\x11\x60\x09\x03"_bs }); // ÷ 1160 × 0903 ÷	
    check_breaks("\x11\x60\x03\x08\x09\x03"_bs, { "\x11\x60\x03\x08\x09\x03"_bs }); // ÷ 1160 × 0308 × 0903 ÷	
    check_breaks("\x11\x60\x11\x00"_bs, { "\x11\x60"_bs, "\x11\x00"_bs }); // ÷ 1160 ÷ 1100 ÷	
    check_breaks("\x11\x60\x03\x08\x11\x00"_bs, { "\x11\x60\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 1160 × 0308 ÷ 1100 ÷	
    check_breaks("\x11\x60\x11\x60"_bs, { "\x11\x60\x11\x60"_bs }); // ÷ 1160 × 1160 ÷	
    check_breaks("\x11\x60\x03\x08\x11\x60"_bs, { "\x11\x60\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 1160 × 0308 ÷ 1160 ÷	
    check_breaks("\x11\x60\x11\xA8"_bs, { "\x11\x60\x11\xA8"_bs }); // ÷ 1160 × 11A8 ÷	
    check_breaks("\x11\x60\x03\x08\x11\xA8"_bs, { "\x11\x60\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 1160 × 0308 ÷ 11A8 ÷	
    check_breaks("\x11\x60\xAC\x00"_bs, { "\x11\x60"_bs, "\xAC\x00"_bs }); // ÷ 1160 ÷ AC00 ÷	
    check_breaks("\x11\x60\x03\x08\xAC\x00"_bs, { "\x11\x60\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 1160 × 0308 ÷ AC00 ÷	
    check_breaks("\x11\x60\xAC\x01"_bs, { "\x11\x60"_bs, "\xAC\x01"_bs }); // ÷ 1160 ÷ AC01 ÷	
    check_breaks("\x11\x60\x03\x08\xAC\x01"_bs, { "\x11\x60\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 1160 × 0308 ÷ AC01 ÷	
    check_breaks("\x11\x60\x23\x1A"_bs, { "\x11\x60"_bs, "\x23\x1A"_bs }); // ÷ 1160 ÷ 231A ÷	
    check_breaks("\x11\x60\x03\x08\x23\x1A"_bs, { "\x11\x60\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 1160 × 0308 ÷ 231A ÷	
    check_breaks("\x11\x60\x03\x00"_bs, { "\x11\x60\x03\x00"_bs }); // ÷ 1160 × 0300 ÷	
    check_breaks("\x11\x60\x03\x08\x03\x00"_bs, { "\x11\x60\x03\x08\x03\x00"_bs }); // ÷ 1160 × 0308 × 0300 ÷	
    check_breaks("\x11\x60\x20\x0D"_bs, { "\x11\x60\x20\x0D"_bs }); // ÷ 1160 × 200D ÷	
    check_breaks("\x11\x60\x03\x08\x20\x0D"_bs, { "\x11\x60\x03\x08\x20\x0D"_bs }); // ÷ 1160 × 0308 × 200D ÷	
    check_breaks("\x11\x60\x03\x78"_bs, { "\x11\x60"_bs, "\x03\x78"_bs }); // ÷ 1160 ÷ 0378 ÷	
    check_breaks("\x11\x60\x03\x08\x03\x78"_bs, { "\x11\x60\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 1160 × 0308 ÷ 0378 ÷	
    check_breaks("\x11\xA8\x00\x20"_bs, { "\x11\xA8"_bs, "\x00\x20"_bs }); // ÷ 11A8 ÷ 0020 ÷	
    check_breaks("\x11\xA8\x03\x08\x00\x20"_bs, { "\x11\xA8\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 11A8 × 0308 ÷ 0020 ÷	
    check_breaks("\x11\xA8\x00\x0D"_bs, { "\x11\xA8"_bs, "\x00\x0D"_bs }); // ÷ 11A8 ÷ 000D ÷	
    check_breaks("\x11\xA8\x03\x08\x00\x0D"_bs, { "\x11\xA8\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 11A8 × 0308 ÷ 000D ÷	
    check_breaks("\x11\xA8\x00\x0A"_bs, { "\x11\xA8"_bs, "\x00\x0A"_bs }); // ÷ 11A8 ÷ 000A ÷	
    check_breaks("\x11\xA8\x03\x08\x00\x0A"_bs, { "\x11\xA8\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 11A8 × 0308 ÷ 000A ÷	
    check_breaks("\x11\xA8\x00\x01"_bs, { "\x11\xA8"_bs, "\x00\x01"_bs }); // ÷ 11A8 ÷ 0001 ÷	
    check_breaks("\x11\xA8\x03\x08\x00\x01"_bs, { "\x11\xA8\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 11A8 × 0308 ÷ 0001 ÷	
    check_breaks("\x11\xA8\x03\x4F"_bs, { "\x11\xA8\x03\x4F"_bs }); // ÷ 11A8 × 034F ÷	
    check_breaks("\x11\xA8\x03\x08\x03\x4F"_bs, { "\x11\xA8\x03\x08\x03\x4F"_bs }); // ÷ 11A8 × 0308 × 034F ÷	
    check_breaks("\x11\xA8\xD8\x3C\xDD\xE6"_bs, { "\x11\xA8"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 11A8 ÷ 1F1E6 ÷	
    check_breaks("\x11\xA8\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x11\xA8\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 11A8 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x11\xA8\x06\x00"_bs, { "\x11\xA8"_bs, "\x06\x00"_bs }); // ÷ 11A8 ÷ 0600 ÷	
    check_breaks("\x11\xA8\x03\x08\x06\x00"_bs, { "\x11\xA8\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 11A8 × 0308 ÷ 0600 ÷	
    check_breaks("\x11\xA8\x09\x03"_bs, { "\x11\xA8\x09\x03"_bs }); // ÷ 11A8 × 0903 ÷	
    check_breaks("\x11\xA8\x03\x08\x09\x03"_bs, { "\x11\xA8\x03\x08\x09\x03"_bs }); // ÷ 11A8 × 0308 × 0903 ÷	
    check_breaks("\x11\xA8\x11\x00"_bs, { "\x11\xA8"_bs, "\x11\x00"_bs }); // ÷ 11A8 ÷ 1100 ÷	
    check_breaks("\x11\xA8\x03\x08\x11\x00"_bs, { "\x11\xA8\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 11A8 × 0308 ÷ 1100 ÷	
    check_breaks("\x11\xA8\x11\x60"_bs, { "\x11\xA8"_bs, "\x11\x60"_bs }); // ÷ 11A8 ÷ 1160 ÷	
    check_breaks("\x11\xA8\x03\x08\x11\x60"_bs, { "\x11\xA8\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 11A8 × 0308 ÷ 1160 ÷	
    check_breaks("\x11\xA8\x11\xA8"_bs, { "\x11\xA8\x11\xA8"_bs }); // ÷ 11A8 × 11A8 ÷	
    check_breaks("\x11\xA8\x03\x08\x11\xA8"_bs, { "\x11\xA8\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 11A8 × 0308 ÷ 11A8 ÷	
    check_breaks("\x11\xA8\xAC\x00"_bs, { "\x11\xA8"_bs, "\xAC\x00"_bs }); // ÷ 11A8 ÷ AC00 ÷	
    check_breaks("\x11\xA8\x03\x08\xAC\x00"_bs, { "\x11\xA8\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 11A8 × 0308 ÷ AC00 ÷	
    check_breaks("\x11\xA8\xAC\x01"_bs, { "\x11\xA8"_bs, "\xAC\x01"_bs }); // ÷ 11A8 ÷ AC01 ÷	
    check_breaks("\x11\xA8\x03\x08\xAC\x01"_bs, { "\x11\xA8\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 11A8 × 0308 ÷ AC01 ÷	
    check_breaks("\x11\xA8\x23\x1A"_bs, { "\x11\xA8"_bs, "\x23\x1A"_bs }); // ÷ 11A8 ÷ 231A ÷	
    check_breaks("\x11\xA8\x03\x08\x23\x1A"_bs, { "\x11\xA8\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 11A8 × 0308 ÷ 231A ÷	
    check_breaks("\x11\xA8\x03\x00"_bs, { "\x11\xA8\x03\x00"_bs }); // ÷ 11A8 × 0300 ÷	
    check_breaks("\x11\xA8\x03\x08\x03\x00"_bs, { "\x11\xA8\x03\x08\x03\x00"_bs }); // ÷ 11A8 × 0308 × 0300 ÷	
    check_breaks("\x11\xA8\x20\x0D"_bs, { "\x11\xA8\x20\x0D"_bs }); // ÷ 11A8 × 200D ÷	
    check_breaks("\x11\xA8\x03\x08\x20\x0D"_bs, { "\x11\xA8\x03\x08\x20\x0D"_bs }); // ÷ 11A8 × 0308 × 200D ÷	
    check_breaks("\x11\xA8\x03\x78"_bs, { "\x11\xA8"_bs, "\x03\x78"_bs }); // ÷ 11A8 ÷ 0378 ÷	
    check_breaks("\x11\xA8\x03\x08\x03\x78"_bs, { "\x11\xA8\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 11A8 × 0308 ÷ 0378 ÷	
    check_breaks("\xAC\x00\x00\x20"_bs, { "\xAC\x00"_bs, "\x00\x20"_bs }); // ÷ AC00 ÷ 0020 ÷	
    check_breaks("\xAC\x00\x03\x08\x00\x20"_bs, { "\xAC\x00\x03\x08"_bs, "\x00\x20"_bs }); // ÷ AC00 × 0308 ÷ 0020 ÷	
    check_breaks("\xAC\x00\x00\x0D"_bs, { "\xAC\x00"_bs, "\x00\x0D"_bs }); // ÷ AC00 ÷ 000D ÷	
    check_breaks("\xAC\x00\x03\x08\x00\x0D"_bs, { "\xAC\x00\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ AC00 × 0308 ÷ 000D ÷	
    check_breaks("\xAC\x00\x00\x0A"_bs, { "\xAC\x00"_bs, "\x00\x0A"_bs }); // ÷ AC00 ÷ 000A ÷	
    check_breaks("\xAC\x00\x03\x08\x00\x0A"_bs, { "\xAC\x00\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ AC00 × 0308 ÷ 000A ÷	
    check_breaks("\xAC\x00\x00\x01"_bs, { "\xAC\x00"_bs, "\x00\x01"_bs }); // ÷ AC00 ÷ 0001 ÷	
    check_breaks("\xAC\x00\x03\x08\x00\x01"_bs, { "\xAC\x00\x03\x08"_bs, "\x00\x01"_bs }); // ÷ AC00 × 0308 ÷ 0001 ÷	
    check_breaks("\xAC\x00\x03\x4F"_bs, { "\xAC\x00\x03\x4F"_bs }); // ÷ AC00 × 034F ÷	
    check_breaks("\xAC\x00\x03\x08\x03\x4F"_bs, { "\xAC\x00\x03\x08\x03\x4F"_bs }); // ÷ AC00 × 0308 × 034F ÷	
    check_breaks("\xAC\x00\xD8\x3C\xDD\xE6"_bs, { "\xAC\x00"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ AC00 ÷ 1F1E6 ÷	
    check_breaks("\xAC\x00\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\xAC\x00\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ AC00 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\xAC\x00\x06\x00"_bs, { "\xAC\x00"_bs, "\x06\x00"_bs }); // ÷ AC00 ÷ 0600 ÷	
    check_breaks("\xAC\x00\x03\x08\x06\x00"_bs, { "\xAC\x00\x03\x08"_bs, "\x06\x00"_bs }); // ÷ AC00 × 0308 ÷ 0600 ÷	
    check_breaks("\xAC\x00\x09\x03"_bs, { "\xAC\x00\x09\x03"_bs }); // ÷ AC00 × 0903 ÷	
    check_breaks("\xAC\x00\x03\x08\x09\x03"_bs, { "\xAC\x00\x03\x08\x09\x03"_bs }); // ÷ AC00 × 0308 × 0903 ÷	
    check_breaks("\xAC\x00\x11\x00"_bs, { "\xAC\x00"_bs, "\x11\x00"_bs }); // ÷ AC00 ÷ 1100 ÷	
    check_breaks("\xAC\x00\x03\x08\x11\x00"_bs, { "\xAC\x00\x03\x08"_bs, "\x11\x00"_bs }); // ÷ AC00 × 0308 ÷ 1100 ÷	
    check_breaks("\xAC\x00\x11\x60"_bs, { "\xAC\x00\x11\x60"_bs }); // ÷ AC00 × 1160 ÷	
    check_breaks("\xAC\x00\x03\x08\x11\x60"_bs, { "\xAC\x00\x03\x08"_bs, "\x11\x60"_bs }); // ÷ AC00 × 0308 ÷ 1160 ÷	
    check_breaks("\xAC\x00\x11\xA8"_bs, { "\xAC\x00\x11\xA8"_bs }); // ÷ AC00 × 11A8 ÷	
    check_breaks("\xAC\x00\x03\x08\x11\xA8"_bs, { "\xAC\x00\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ AC00 × 0308 ÷ 11A8 ÷	
    check_breaks("\xAC\x00\xAC\x00"_bs, { "\xAC\x00"_bs, "\xAC\x00"_bs }); // ÷ AC00 ÷ AC00 ÷	
    check_breaks("\xAC\x00\x03\x08\xAC\x00"_bs, { "\xAC\x00\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ AC00 × 0308 ÷ AC00 ÷	
    check_breaks("\xAC\x00\xAC\x01"_bs, { "\xAC\x00"_bs, "\xAC\x01"_bs }); // ÷ AC00 ÷ AC01 ÷	
    check_breaks("\xAC\x00\x03\x08\xAC\x01"_bs, { "\xAC\x00\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ AC00 × 0308 ÷ AC01 ÷	
    check_breaks("\xAC\x00\x23\x1A"_bs, { "\xAC\x00"_bs, "\x23\x1A"_bs }); // ÷ AC00 ÷ 231A ÷	
    check_breaks("\xAC\x00\x03\x08\x23\x1A"_bs, { "\xAC\x00\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ AC00 × 0308 ÷ 231A ÷	
    check_breaks("\xAC\x00\x03\x00"_bs, { "\xAC\x00\x03\x00"_bs }); // ÷ AC00 × 0300 ÷	
    check_breaks("\xAC\x00\x03\x08\x03\x00"_bs, { "\xAC\x00\x03\x08\x03\x00"_bs }); // ÷ AC00 × 0308 × 0300 ÷	
    check_breaks("\xAC\x00\x20\x0D"_bs, { "\xAC\x00\x20\x0D"_bs }); // ÷ AC00 × 200D ÷	
    check_breaks("\xAC\x00\x03\x08\x20\x0D"_bs, { "\xAC\x00\x03\x08\x20\x0D"_bs }); // ÷ AC00 × 0308 × 200D ÷	
    check_breaks("\xAC\x00\x03\x78"_bs, { "\xAC\x00"_bs, "\x03\x78"_bs }); // ÷ AC00 ÷ 0378 ÷	
    check_breaks("\xAC\x00\x03\x08\x03\x78"_bs, { "\xAC\x00\x03\x08"_bs, "\x03\x78"_bs }); // ÷ AC00 × 0308 ÷ 0378 ÷	
    check_breaks("\xAC\x01\x00\x20"_bs, { "\xAC\x01"_bs, "\x00\x20"_bs }); // ÷ AC01 ÷ 0020 ÷	
    check_breaks("\xAC\x01\x03\x08\x00\x20"_bs, { "\xAC\x01\x03\x08"_bs, "\x00\x20"_bs }); // ÷ AC01 × 0308 ÷ 0020 ÷	
    check_breaks("\xAC\x01\x00\x0D"_bs, { "\xAC\x01"_bs, "\x00\x0D"_bs }); // ÷ AC01 ÷ 000D ÷	
    check_breaks("\xAC\x01\x03\x08\x00\x0D"_bs, { "\xAC\x01\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ AC01 × 0308 ÷ 000D ÷	
    check_breaks("\xAC\x01\x00\x0A"_bs, { "\xAC\x01"_bs, "\x00\x0A"_bs }); // ÷ AC01 ÷ 000A ÷	
    check_breaks("\xAC\x01\x03\x08\x00\x0A"_bs, { "\xAC\x01\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ AC01 × 0308 ÷ 000A ÷	
    check_breaks("\xAC\x01\x00\x01"_bs, { "\xAC\x01"_bs, "\x00\x01"_bs }); // ÷ AC01 ÷ 0001 ÷	
    check_breaks("\xAC\x01\x03\x08\x00\x01"_bs, { "\xAC\x01\x03\x08"_bs, "\x00\x01"_bs }); // ÷ AC01 × 0308 ÷ 0001 ÷	
    check_breaks("\xAC\x01\x03\x4F"_bs, { "\xAC\x01\x03\x4F"_bs }); // ÷ AC01 × 034F ÷	
    check_breaks("\xAC\x01\x03\x08\x03\x4F"_bs, { "\xAC\x01\x03\x08\x03\x4F"_bs }); // ÷ AC01 × 0308 × 034F ÷	
    check_breaks("\xAC\x01\xD8\x3C\xDD\xE6"_bs, { "\xAC\x01"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ AC01 ÷ 1F1E6 ÷	
    check_breaks("\xAC\x01\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\xAC\x01\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ AC01 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\xAC\x01\x06\x00"_bs, { "\xAC\x01"_bs, "\x06\x00"_bs }); // ÷ AC01 ÷ 0600 ÷	
    check_breaks("\xAC\x01\x03\x08\x06\x00"_bs, { "\xAC\x01\x03\x08"_bs, "\x06\x00"_bs }); // ÷ AC01 × 0308 ÷ 0600 ÷	
    check_breaks("\xAC\x01\x09\x03"_bs, { "\xAC\x01\x09\x03"_bs }); // ÷ AC01 × 0903 ÷	
    check_breaks("\xAC\x01\x03\x08\x09\x03"_bs, { "\xAC\x01\x03\x08\x09\x03"_bs }); // ÷ AC01 × 0308 × 0903 ÷	
    check_breaks("\xAC\x01\x11\x00"_bs, { "\xAC\x01"_bs, "\x11\x00"_bs }); // ÷ AC01 ÷ 1100 ÷	
    check_breaks("\xAC\x01\x03\x08\x11\x00"_bs, { "\xAC\x01\x03\x08"_bs, "\x11\x00"_bs }); // ÷ AC01 × 0308 ÷ 1100 ÷	
    check_breaks("\xAC\x01\x11\x60"_bs, { "\xAC\x01"_bs, "\x11\x60"_bs }); // ÷ AC01 ÷ 1160 ÷	
    check_breaks("\xAC\x01\x03\x08\x11\x60"_bs, { "\xAC\x01\x03\x08"_bs, "\x11\x60"_bs }); // ÷ AC01 × 0308 ÷ 1160 ÷	
    check_breaks("\xAC\x01\x11\xA8"_bs, { "\xAC\x01\x11\xA8"_bs }); // ÷ AC01 × 11A8 ÷	
    check_breaks("\xAC\x01\x03\x08\x11\xA8"_bs, { "\xAC\x01\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ AC01 × 0308 ÷ 11A8 ÷	
    check_breaks("\xAC\x01\xAC\x00"_bs, { "\xAC\x01"_bs, "\xAC\x00"_bs }); // ÷ AC01 ÷ AC00 ÷	
    check_breaks("\xAC\x01\x03\x08\xAC\x00"_bs, { "\xAC\x01\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ AC01 × 0308 ÷ AC00 ÷	
    check_breaks("\xAC\x01\xAC\x01"_bs, { "\xAC\x01"_bs, "\xAC\x01"_bs }); // ÷ AC01 ÷ AC01 ÷	
    check_breaks("\xAC\x01\x03\x08\xAC\x01"_bs, { "\xAC\x01\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ AC01 × 0308 ÷ AC01 ÷	
    check_breaks("\xAC\x01\x23\x1A"_bs, { "\xAC\x01"_bs, "\x23\x1A"_bs }); // ÷ AC01 ÷ 231A ÷	
    check_breaks("\xAC\x01\x03\x08\x23\x1A"_bs, { "\xAC\x01\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ AC01 × 0308 ÷ 231A ÷	
    check_breaks("\xAC\x01\x03\x00"_bs, { "\xAC\x01\x03\x00"_bs }); // ÷ AC01 × 0300 ÷	
    check_breaks("\xAC\x01\x03\x08\x03\x00"_bs, { "\xAC\x01\x03\x08\x03\x00"_bs }); // ÷ AC01 × 0308 × 0300 ÷	
    check_breaks("\xAC\x01\x20\x0D"_bs, { "\xAC\x01\x20\x0D"_bs }); // ÷ AC01 × 200D ÷	
    check_breaks("\xAC\x01\x03\x08\x20\x0D"_bs, { "\xAC\x01\x03\x08\x20\x0D"_bs }); // ÷ AC01 × 0308 × 200D ÷	
    check_breaks("\xAC\x01\x03\x78"_bs, { "\xAC\x01"_bs, "\x03\x78"_bs }); // ÷ AC01 ÷ 0378 ÷	
    check_breaks("\xAC\x01\x03\x08\x03\x78"_bs, { "\xAC\x01\x03\x08"_bs, "\x03\x78"_bs }); // ÷ AC01 × 0308 ÷ 0378 ÷	
    check_breaks("\x23\x1A\x00\x20"_bs, { "\x23\x1A"_bs, "\x00\x20"_bs }); // ÷ 231A ÷ 0020 ÷	
    check_breaks("\x23\x1A\x03\x08\x00\x20"_bs, { "\x23\x1A\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 231A × 0308 ÷ 0020 ÷	
    check_breaks("\x23\x1A\x00\x0D"_bs, { "\x23\x1A"_bs, "\x00\x0D"_bs }); // ÷ 231A ÷ 000D ÷	
    check_breaks("\x23\x1A\x03\x08\x00\x0D"_bs, { "\x23\x1A\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 231A × 0308 ÷ 000D ÷	
    check_breaks("\x23\x1A\x00\x0A"_bs, { "\x23\x1A"_bs, "\x00\x0A"_bs }); // ÷ 231A ÷ 000A ÷	
    check_breaks("\x23\x1A\x03\x08\x00\x0A"_bs, { "\x23\x1A\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 231A × 0308 ÷ 000A ÷	
    check_breaks("\x23\x1A\x00\x01"_bs, { "\x23\x1A"_bs, "\x00\x01"_bs }); // ÷ 231A ÷ 0001 ÷	
    check_breaks("\x23\x1A\x03\x08\x00\x01"_bs, { "\x23\x1A\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 231A × 0308 ÷ 0001 ÷	
    check_breaks("\x23\x1A\x03\x4F"_bs, { "\x23\x1A\x03\x4F"_bs }); // ÷ 231A × 034F ÷	
    check_breaks("\x23\x1A\x03\x08\x03\x4F"_bs, { "\x23\x1A\x03\x08\x03\x4F"_bs }); // ÷ 231A × 0308 × 034F ÷	
    check_breaks("\x23\x1A\xD8\x3C\xDD\xE6"_bs, { "\x23\x1A"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 231A ÷ 1F1E6 ÷	
    check_breaks("\x23\x1A\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x23\x1A\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 231A × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x23\x1A\x06\x00"_bs, { "\x23\x1A"_bs, "\x06\x00"_bs }); // ÷ 231A ÷ 0600 ÷	
    check_breaks("\x23\x1A\x03\x08\x06\x00"_bs, { "\x23\x1A\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 231A × 0308 ÷ 0600 ÷	
    check_breaks("\x23\x1A\x09\x03"_bs, { "\x23\x1A\x09\x03"_bs }); // ÷ 231A × 0903 ÷	
    check_breaks("\x23\x1A\x03\x08\x09\x03"_bs, { "\x23\x1A\x03\x08\x09\x03"_bs }); // ÷ 231A × 0308 × 0903 ÷	
    check_breaks("\x23\x1A\x11\x00"_bs, { "\x23\x1A"_bs, "\x11\x00"_bs }); // ÷ 231A ÷ 1100 ÷	
    check_breaks("\x23\x1A\x03\x08\x11\x00"_bs, { "\x23\x1A\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 231A × 0308 ÷ 1100 ÷	
    check_breaks("\x23\x1A\x11\x60"_bs, { "\x23\x1A"_bs, "\x11\x60"_bs }); // ÷ 231A ÷ 1160 ÷	
    check_breaks("\x23\x1A\x03\x08\x11\x60"_bs, { "\x23\x1A\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 231A × 0308 ÷ 1160 ÷	
    check_breaks("\x23\x1A\x11\xA8"_bs, { "\x23\x1A"_bs, "\x11\xA8"_bs }); // ÷ 231A ÷ 11A8 ÷	
    check_breaks("\x23\x1A\x03\x08\x11\xA8"_bs, { "\x23\x1A\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 231A × 0308 ÷ 11A8 ÷	
    check_breaks("\x23\x1A\xAC\x00"_bs, { "\x23\x1A"_bs, "\xAC\x00"_bs }); // ÷ 231A ÷ AC00 ÷	
    check_breaks("\x23\x1A\x03\x08\xAC\x00"_bs, { "\x23\x1A\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 231A × 0308 ÷ AC00 ÷	
    check_breaks("\x23\x1A\xAC\x01"_bs, { "\x23\x1A"_bs, "\xAC\x01"_bs }); // ÷ 231A ÷ AC01 ÷	
    check_breaks("\x23\x1A\x03\x08\xAC\x01"_bs, { "\x23\x1A\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 231A × 0308 ÷ AC01 ÷	
    check_breaks("\x23\x1A\x23\x1A"_bs, { "\x23\x1A"_bs, "\x23\x1A"_bs }); // ÷ 231A ÷ 231A ÷	
    check_breaks("\x23\x1A\x03\x08\x23\x1A"_bs, { "\x23\x1A\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 231A × 0308 ÷ 231A ÷	
    check_breaks("\x23\x1A\x03\x00"_bs, { "\x23\x1A\x03\x00"_bs }); // ÷ 231A × 0300 ÷	
    check_breaks("\x23\x1A\x03\x08\x03\x00"_bs, { "\x23\x1A\x03\x08\x03\x00"_bs }); // ÷ 231A × 0308 × 0300 ÷	
    check_breaks("\x23\x1A\x20\x0D"_bs, { "\x23\x1A\x20\x0D"_bs }); // ÷ 231A × 200D ÷	
    check_breaks("\x23\x1A\x03\x08\x20\x0D"_bs, { "\x23\x1A\x03\x08\x20\x0D"_bs }); // ÷ 231A × 0308 × 200D ÷	
    check_breaks("\x23\x1A\x03\x78"_bs, { "\x23\x1A"_bs, "\x03\x78"_bs }); // ÷ 231A ÷ 0378 ÷	
    check_breaks("\x23\x1A\x03\x08\x03\x78"_bs, { "\x23\x1A\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 231A × 0308 ÷ 0378 ÷	
    check_breaks("\x03\x00\x00\x20"_bs, { "\x03\x00"_bs, "\x00\x20"_bs }); // ÷ 0300 ÷ 0020 ÷	
    check_breaks("\x03\x00\x03\x08\x00\x20"_bs, { "\x03\x00\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0300 × 0308 ÷ 0020 ÷	
    check_breaks("\x03\x00\x00\x0D"_bs, { "\x03\x00"_bs, "\x00\x0D"_bs }); // ÷ 0300 ÷ 000D ÷	
    check_breaks("\x03\x00\x03\x08\x00\x0D"_bs, { "\x03\x00\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0300 × 0308 ÷ 000D ÷	
    check_breaks("\x03\x00\x00\x0A"_bs, { "\x03\x00"_bs, "\x00\x0A"_bs }); // ÷ 0300 ÷ 000A ÷	
    check_breaks("\x03\x00\x03\x08\x00\x0A"_bs, { "\x03\x00\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0300 × 0308 ÷ 000A ÷	
    check_breaks("\x03\x00\x00\x01"_bs, { "\x03\x00"_bs, "\x00\x01"_bs }); // ÷ 0300 ÷ 0001 ÷	
    check_breaks("\x03\x00\x03\x08\x00\x01"_bs, { "\x03\x00\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0300 × 0308 ÷ 0001 ÷	
    check_breaks("\x03\x00\x03\x4F"_bs, { "\x03\x00\x03\x4F"_bs }); // ÷ 0300 × 034F ÷	
    check_breaks("\x03\x00\x03\x08\x03\x4F"_bs, { "\x03\x00\x03\x08\x03\x4F"_bs }); // ÷ 0300 × 0308 × 034F ÷	
    check_breaks("\x03\x00\xD8\x3C\xDD\xE6"_bs, { "\x03\x00"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0300 ÷ 1F1E6 ÷	
    check_breaks("\x03\x00\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x03\x00\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0300 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x03\x00\x06\x00"_bs, { "\x03\x00"_bs, "\x06\x00"_bs }); // ÷ 0300 ÷ 0600 ÷	
    check_breaks("\x03\x00\x03\x08\x06\x00"_bs, { "\x03\x00\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0300 × 0308 ÷ 0600 ÷	
    check_breaks("\x03\x00\x09\x03"_bs, { "\x03\x00\x09\x03"_bs }); // ÷ 0300 × 0903 ÷	
    check_breaks("\x03\x00\x03\x08\x09\x03"_bs, { "\x03\x00\x03\x08\x09\x03"_bs }); // ÷ 0300 × 0308 × 0903 ÷	
    check_breaks("\x03\x00\x11\x00"_bs, { "\x03\x00"_bs, "\x11\x00"_bs }); // ÷ 0300 ÷ 1100 ÷	
    check_breaks("\x03\x00\x03\x08\x11\x00"_bs, { "\x03\x00\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0300 × 0308 ÷ 1100 ÷	
    check_breaks("\x03\x00\x11\x60"_bs, { "\x03\x00"_bs, "\x11\x60"_bs }); // ÷ 0300 ÷ 1160 ÷	
    check_breaks("\x03\x00\x03\x08\x11\x60"_bs, { "\x03\x00\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0300 × 0308 ÷ 1160 ÷	
    check_breaks("\x03\x00\x11\xA8"_bs, { "\x03\x00"_bs, "\x11\xA8"_bs }); // ÷ 0300 ÷ 11A8 ÷	
    check_breaks("\x03\x00\x03\x08\x11\xA8"_bs, { "\x03\x00\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0300 × 0308 ÷ 11A8 ÷	
    check_breaks("\x03\x00\xAC\x00"_bs, { "\x03\x00"_bs, "\xAC\x00"_bs }); // ÷ 0300 ÷ AC00 ÷	
    check_breaks("\x03\x00\x03\x08\xAC\x00"_bs, { "\x03\x00\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0300 × 0308 ÷ AC00 ÷	
    check_breaks("\x03\x00\xAC\x01"_bs, { "\x03\x00"_bs, "\xAC\x01"_bs }); // ÷ 0300 ÷ AC01 ÷	
    check_breaks("\x03\x00\x03\x08\xAC\x01"_bs, { "\x03\x00\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0300 × 0308 ÷ AC01 ÷	
    check_breaks("\x03\x00\x23\x1A"_bs, { "\x03\x00"_bs, "\x23\x1A"_bs }); // ÷ 0300 ÷ 231A ÷	
    check_breaks("\x03\x00\x03\x08\x23\x1A"_bs, { "\x03\x00\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0300 × 0308 ÷ 231A ÷	
    check_breaks("\x03\x00\x03\x00"_bs, { "\x03\x00\x03\x00"_bs }); // ÷ 0300 × 0300 ÷	
    check_breaks("\x03\x00\x03\x08\x03\x00"_bs, { "\x03\x00\x03\x08\x03\x00"_bs }); // ÷ 0300 × 0308 × 0300 ÷	
    check_breaks("\x03\x00\x20\x0D"_bs, { "\x03\x00\x20\x0D"_bs }); // ÷ 0300 × 200D ÷	
    check_breaks("\x03\x00\x03\x08\x20\x0D"_bs, { "\x03\x00\x03\x08\x20\x0D"_bs }); // ÷ 0300 × 0308 × 200D ÷	
    check_breaks("\x03\x00\x03\x78"_bs, { "\x03\x00"_bs, "\x03\x78"_bs }); // ÷ 0300 ÷ 0378 ÷	
    check_breaks("\x03\x00\x03\x08\x03\x78"_bs, { "\x03\x00\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0300 × 0308 ÷ 0378 ÷	
    check_breaks("\x20\x0D\x00\x20"_bs, { "\x20\x0D"_bs, "\x00\x20"_bs }); // ÷ 200D ÷ 0020 ÷	
    check_breaks("\x20\x0D\x03\x08\x00\x20"_bs, { "\x20\x0D\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 200D × 0308 ÷ 0020 ÷	
    check_breaks("\x20\x0D\x00\x0D"_bs, { "\x20\x0D"_bs, "\x00\x0D"_bs }); // ÷ 200D ÷ 000D ÷	
    check_breaks("\x20\x0D\x03\x08\x00\x0D"_bs, { "\x20\x0D\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 200D × 0308 ÷ 000D ÷	
    check_breaks("\x20\x0D\x00\x0A"_bs, { "\x20\x0D"_bs, "\x00\x0A"_bs }); // ÷ 200D ÷ 000A ÷	
    check_breaks("\x20\x0D\x03\x08\x00\x0A"_bs, { "\x20\x0D\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 200D × 0308 ÷ 000A ÷	
    check_breaks("\x20\x0D\x00\x01"_bs, { "\x20\x0D"_bs, "\x00\x01"_bs }); // ÷ 200D ÷ 0001 ÷	
    check_breaks("\x20\x0D\x03\x08\x00\x01"_bs, { "\x20\x0D\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 200D × 0308 ÷ 0001 ÷	
    check_breaks("\x20\x0D\x03\x4F"_bs, { "\x20\x0D\x03\x4F"_bs }); // ÷ 200D × 034F ÷	
    check_breaks("\x20\x0D\x03\x08\x03\x4F"_bs, { "\x20\x0D\x03\x08\x03\x4F"_bs }); // ÷ 200D × 0308 × 034F ÷	
    check_breaks("\x20\x0D\xD8\x3C\xDD\xE6"_bs, { "\x20\x0D"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 200D ÷ 1F1E6 ÷	
    check_breaks("\x20\x0D\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x20\x0D\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 200D × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x20\x0D\x06\x00"_bs, { "\x20\x0D"_bs, "\x06\x00"_bs }); // ÷ 200D ÷ 0600 ÷	
    check_breaks("\x20\x0D\x03\x08\x06\x00"_bs, { "\x20\x0D\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 200D × 0308 ÷ 0600 ÷	
    check_breaks("\x20\x0D\x09\x03"_bs, { "\x20\x0D\x09\x03"_bs }); // ÷ 200D × 0903 ÷	
    check_breaks("\x20\x0D\x03\x08\x09\x03"_bs, { "\x20\x0D\x03\x08\x09\x03"_bs }); // ÷ 200D × 0308 × 0903 ÷	
    check_breaks("\x20\x0D\x11\x00"_bs, { "\x20\x0D"_bs, "\x11\x00"_bs }); // ÷ 200D ÷ 1100 ÷	
    check_breaks("\x20\x0D\x03\x08\x11\x00"_bs, { "\x20\x0D\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 200D × 0308 ÷ 1100 ÷	
    check_breaks("\x20\x0D\x11\x60"_bs, { "\x20\x0D"_bs, "\x11\x60"_bs }); // ÷ 200D ÷ 1160 ÷	
    check_breaks("\x20\x0D\x03\x08\x11\x60"_bs, { "\x20\x0D\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 200D × 0308 ÷ 1160 ÷	
    check_breaks("\x20\x0D\x11\xA8"_bs, { "\x20\x0D"_bs, "\x11\xA8"_bs }); // ÷ 200D ÷ 11A8 ÷	
    check_breaks("\x20\x0D\x03\x08\x11\xA8"_bs, { "\x20\x0D\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 200D × 0308 ÷ 11A8 ÷	
    check_breaks("\x20\x0D\xAC\x00"_bs, { "\x20\x0D"_bs, "\xAC\x00"_bs }); // ÷ 200D ÷ AC00 ÷	
    check_breaks("\x20\x0D\x03\x08\xAC\x00"_bs, { "\x20\x0D\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 200D × 0308 ÷ AC00 ÷	
    check_breaks("\x20\x0D\xAC\x01"_bs, { "\x20\x0D"_bs, "\xAC\x01"_bs }); // ÷ 200D ÷ AC01 ÷	
    check_breaks("\x20\x0D\x03\x08\xAC\x01"_bs, { "\x20\x0D\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 200D × 0308 ÷ AC01 ÷	
    check_breaks("\x20\x0D\x23\x1A"_bs, { "\x20\x0D"_bs, "\x23\x1A"_bs }); // ÷ 200D ÷ 231A ÷	
    check_breaks("\x20\x0D\x03\x08\x23\x1A"_bs, { "\x20\x0D\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 200D × 0308 ÷ 231A ÷	
    check_breaks("\x20\x0D\x03\x00"_bs, { "\x20\x0D\x03\x00"_bs }); // ÷ 200D × 0300 ÷	
    check_breaks("\x20\x0D\x03\x08\x03\x00"_bs, { "\x20\x0D\x03\x08\x03\x00"_bs }); // ÷ 200D × 0308 × 0300 ÷	
    check_breaks("\x20\x0D\x20\x0D"_bs, { "\x20\x0D\x20\x0D"_bs }); // ÷ 200D × 200D ÷	
    check_breaks("\x20\x0D\x03\x08\x20\x0D"_bs, { "\x20\x0D\x03\x08\x20\x0D"_bs }); // ÷ 200D × 0308 × 200D ÷	
    check_breaks("\x20\x0D\x03\x78"_bs, { "\x20\x0D"_bs, "\x03\x78"_bs }); // ÷ 200D ÷ 0378 ÷	
    check_breaks("\x20\x0D\x03\x08\x03\x78"_bs, { "\x20\x0D\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 200D × 0308 ÷ 0378 ÷	
    check_breaks("\x03\x78\x00\x20"_bs, { "\x03\x78"_bs, "\x00\x20"_bs }); // ÷ 0378 ÷ 0020 ÷	
    check_breaks("\x03\x78\x03\x08\x00\x20"_bs, { "\x03\x78\x03\x08"_bs, "\x00\x20"_bs }); // ÷ 0378 × 0308 ÷ 0020 ÷	
    check_breaks("\x03\x78\x00\x0D"_bs, { "\x03\x78"_bs, "\x00\x0D"_bs }); // ÷ 0378 ÷ 000D ÷	
    check_breaks("\x03\x78\x03\x08\x00\x0D"_bs, { "\x03\x78\x03\x08"_bs, "\x00\x0D"_bs }); // ÷ 0378 × 0308 ÷ 000D ÷	
    check_breaks("\x03\x78\x00\x0A"_bs, { "\x03\x78"_bs, "\x00\x0A"_bs }); // ÷ 0378 ÷ 000A ÷	
    check_breaks("\x03\x78\x03\x08\x00\x0A"_bs, { "\x03\x78\x03\x08"_bs, "\x00\x0A"_bs }); // ÷ 0378 × 0308 ÷ 000A ÷	
    check_breaks("\x03\x78\x00\x01"_bs, { "\x03\x78"_bs, "\x00\x01"_bs }); // ÷ 0378 ÷ 0001 ÷	
    check_breaks("\x03\x78\x03\x08\x00\x01"_bs, { "\x03\x78\x03\x08"_bs, "\x00\x01"_bs }); // ÷ 0378 × 0308 ÷ 0001 ÷	
    check_breaks("\x03\x78\x03\x4F"_bs, { "\x03\x78\x03\x4F"_bs }); // ÷ 0378 × 034F ÷	
    check_breaks("\x03\x78\x03\x08\x03\x4F"_bs, { "\x03\x78\x03\x08\x03\x4F"_bs }); // ÷ 0378 × 0308 × 034F ÷	
    check_breaks("\x03\x78\xD8\x3C\xDD\xE6"_bs, { "\x03\x78"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0378 ÷ 1F1E6 ÷	
    check_breaks("\x03\x78\x03\x08\xD8\x3C\xDD\xE6"_bs, { "\x03\x78\x03\x08"_bs, "\xD8\x3C\xDD\xE6"_bs }); // ÷ 0378 × 0308 ÷ 1F1E6 ÷	
    check_breaks("\x03\x78\x06\x00"_bs, { "\x03\x78"_bs, "\x06\x00"_bs }); // ÷ 0378 ÷ 0600 ÷	
    check_breaks("\x03\x78\x03\x08\x06\x00"_bs, { "\x03\x78\x03\x08"_bs, "\x06\x00"_bs }); // ÷ 0378 × 0308 ÷ 0600 ÷	
    check_breaks("\x03\x78\x09\x03"_bs, { "\x03\x78\x09\x03"_bs }); // ÷ 0378 × 0903 ÷	
    check_breaks("\x03\x78\x03\x08\x09\x03"_bs, { "\x03\x78\x03\x08\x09\x03"_bs }); // ÷ 0378 × 0308 × 0903 ÷	
    check_breaks("\x03\x78\x11\x00"_bs, { "\x03\x78"_bs, "\x11\x00"_bs }); // ÷ 0378 ÷ 1100 ÷	
    check_breaks("\x03\x78\x03\x08\x11\x00"_bs, { "\x03\x78\x03\x08"_bs, "\x11\x00"_bs }); // ÷ 0378 × 0308 ÷ 1100 ÷	
    check_breaks("\x03\x78\x11\x60"_bs, { "\x03\x78"_bs, "\x11\x60"_bs }); // ÷ 0378 ÷ 1160 ÷	
    check_breaks("\x03\x78\x03\x08\x11\x60"_bs, { "\x03\x78\x03\x08"_bs, "\x11\x60"_bs }); // ÷ 0378 × 0308 ÷ 1160 ÷	
    check_breaks("\x03\x78\x11\xA8"_bs, { "\x03\x78"_bs, "\x11\xA8"_bs }); // ÷ 0378 ÷ 11A8 ÷	
    check_breaks("\x03\x78\x03\x08\x11\xA8"_bs, { "\x03\x78\x03\x08"_bs, "\x11\xA8"_bs }); // ÷ 0378 × 0308 ÷ 11A8 ÷	
    check_breaks("\x03\x78\xAC\x00"_bs, { "\x03\x78"_bs, "\xAC\x00"_bs }); // ÷ 0378 ÷ AC00 ÷	
    check_breaks("\x03\x78\x03\x08\xAC\x00"_bs, { "\x03\x78\x03\x08"_bs, "\xAC\x00"_bs }); // ÷ 0378 × 0308 ÷ AC00 ÷	
    check_breaks("\x03\x78\xAC\x01"_bs, { "\x03\x78"_bs, "\xAC\x01"_bs }); // ÷ 0378 ÷ AC01 ÷	
    check_breaks("\x03\x78\x03\x08\xAC\x01"_bs, { "\x03\x78\x03\x08"_bs, "\xAC\x01"_bs }); // ÷ 0378 × 0308 ÷ AC01 ÷	
    check_breaks("\x03\x78\x23\x1A"_bs, { "\x03\x78"_bs, "\x23\x1A"_bs }); // ÷ 0378 ÷ 231A ÷	
    check_breaks("\x03\x78\x03\x08\x23\x1A"_bs, { "\x03\x78\x03\x08"_bs, "\x23\x1A"_bs }); // ÷ 0378 × 0308 ÷ 231A ÷	
    check_breaks("\x03\x78\x03\x00"_bs, { "\x03\x78\x03\x00"_bs }); // ÷ 0378 × 0300 ÷	
    check_breaks("\x03\x78\x03\x08\x03\x00"_bs, { "\x03\x78\x03\x08\x03\x00"_bs }); // ÷ 0378 × 0308 × 0300 ÷	
    check_breaks("\x03\x78\x20\x0D"_bs, { "\x03\x78\x20\x0D"_bs }); // ÷ 0378 × 200D ÷	
    check_breaks("\x03\x78\x03\x08\x20\x0D"_bs, { "\x03\x78\x03\x08\x20\x0D"_bs }); // ÷ 0378 × 0308 × 200D ÷	
    check_breaks("\x03\x78\x03\x78"_bs, { "\x03\x78"_bs, "\x03\x78"_bs }); // ÷ 0378 ÷ 0378 ÷	
    check_breaks("\x03\x78\x03\x08\x03\x78"_bs, { "\x03\x78\x03\x08"_bs, "\x03\x78"_bs }); // ÷ 0378 × 0308 ÷ 0378 ÷	
    check_breaks("\x00\x0D\x00\x0A\x00\x61\x00\x0A\x03\x08"_bs, { "\x00\x0D\x00\x0A"_bs, "\x00\x61"_bs, "\x00\x0A"_bs, "\x03\x08"_bs }); // ÷ 000D × 000A ÷ 0061 ÷ 000A ÷ 0308 ÷	
    check_breaks("\x00\x61\x03\x08"_bs, { "\x00\x61\x03\x08"_bs }); // ÷ 0061 × 0308 ÷	
    check_breaks("\x00\x20\x20\x0D\x06\x46"_bs, { "\x00\x20\x20\x0D"_bs, "\x06\x46"_bs }); // ÷ 0020 × 200D ÷ 0646 ÷	
    check_breaks("\x06\x46\x20\x0D\x00\x20"_bs, { "\x06\x46\x20\x0D"_bs, "\x00\x20"_bs }); // ÷ 0646 × 200D ÷ 0020 ÷	
    check_breaks("\x11\x00\x11\x00"_bs, { "\x11\x00\x11\x00"_bs }); // ÷ 1100 × 1100 ÷	
    check_breaks("\xAC\x00\x11\xA8\x11\x00"_bs, { "\xAC\x00\x11\xA8"_bs, "\x11\x00"_bs }); // ÷ AC00 × 11A8 ÷ 1100 ÷	
    check_breaks("\xAC\x01\x11\xA8\x11\x00"_bs, { "\xAC\x01\x11\xA8"_bs, "\x11\x00"_bs }); // ÷ AC01 × 11A8 ÷ 1100 ÷	
    check_breaks("\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7\xD8\x3C\xDD\xE8\x00\x62"_bs, { "\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7"_bs, "\xD8\x3C\xDD\xE8"_bs, "\x00\x62"_bs }); // ÷ 1F1E6 × 1F1E7 ÷ 1F1E8 ÷ 0062 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7\xD8\x3C\xDD\xE8\x00\x62"_bs, { "\x00\x61"_bs, "\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7"_bs, "\xD8\x3C\xDD\xE8"_bs, "\x00\x62"_bs }); // ÷ 0061 ÷ 1F1E6 × 1F1E7 ÷ 1F1E8 ÷ 0062 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7\x20\x0D\xD8\x3C\xDD\xE8\x00\x62"_bs, { "\x00\x61"_bs, "\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7\x20\x0D"_bs, "\xD8\x3C\xDD\xE8"_bs, "\x00\x62"_bs }); // ÷ 0061 ÷ 1F1E6 × 1F1E7 × 200D ÷ 1F1E8 ÷ 0062 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDD\xE6\x20\x0D\xD8\x3C\xDD\xE7\xD8\x3C\xDD\xE8\x00\x62"_bs, { "\x00\x61"_bs, "\xD8\x3C\xDD\xE6\x20\x0D"_bs, "\xD8\x3C\xDD\xE7\xD8\x3C\xDD\xE8"_bs, "\x00\x62"_bs }); // ÷ 0061 ÷ 1F1E6 × 200D ÷ 1F1E7 × 1F1E8 ÷ 0062 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7\xD8\x3C\xDD\xE8\xD8\x3C\xDD\xE9\x00\x62"_bs, { "\x00\x61"_bs, "\xD8\x3C\xDD\xE6\xD8\x3C\xDD\xE7"_bs, "\xD8\x3C\xDD\xE8\xD8\x3C\xDD\xE9"_bs, "\x00\x62"_bs }); // ÷ 0061 ÷ 1F1E6 × 1F1E7 ÷ 1F1E8 × 1F1E9 ÷ 0062 ÷	
    check_breaks("\x00\x61\x20\x0D"_bs, { "\x00\x61\x20\x0D"_bs }); // ÷ 0061 × 200D ÷	
    check_breaks("\x00\x61\x03\x08\x00\x62"_bs, { "\x00\x61\x03\x08"_bs, "\x00\x62"_bs }); // ÷ 0061 × 0308 ÷ 0062 ÷	
    check_breaks("\x00\x61\x09\x03\x00\x62"_bs, { "\x00\x61\x09\x03"_bs, "\x00\x62"_bs }); // ÷ 0061 × 0903 ÷ 0062 ÷	
    check_breaks("\x00\x61\x06\x00\x00\x62"_bs, { "\x00\x61"_bs, "\x06\x00\x00\x62"_bs }); // ÷ 0061 ÷ 0600 × 0062 ÷	
    check_breaks("\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF\xD8\x3D\xDC\x76"_bs, { "\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF"_bs, "\xD8\x3D\xDC\x76"_bs }); // ÷ 1F476 × 1F3FF ÷ 1F476 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDF\xFF\xD8\x3D\xDC\x76"_bs, { "\x00\x61\xD8\x3C\xDF\xFF"_bs, "\xD8\x3D\xDC\x76"_bs }); // ÷ 0061 × 1F3FF ÷ 1F476 ÷	
    check_breaks("\x00\x61\xD8\x3C\xDF\xFF\xD8\x3D\xDC\x76\x20\x0D\xD8\x3D\xDE\xD1"_bs, { "\x00\x61\xD8\x3C\xDF\xFF"_bs, "\xD8\x3D\xDC\x76\x20\x0D\xD8\x3D\xDE\xD1"_bs }); // ÷ 0061 × 1F3FF ÷ 1F476 × 200D × 1F6D1 ÷	
    check_breaks("\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF\x03\x08\x20\x0D\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF"_bs, { "\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF\x03\x08\x20\x0D\xD8\x3D\xDC\x76\xD8\x3C\xDF\xFF"_bs }); // ÷ 1F476 × 1F3FF × 0308 × 200D × 1F476 × 1F3FF ÷	
    check_breaks("\xD8\x3D\xDE\xD1\x20\x0D\xD8\x3D\xDE\xD1"_bs, { "\xD8\x3D\xDE\xD1\x20\x0D\xD8\x3D\xDE\xD1"_bs }); // ÷ 1F6D1 × 200D × 1F6D1 ÷	
    check_breaks("\x00\x61\x20\x0D\xD8\x3D\xDE\xD1"_bs, { "\x00\x61\x20\x0D"_bs, "\xD8\x3D\xDE\xD1"_bs }); // ÷ 0061 × 200D ÷ 1F6D1 ÷	
    check_breaks("\x27\x01\x20\x0D\x27\x01"_bs, { "\x27\x01\x20\x0D\x27\x01"_bs }); // ÷ 2701 × 200D × 2701 ÷	
    check_breaks("\x00\x61\x20\x0D\x27\x01"_bs, { "\x00\x61\x20\x0D"_bs, "\x27\x01"_bs }); // ÷ 0061 × 200D ÷ 2701 ÷	
}

TEST(DataforgeTest, graheme_break)
{
    grapheme_break_test();
}

void icu_test()
{
    const char16_t utf16text[] = {
        0x0065, 0x006e, 0x0067, 0x0069, 0x0073, 0x0068, 0x002b, 0x0440,
        0x0443, 0x0441,	0x0441, 0x043a, 0x0438, 0x0439
    };

    DATAFORGE_TEST(utf8 | utf16 | (icu_qrk<icu_basic_escaper_handler<>, 1>("866")), span_cast<const char8_t>("\x0d\x0aп\xf0\x9f\x91\x8c\xf0\x9f\x8f\xbfРИВЕТ"_sp), "\xd\xa\xaf\\U0001f44c\\U0001f3ff\x90\x88\x82\x85\x92"_bs);
    DATAFORGE_TEST(utf16 | (icu_qrk<void, 32, false>("windows-1251")), utf16text, "\x65\x6e\x67\x69\x73\x68\x2b\xf0\xf3\xf1\xf1\xea\xe8\xe9"_bs);
    DATAFORGE_TEST(base16l | int8/be | utf16 | icu_qrk<void>("866"), "0065006e0067006900730068002b0440044304410441043a04380439"_bs, "\x65\x6e\x67\x69\x73\x68\x2b\xe0\xe3\xe1\xe1\xaa\xa8\xa9"_bs);

    DATAFORGE_TEST((icu_qrk<icu_basic_escaper_handler<>, 1>("866")) | utf16 | utf8, "\xd\xa\xaf\x90\x88\x82\x85\x92"_sp, span_cast<const char8_t>("\x0d\x0aпРИВЕТ"_sp));
    DATAFORGE_TEST((icu_qrk<icu_basic_escaper_handler<>, 1>("UTF8")) | utf16 | utf8, "\xff\xf0\x9f\x91\x01\x0d\x0aпРИВЕТ"_sp, span_cast<const char8_t>("\\xff\\xf0\\x9f\\x91\x01\x0d\x0aпРИВЕТ"_sp));
}

TEST(DataforgeTest, icu)
{
    icu_test();
}

}
