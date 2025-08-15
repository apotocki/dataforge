/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename ET, typename PredicateT>
class filter_pusher : public generic_pusher<void>
{
    PredicateT predicate;

public:
    using input_element_type = ET;
    using output_element_type = ET;

    template <typename SrcT>
    filter_pusher(SrcT const&, filter_qrk<ET, PredicateT> const& t)
        : predicate { t.predicate }
    {}

    template <CompatibleSpan<ET> SpanT, typename ConsumerT>
    void push(SpanT ispan, ConsumerT cons)
    {
        auto* b = ispan.data(), *e = ispan.data() + ispan.size();
        for (auto* p = b; p != e;) {
            if (!predicate(*p)) {
                if (b != p) cons(std::span{b, p});
                b = ++p;
            } else {
                ++p;
            }
        }
        if (b != e) {
            cons(std::span{b, e});
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        if (predicate(static_cast<ET>(ival))) {
            cons(ival);
        }
    }

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        if (input.empty()) {
            input = span_cast<const input_element_type>(p());
            if (input.empty()) {
                return { };
            }
        }
        for (;;) {
            auto* b = input.data(), * e = input.data() + input.size();
            for (auto* p = b; p != e;) {
                if (!predicate(*p)) {
                    input = { p + 1, e };
                    if (b != p) return { b, p };
                    b = p + 1;
                }
                ++p;
            }
                 
            if (b != e) {
                input = {};
                return { b, e };
            }
            input = span_cast<const input_element_type>(p());
            if (input.empty()) {
                return { };
            }
        }
    }

    template <typename ConsumerT>
    inline void flush(ConsumerT &&) noexcept
    {}

    template <typename ConsumerT>
    inline void finish(ConsumerT &&) noexcept
    {}

    inline void reset() noexcept {}
};

template <typename SrcT, typename IT, typename PredicateT>
struct cvt_resolver<SrcT, filter_qrk<IT, PredicateT>>
{
    using type = filter_pusher<IT, PredicateT>;
};

}
