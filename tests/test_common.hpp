/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <string>
#include <vector>
#include <utility>
#include <iterator>
#include <iostream>
#include <span>

#include <gtest/gtest.h>

#ifdef ADDITIONAL_TEST_HEADER
#   include ADDITIONAL_TEST_HEADER
#endif

#ifndef DO_NOT_HAVE_RANGES
#   include <ranges>
#endif

#include "dataforge/quark_push_iterator.hpp"
#include "dataforge/quark_pull_iterator.hpp"

template <typename RangeT1, typename RangeT2>
bool equal_to(RangeT1 const& l, RangeT2 const& r)
{
    if (std::size(l) != std::size(r)) return false;
    return std::equal(std::begin(l), std::end(l), std::begin(r));
}

template <typename ConverterChainT, typename IRangeT>
void conv_push_exception_test(ConverterChainT enc, IRangeT&& input, std::string_view expstr, std::string const& descr = "")
{
    bool has_exception = true;
    try {
        using namespace dataforge;

        using cvt_tuple_type = typename ConverterChainT::cvt_tuple_type;
        using output_element_type = typename std::tuple_element_t<std::tuple_size_v<cvt_tuple_type> - 1, cvt_tuple_type>::output_element_type;

        std::vector<output_element_type> result;
        quark_push_iterator cvt_it{ std::move(enc), std::back_inserter(result) };

        using input_element_type = typename decltype(cvt_it)::converter_type::input_element_type;
        last_provider<input_element_type, std::remove_cvref_t<IRangeT>> provider{std::forward<IRangeT>(input)};
        for (auto sp = provider(); !sp.empty(); cvt_it << sp, sp = provider());

        cvt_it.finish();
        has_exception = false;
    } catch (std::exception const& e) {
        has_exception = std::string_view(e.what()).starts_with(expstr);
        EXPECT_TRUE(has_exception) << "ERROR in PUSH TEST(" << descr << "), wrong exception: '" << e.what() << "' instead of '" << expstr << "'";
    }
    EXPECT_TRUE(has_exception) << "ERROR in PUSH TEST(" << descr << "): the exception '" << expstr << "' was expected";
}

template <typename ConverterChainT, typename IRangeT>
void conv_pull_exception_test(ConverterChainT enc, IRangeT&& input, std::string_view expstr, std::string const& descr = "")
{
    bool has_exception = true;
    try {
        using namespace dataforge;
        quark_pull_iterator cvt_it{ std::move(enc), std::move(input) };
        *cvt_it;
        has_exception = false;
    } catch (std::exception const& e) {
        has_exception = std::string_view(e.what()).starts_with(expstr);
        EXPECT_TRUE(has_exception) << "ERROR in PULL TEST(" << descr << "), wrong exception: '" << e.what() << "' instead of '" << expstr << "'";
    }
    EXPECT_TRUE(has_exception) << "ERROR in PULL TEST(" << descr << "): the exception '" << expstr << "' was expected";
}

#define CONV_PUSH_EXCEPTION_TEST(conv, rng, exp) conv_push_exception_test(conv, rng, exp, #conv)
#define CONV_PULL_EXCEPTION_TEST(conv, rng, exp) conv_pull_exception_test(conv, rng, exp, #conv)
#define CONV_EXCEPTION_TEST(conv, rng, exp) conv_push_exception_test(conv, rng, exp, #conv); conv_pull_exception_test(conv, rng, exp, #conv)

template <typename ConverterChainT, typename IRangeT, typename RRangeT>
void dataforge_push_test(ConverterChainT enc, IRangeT && input, RRangeT expected_result, std::string const& descr = "")
{
    try {
        using namespace dataforge;

        using rit_t = std::ranges::iterator_t<RRangeT>;
        using r_element_type = typename std::iterator_traits<rit_t>::value_type;
        std::vector<r_element_type> result;

        quark_push_iterator cvt_it{std::move(enc), std::back_inserter(result)};

        using input_element_type = typename decltype(cvt_it)::converter_type::input_element_type;
        last_provider<input_element_type, std::remove_cvref_t<IRangeT>> provider{std::forward<IRangeT>(input)};
        for (auto sp = provider(); !sp.empty(); cvt_it << sp, sp = provider());
        
        cvt_it.finish();

        bool test_result = equal_to(result, expected_result);
        if (!test_result) {
            EXPECT_TRUE(test_result) << "ERROR in PUSH TEST (" << descr << ") : ranges are different";
        }
    } catch (std::exception const& e) {
        GTEST_FAIL() << "ERROR in (" << descr << ") : " << e.what();
    }
}

template <typename ConverterChainT, typename IRangeT, typename RRangeT>
void dataforge_pull_test(ConverterChainT enc, IRangeT && input, RRangeT expected_result, std::string const& descr = "")
{
    try {
        using namespace dataforge;

        using rit_t = std::ranges::iterator_t<RRangeT>;
        using r_element_type = typename std::iterator_traits<rit_t>::value_type;
        std::vector<r_element_type> result;

        auto cvt_it = quark_pull_iterator{ std::move(enc), input };
        for (auto sp = *cvt_it; !sp.empty(); sp = *cvt_it) {
            result.insert(result.end(), sp.begin(), sp.end());
            ++cvt_it;
        }
        cvt_it.reset(input); // just to test 'reset'

        bool test_result = equal_to(result, expected_result);
        if (!test_result) {
            EXPECT_TRUE(test_result) << "ERROR in PULL TEST (" << descr << ") : ranges are different";
        }
    } catch (std::exception const& e) {
        GTEST_FAIL() << "ERROR in (" << descr << ") : " << e.what();
    }
}

#define DATAFORGE_PUSH_TEST(conv, rng, exp) dataforge_push_test(conv, rng, exp, #conv)
#define DATAFORGE_PULL_TEST(conv, rng, exp) dataforge_pull_test(conv, rng, exp, #conv)
#define DATAFORGE_TEST(conv, rng, exp) dataforge_push_test(conv, rng, exp, #conv); dataforge_pull_test(conv, rng, exp, #conv)

template <typename ConverterT, typename TestSetT>
void conv_push_test_set(ConverterT enc, TestSetT const& ts, std::string const& descr = "")
{
    for (size_t i = 0; i < sizeof(ts.testtext) / sizeof(char*); ++i)
    {
        std::string in{ ts.testtext[i] };
        std::string check{ ts.expresults[i] };
        dataforge_push_test(std::move(enc), in, std::span{ check }, descr);
    }
}

template <typename ConverterT, typename TestSetT>
void conv_pull_test_set(ConverterT enc, TestSetT const& ts, std::string const& descr = "")
{
    for (size_t i = 0; i < sizeof(ts.testtext) / sizeof(char*); ++i)
    {
        std::string in{ ts.testtext[i] };
        std::string check{ ts.expresults[i] };
        dataforge_pull_test(std::move(enc), in, std::span{ check }, descr);
    }
}

#define DATAFORGE_PUSH_TEST_SET(conv, ts) conv_push_test_set(conv, ts, #conv)
#define DATAFORGE_PULL_TEST_SET(conv, ts) conv_pull_test_set(conv, ts, #conv)
#define DATAFORGE_TEST_SET(conv, ts) conv_push_test_set(conv, ts, #conv); conv_pull_test_set(conv, ts, #conv)

namespace dataforge {

void generic_base_test();
void base16_test();
void base32_test();
void base58_test();
void base64_test();
void ascii85_test();
void z85_test();

void utf_test();
void grapheme_break_test();
void icu_test();

void bsd_checksum_test();
void adler32_test();
void crc_test();

void md2_test();
void md4_test();
void md5_test();
void md6_test();
void ripemd_test();
void sha1_test();
void sha2_test();
void sha3_test();
void tiger_test();
void gost_test();
void streebog_test();
void belt_hash_test();
void whirlpool_test();
void blake_test();

void blowfish_test();
void rc2_test();
void rc4_test();
void rc5_test();
void rc6_test();
void des_test();
void aes_test();
void belt_test();
void magma_test();
//void kuznyechik_test();

void group_test();
void deflate_test();
void bzip2_test();
void lzma_test();
void lz4_test();

}
