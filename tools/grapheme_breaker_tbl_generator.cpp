/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#include <fstream>
#include <iostream>
#include <string>
#include <iterator>
#include <vector>
#include <map>
#include <algorithm>

#include "dataforge/unicode/utf.hpp"
#include "dataforge/base_xx/base16.hpp"
#include "dataforge/basic/group.hpp"
#include "dataforge/quark_push_iterator.hpp"

namespace dataforge {
void graheme_break_test();
}

static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

template <typename CharT>
uint8_t get_hexdigit(CharT c) noexcept
{
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    else if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a' + 10);
    else if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A' + 10);
    else return 0xff;
}

template <typename IteratorT, typename IntegerT>
bool hexinteger(IteratorT& b, IteratorT const& e, unsigned int mindigits, unsigned int maxdigits, IntegerT& result) noexcept
{
    IteratorT pos = b;
    using char_type = typename std::iterator_traits<IteratorT>::value_type;
    result = 0;
    unsigned int dc = 0;
    for (; pos != e && dc < maxdigits; ++pos, ++dc) {
        const char_type c0 = *pos;
        uint8_t v = get_hexdigit(c0);
        if (v != 0xff) {
            result = result * 16 + v;
        }
        else {
            break;
        }
    }
    if (dc < mindigits) {
        return false;
    }
    b = pos;
    return true;
}

template <typename IteratorT>
bool try_read(IteratorT & it, IteratorT eit, char s, std::string const& errormsg)
{
    if (it == eit) throw std::runtime_error("unexpected eol, " + errormsg);
    if (*it == s) {
        ++it;
        return true;
    }
    return false;
}

template <typename IteratorT>
void read_assert(IteratorT& it, IteratorT eit, char s, std::string const& errormsg)
{
    if (!try_read(it, eit, s, errormsg)) {
        throw std::runtime_error(std::string("unexpected char, expected: ") + s + " at: " + errormsg);
    }
}

std::vector<std::tuple<uint_least32_t, uint_least32_t, int>>
parse_file(const char* fpath, std::map<std::string, int> const& prop_map)
{
    std::ifstream propss;
    propss.open(fpath);
    if (!propss.is_open()) {
        throw std::runtime_error("can't open input file");
    }

    std::vector<std::tuple<uint_least32_t, uint_least32_t, int>> cpmap;

    for (std::string line; !propss.eof() && getline(propss, line); )
    {
        ltrim(line);
        if (line.empty() || line.starts_with("#")) continue;
        auto it = line.cbegin(), eit = line.cend();
        int_least32_t s, e = -1;
        if (!hexinteger(it, eit, 4, 5, s)) throw std::runtime_error("can't parse start point: " + line);
        if (try_read(it, eit, '.', line)) {
            read_assert(it, eit, '.', line);
            if (!hexinteger(it, eit, 4, 5, e)) throw std::runtime_error("can't parse end point: " + line);
        }
        for (; it != eit && std::isspace(*it); ++it);
        read_assert(it, eit, ';', line);
        for (; it != eit && std::isspace(*it); ++it);
        auto propit = it;
        for (; propit != eit && !std::isspace(*propit) && *propit != '#'; ++propit);
        std::string prop(it, propit);
        //std::cout << line << "\n";
        //std::cout << line << "\n" << s << " .. " << e << " " << prop << "\n";

        auto pmit = prop_map.find(prop);
        if (pmit == prop_map.end()) throw std::runtime_error("unexpected code point property: " + prop + " in the line: " + line);

        cpmap.push_back(std::tuple{ s, e > 0 ? e : s, pmit->second });
    }
    std::sort(cpmap.begin(), cpmap.end(), [](auto const& l, auto const& r) {
        return std::get<0>(l) < std::get<0>(r);
    });

    return std::move(cpmap);
}

enum class emoji_property
{
    Any = 0, Emoji, Emoji_Presentation, Emoji_Modifier, Emoji_Modifier_Base, Emoji_Component, Extended_Pictographic
};

#define CP_EPAIR(n) {#n, (int)emoji_property::n}
std::vector<std::tuple<uint_least32_t, uint_least32_t>> parse_emoji_Extended_Pictographic()
{
    std::map<std::string, int> prop_map = {
        CP_EPAIR(Emoji), CP_EPAIR(Emoji_Presentation), CP_EPAIR(Emoji_Modifier), CP_EPAIR(Emoji_Modifier_Base), CP_EPAIR(Emoji_Component), CP_EPAIR(Extended_Pictographic) };

    std::vector<std::tuple<uint_least32_t, uint_least32_t, int>> cpmap = parse_file("../../resources/emoji-data.txt", prop_map);
    // filter Extended_Pictographic
    std::vector<std::tuple<uint_least32_t, uint_least32_t>> result;
    for (auto const& tpl : cpmap) {
        if (std::get<2>(tpl) == (int)emoji_property::Extended_Pictographic) {
            result.emplace_back(std::get<0>(tpl), std::get<1>(tpl));
        }
    }
    return result;
}

enum class code_point_property
{
    Any = 0, CR, LF, Control, Extend, ZWJ, Regional_Indicator, Prepend, SpacingMark, L, V, T, LV, LVT, Extended_Pictographic
};
#define CP_PAIR(n) {#n, (int)code_point_property::n}
std::vector<std::tuple<uint_least32_t, code_point_property>>
parse_break_property()
{
    std::map<std::string, int> prop_map = {
        CP_PAIR(CR), CP_PAIR(LF), CP_PAIR(Control), CP_PAIR(Extend), CP_PAIR(Extend), CP_PAIR(ZWJ), CP_PAIR(Regional_Indicator), CP_PAIR(Prepend), CP_PAIR(SpacingMark),
        CP_PAIR(L), CP_PAIR(V), CP_PAIR(T), CP_PAIR(LV), CP_PAIR(LVT), CP_PAIR(Extended_Pictographic) };

    std::vector<std::tuple<uint_least32_t, uint_least32_t, int>> cpmap = parse_file("../../resources/GraphemeBreakProperty.txt", prop_map);

    std::vector<std::tuple<uint_least32_t, uint_least32_t>> eps = parse_emoji_Extended_Pictographic();

    // append Extended_Pictographic
    for (auto const& tpl : eps) {
        cpmap.emplace_back(std::get<0>(tpl), std::get<1>(tpl), (int)code_point_property::Extended_Pictographic);
    }
    // sort again
    std::sort(cpmap.begin(), cpmap.end(), [](auto const& l, auto const& r) {
        return std::get<0>(l) < std::get<0>(r);
    });

    // create code point map for lower bound search, add 'Any' intervals
    std::vector<std::tuple<uint_least32_t, code_point_property>> cp_lb_map;
    size_t maxidx = cpmap.size();
    uint_least32_t prev = 0;
    //code_point_property prevprop = code_point_property::Any;
    for (size_t i = 0; i < maxidx; ++i) {
        auto const& tpl = cpmap[i];
        if (std::get<0>(tpl) < prev) {
            throw std::runtime_error("range intersection found");
        }
        int_least32_t dist = std::get<0>(tpl) - prev;
        if (dist > 1) {
            cp_lb_map.push_back(std::tuple{ (uint_least32_t)(std::get<0>(tpl) - 1), code_point_property::Any });
        } else if (dist == 1) {
            cp_lb_map.push_back(std::tuple{ prev, code_point_property::Any });
        } else if (!cp_lb_map.empty() && std::get<1>(cp_lb_map.back()) == (code_point_property)std::get<2>(tpl)) { // dist == 0
            std::get<0>(cp_lb_map.back()) = std::get<1>(tpl);
            prev = std::get<1>(tpl) + 1;
            continue;
        }
        cp_lb_map.push_back(std::tuple{ (uint_least32_t)std::get<1>(tpl), (code_point_property)std::get<2>(tpl) });
        prev = std::get<1>(tpl) + 1;
    }

    return cp_lb_map;
}

void parse_grapheme_break_test()
{
    std::ifstream propss;
    propss.open("../../resources/GraphemeBreakTest.txt");
    if (!propss.is_open()) {
        throw std::runtime_error("can't open input file");
    }

    std::vector<std::pair<std::vector<std::vector<int_least32_t>>, std::string>> tests;

    for (std::string line; !propss.eof() && getline(propss, line); )
    {
        ltrim(line);
        if (line.empty() || line.starts_with("#")) continue;
        auto it = line.cbegin(), eit = line.cend();
        for (; it != eit && (*it == '\xc3' || *it == '\xb7' || std::isspace(*it)); ++it); // ÷
        if (it == eit || *it == '#') continue;

        std::vector<int_least32_t> acc;
        std::vector<std::vector<int_least32_t>> gs; // graphemes

        while (it != eit) {
            int_least32_t c;
            if (!hexinteger(it, eit, 4, 5, c)) throw std::runtime_error("can't parse start point: " + line);
            acc.push_back(c);

            for (; it != eit && *it != '\xc3' && std::isspace(*it); ++it);
            if (it == eit || *it == '#') break;
            read_assert(it, eit, '\xc3', "unexpected char"); // '×' or '×'
            
            if (try_read(it, eit, '\xb7', "")) { // '÷'
                gs.push_back(std::move(acc));
                acc.clear();
            } else {
                read_assert(it, eit, '\x97', "unexpected char"); // '×'
            }
            for (; it != eit && std::isspace(*it); ++it);
            if (it != eit && *it == '#') break;
        }
        if (!acc.empty()) {
            gs.push_back(std::move(acc));
        }
        tests.emplace_back(std::move(gs), std::string(line.cbegin(), it));
    }

    using namespace dataforge;
    std::vector<char> result;
    //result.append_range("inline const uint_least32_t grapheme_breaks_tbl0[] = {\n    "_bs);
    auto cvt_it = quark_push_iterator{ utf32 | utf16 | be / int8 | base16u | byte_group(2, "\\x"_bs, ""_bs, ""_bs), std::back_inserter(result) };

    for (auto const& t : tests) {
        result.append_range("check_breaks(\""_bs);
        for (auto const& g : t.first) {
            *cvt_it = g; ++cvt_it;
        }
        cvt_it.finish();
        result.append_range("\"_bs, {"_bs);
        for (auto const& g : t.first) {
            result.append_range("\""_bs);
            *cvt_it = g; ++cvt_it;
            result.append_range("\"_bs"_bs);
            if (&g != &t.first.back()) {
                result.append_range(", "_bs);
            }
        }
        result.append_range("}); // "_bs);
        result.append_range(t.second);
        result.push_back('\n');
    }
    result.push_back(0);
    //std::cout << result.data() << "\n\n";

    std::ofstream rfs("result.cpp", std::ios::binary);
    std::copy(result.begin(), result.end(), std::ostream_iterator<char>(rfs));
}

int main()
{
    using namespace dataforge;

    try {
        //parse_grapheme_break_test();

        std::vector<std::tuple<uint_least32_t, code_point_property>> cp_lb_map = parse_break_property();

#if 0
        std::vector<std::tuple<uint_least32_t, uint_least32_t>> eps = parse_emoji_Extended_Pictographic();

        // check for no intersections
        for (auto const& tpl : eps) {
            for (uint_least32_t b = std::get<0>(tpl), e = std::get<1>(tpl); b <= e; ++b) {
                auto it = std::lower_bound(cp_lb_map.begin(), cp_lb_map.end(), b, [](auto const& tpl, uint_least32_t r) { return std::get<0>(tpl) < r; });
                if (it == cp_lb_map.end() || std::get<1>(*it) == code_point_property::Any) continue;
                std::ostringstream ss;
                ss << "found cp: " << std::hex << b << ", property: " << (int)std::get<1>(*it);
                throw std::runtime_error(ss.str());
            }
        }
#endif
        /*
        for (auto const& e : cp_lb_map) {
            std::cout << std::hex << std::get<0>(e) << " " << (int)std::get<1>(e) << "\n";
        }
        */
#if 1
        // dump leaf table
        

        std::vector<char> result;
        

        std::vector<uint_least32_t> cp_lb_bounds, cp_lb_bounds1, cp_lb_bounds2;
        cp_lb_bounds.reserve(cp_lb_map.size());
        cp_lb_bounds1.reserve(cp_lb_map.size() / 16);
        cp_lb_bounds2.reserve(cp_lb_map.size() / 256);

#if 0 // just bounds
        std::transform(cp_lb_map.begin(), cp_lb_map.end(), std::back_inserter(cp_lb_bounds), [](auto const& v) { return std::get<0>(v); });
#else // bounds with types
        std::transform(cp_lb_map.begin(), cp_lb_map.end(), std::back_inserter(cp_lb_bounds), [](auto const& v) {
            //return std::get<0>(v) | (((uint_least32_t)std::get<1>(v)) << 24);
            return std::get<0>(v) << 8 | ((uint_least32_t)std::get<1>(v));
        });
        for (size_t i = 15; i < cp_lb_bounds.size(); i+=16) {
            cp_lb_bounds1.push_back(cp_lb_bounds[i]);
        }
        for (size_t i = 15; i < cp_lb_bounds1.size(); i += 16) {
            cp_lb_bounds2.push_back(cp_lb_bounds1[i]);
        }
#endif
        auto cvt_it = quark_push_iterator{ int32 | be / int8 | base16u | byte_group(8, "0x"_bs, ""_bs, ", "_bs, 16, "\n    "_bs), std::back_inserter(result) };
        result.append_range("alignas(64) inline const uint_least32_t grapheme_breaks_tbl0[] = {\n    "_bs);
        cvt_it << cp_lb_bounds;
        cvt_it.finish();
        result.append_range("\n};\n\n"_bs);
        result.append_range("alignas(64) inline const uint_least32_t grapheme_breaks_tbl1[] = {\n    "_bs);
        cvt_it << cp_lb_bounds1;
        cvt_it.finish();
        result.append_range("\n};\n\n"_bs);
        result.append_range("alignas(64) inline const uint_least32_t grapheme_breaks_tbl2[] = {\n    "_bs);
        cvt_it << cp_lb_bounds2;
        cvt_it.finish();
        result.append_range("\n};\n\n"_bs);
#if 0
        result.append_range("inline const uint_least8_t grapheme_types_tbl0[] = {\n    "_bs);
        // dump leaf type table
        std::vector<uint_least8_t> cp_lb_types;
        cp_lb_types.reserve(cp_lb_map.size());
        std::transform(cp_lb_map.begin(), cp_lb_map.end(), std::back_inserter(cp_lb_types), [](auto const& v) { return (uint_least8_t)std::get<1>(v); });

        auto cvt_tp_it = quark_push_iterator{ int8 | base16u | byte_group(2, "0x"_bs, ""_bs, ", "_bs, 8, "\n    "_bs), std::back_inserter(result) };
        for (int i = 0; i < cp_lb_types.size(); i += 2) {
            uint_least8_t v = cp_lb_types[i];
            if (i + 1 < cp_lb_types.size()) {
                v |= (cp_lb_types[i + 1] << 4);
            }
            *cvt_tp_it = v;
            ++cvt_tp_it;
        }
        cvt_tp_it.finish();
        result.append_range("\n};"_bs);
#endif
        result.push_back(0);
        std::cout << result.data() << "\n";

        std::cout << "entries: " << cp_lb_map.size() << "\n";


#endif
    }
    catch (std::exception const& e) {
        std::cout << e.what();
    }
    
    graheme_break_test();
}
