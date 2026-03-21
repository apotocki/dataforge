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

    static inline unicode_detail::code_point_property get_property(char32_t uchar)
    {
        return unicode_detail::grapheme_cluster_property(uchar);
    }

    uint_least8_t prev_prop : 4;
    uint_least8_t sot : 1; // start of text
    // flags for a context before prev
    uint_least8_t extended_pictographic_extend_times : 1;
    uint_least8_t odd_number_of_RI : 1;

    bool need_break(unicode_detail::code_point_property prev, unicode_detail::code_point_property next) const
    {
        unicode_detail::grapheme_break_context ctx;
        ctx.extended_pictographic_extend_times = extended_pictographic_extend_times;
        ctx.odd_number_of_RI = odd_number_of_RI;
        return unicode_detail::grapheme_need_break(prev, next, ctx);
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
        unicode_detail::grapheme_break_context ctx;
        ctx.extended_pictographic_extend_times = extended_pictographic_extend_times;
        ctx.odd_number_of_RI = odd_number_of_RI;
        ctx.update(prev);
        extended_pictographic_extend_times = ctx.extended_pictographic_extend_times;
        odd_number_of_RI = ctx.odd_number_of_RI;
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
