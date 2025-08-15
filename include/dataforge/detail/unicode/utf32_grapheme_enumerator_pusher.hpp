/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <vector>

#include "gbr.hpp"

namespace dataforge {

class utf32_grapheme_enumerator_pusher : public generic_pusher<void>
{
    using base_t = generic_pusher<void>;
    using base_t::on_error;

    size_t grapheme_num_{ 0 };

#if 0 // basic
    static inline unicode_detail::code_point_property get_property(char32_t uchar)
    {
        using namespace unicode_detail;
        auto it = std::lower_bound(grapheme_breaks_tbl0, grapheme_breaks_tbl0 + sizeof(grapheme_breaks_tbl0) / sizeof(uint_least32_t), (uint_least32_t)uchar);
        size_t offs = it - grapheme_breaks_tbl0;
        assert(offs/2 < sizeof(grapheme_types_tbl0));
        return static_cast<code_point_property>(0xf & (grapheme_types_tbl0[offs / 2] >> (4 * (offs & 1))));
    }
#endif
#if 0 // binary search
    static inline unicode_detail::code_point_property get_property(char32_t uchar)
    {
        using namespace unicode_detail;
        uint_least32_t const* b0 = grapheme_breaks_tbl0;
        uint_least32_t const* e0 = grapheme_breaks_tbl0 + sizeof(grapheme_breaks_tbl0) / sizeof(uint_least32_t);
        auto it = std::lower_bound(b0, e0, (uint_least32_t)uchar, [](uint_least32_t l, uint_least32_t r) { return (l << 8) < (r << 8); });
        return it != e0 ? static_cast<code_point_property>((*it >> 24)) : code_point_property::Any;
    }
#endif
#if 1 // 3 levels b+tree search, touches only 1 cache line (64 bytes) per lower_bound search
    static inline unicode_detail::code_point_property get_property(char32_t uchar)
    {
        using namespace unicode_detail;
        uint_least32_t sval = uchar << 8;
        uint_least32_t const* b2 = grapheme_breaks_tbl2;
        uint_least32_t const* e2 = grapheme_breaks_tbl2 + sizeof(grapheme_breaks_tbl2) / sizeof(uint_least32_t);
        auto it2 = std::lower_bound(b2, e2, sval);
        if (it2 != e2 && ((*it2) >> 8) == uchar ) return static_cast<code_point_property>((*it2 & 0xf));
        
        uint_least32_t const* b1 = grapheme_breaks_tbl1 + (it2 - b2) * 16;
        uint_least32_t const* e1 = (std::min)(b1 + 16, grapheme_breaks_tbl1 + sizeof(grapheme_breaks_tbl1) / sizeof(uint_least32_t));
        auto it1 = std::lower_bound(b1, e1, sval);
        if (it1 != e1 && ((*it1) >> 8) == uchar) return static_cast<code_point_property>((*it1 & 0xf));

        uint_least32_t const* b0 = grapheme_breaks_tbl0 + (it2 - b2) * 256 + (it1 - b1) * 16;
        uint_least32_t const* e0 = (std::min)(b0 + 16, grapheme_breaks_tbl0 + sizeof(grapheme_breaks_tbl0) / sizeof(uint_least32_t));
        auto it0 = std::lower_bound(b0, e0, sval);
        return it0 != e0 ? static_cast<code_point_property>((*it0 & 0xf)) : code_point_property::Any;
    }
#endif

    uint_least8_t prev_prop : 4;
    uint_least8_t sot : 1; // start of text
    // flags for a context before prev
    uint_least8_t extended_pictographic_extend_times : 1;
    uint_least8_t odd_number_of_RI : 1;

    bool need_break(unicode_detail::code_point_property prev, unicode_detail::code_point_property next) const
    {
        using namespace unicode_detail;

        // based on https://www.unicode.org/reports/tr29/tr29-41.html  (version date 2022-08-26)
        // Do not break between a CR and LF. Otherwise, break before and after controls.
        // GB3      CR × LF
        if (prev == code_point_property::CR && next == code_point_property::LF) return false;
        // GB4      (Control | CR | LF) ÷
        if (prev == code_point_property::Control || prev == code_point_property::CR || prev == code_point_property::LF) return true;
        // GB5      ÷ (Control | CR | LF)
        if (next == code_point_property::Control || next == code_point_property::CR || next == code_point_property::LF) return true;

        // Do not break Hangul syllable sequences.
        // GB6      L × (L | V | LV | LVT)
        if (prev == code_point_property::L && (next == code_point_property::L || next == code_point_property::V || next == code_point_property::LV || next == code_point_property::LVT)) return false;
        // GB7      (LV | V) × (V | T)
        if ((prev == code_point_property::LV || prev == code_point_property::V) && (next == code_point_property::V || next == code_point_property::T)) return false;
        // GB8      (LVT | T) × T
        if ((prev == code_point_property::LVT || prev == code_point_property::T) && (next == code_point_property::T)) return false;

        // Do not break before extending characters or ZWJ.
        // GB9      ×(Extend | ZWJ)
        if (next == code_point_property::Extend || next == code_point_property::ZWJ) return false;

        // The GB9a and GB9b rules only apply to extended grapheme clusters :
        // Do not break before SpacingMarks, or after Prepend characters.
        // GB9a     × SpacingMark
        if (next == code_point_property::SpacingMark) return false;
        // GB9b     Prepend ×
        if (prev == code_point_property::Prepend) return false;

        // Do not break within emoji modifier sequences or emoji zwj sequences.
        // GB11	    \p{ Extended_Pictographic } Extend * ZWJ × \p{ Extended_Pictographic }
        if (extended_pictographic_extend_times && prev == code_point_property::ZWJ && next == code_point_property::Extended_Pictographic) return false;

        // Do not break within emoji flag sequences.That is, do not break between regional indicator(RI) symbols if there is an odd number of RI characters before the break point.
        // GB12     sot(RI RI)* RI × RI
        // GB13     [^ RI](RI RI)* RI × RI
        if (!odd_number_of_RI && prev == code_point_property::Regional_Indicator && next == code_point_property::Regional_Indicator) return false;
        // Otherwise, break everywhere.
        // GB999    Any ÷ Any
        return true;
    }

public:
    template <typename ErrorHandlerT>
    explicit utf32_grapheme_enumerator_pusher(utf32_qrk<ErrorHandlerT> const&, enumerated_graphemes_qrk const&)
        : prev_prop{ 0 }
        , sot{ 1 }
        , extended_pictographic_extend_times{ 0 }
        , odd_number_of_RI{ 0 }
    {}

    inline void update_context_flags(unicode_detail::code_point_property prev)
    {
        using namespace unicode_detail;
        if (prev == code_point_property::Extended_Pictographic) {
            extended_pictographic_extend_times = 1;
        }
        else if (extended_pictographic_extend_times && prev != code_point_property::Extend) {
            extended_pictographic_extend_times = 0;
        }
        if (prev == code_point_property::Regional_Indicator && !odd_number_of_RI) {
            odd_number_of_RI = 1;
        } else {
            odd_number_of_RI = 0;
        }
    }

    template <SpanOf<char32_t> SpanT, typename ConsumerT>
    void push(SpanT chars, ConsumerT cons)
    {
        using namespace unicode_detail;
        if (chars.empty()) return;
        auto it = chars.begin(), sit = it, eit = chars.end();
        code_point_property prev;
        if (sot) {
            prev = get_property(chars.front());
            ++it;
            sot = 0;
        } else {
            prev = (code_point_property)prev_prop;
        }
        
        for (; it < eit; ++it) {
            code_point_property next = get_property(*it);
            if (need_break(prev, next)) {
                if (sit != chars.begin()) {
                    cons(std::pair{ grapheme_num_, chars.subspan(sit - chars.begin(), it - sit) });
                    sit = it;
                }
                ++grapheme_num_;
            }
            update_context_flags(prev);
            prev = next;
        }
        prev_prop = (uint_least8_t)prev;
        if (sit != eit) {
            cons(std::pair{ grapheme_num_, chars.subspan(sit - chars.begin(), eit - sit) });
        }
    }

    template <typename ConsumerT>
    void push(char32_t uchar, ConsumerT && cons)
    {
        push(std::span{ &uchar, 1 }, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT&&)
    {
        reset();
    }

    void reset()
    {
        grapheme_num_ = 0;

        sot = 1;
        extended_pictographic_extend_times = 0;
        odd_number_of_RI = 0;
    }
};

template <typename FromEHT>
struct cvt_resolver<utf32_qrk<FromEHT>, enumerated_graphemes_qrk>
{
    using type = utf32_grapheme_enumerator_pusher;
};

}
