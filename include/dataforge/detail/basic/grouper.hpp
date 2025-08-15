/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

// that's not a fish
template <typename ET>
class grouper : public generic_pusher<void>
{
    size_t item_num { 0 };
    size_t group_num { 0 };
    size_t group_sz;
    size_t row_sz;
    std::span<const ET> prefix;
    std::span<const ET> suffix;
    std::span<const ET> delim;
    std::span<const ET> rowdelim;

public:
    using input_element_type = ET;
    using output_element_type = ET;

    template <typename SrcT>
    grouper(SrcT const& quark, grouped_qrk<ET> const& gt)
        : group_sz{ gt.group_sz }
        , row_sz{ gt.row_sz }
        , prefix{ gt.prefix }
        , suffix{ gt.suffix }
        , delim{ gt.delim }
        , rowdelim { gt.rowdelim }
    {}

    template <CompatibleSpan<ET> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        if (item_num == group_sz) {
            item_num = 0;
            ++group_num;
            if (group_num == row_sz) {
                if (!rowdelim.empty()) cons(rowdelim);
                group_num = 0;
            } else if (!delim.empty()) {
                cons(delim);
            }
        }

        if (!item_num && !prefix.empty()) {
            cons(prefix);
        }
        cons(ival);
        if (++item_num == group_sz) {
            if (!suffix.empty()) cons(suffix);
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept
    {
        reset();
    }

    inline void reset() noexcept { group_num = item_num = 0; }

    template <typename ProviderT>
    std::span<const ET> pull(std::span<const ET>& input, ProviderT p)
    {
        if (item_num == group_sz) {
            ++item_num;
            if (!suffix.empty()) return suffix;
        }
        
        if (input.empty()) {
            input = span_cast<const ET>(p());
            if (input.empty()) {
                return input;
            }
        }

        if (item_num == group_sz + 1) {
            item_num = 0;
            ++group_num;
            if (group_num == row_sz) {
                group_num = 0;
                if (!rowdelim.empty()) {
                    return rowdelim;
                }
            } else if (!delim.empty()) {
                return delim;
            }
        }

        if (!item_num) {
            item_num = (std::numeric_limits<size_t>::max)(); // let's it's theposition after prefix
            if (!prefix.empty()) return prefix;
        }

        if (item_num == (std::numeric_limits<size_t>::max)()) {
            item_num = 0;
        }
        auto sz = (std::min)(group_sz - item_num, input.size());
        std::span<const ET> r = input.subspan(0, sz);
        input = input.subspan(sz);
        item_num += sz;
        return r;
    }
};

/*
template <typename IT, typename FromEHT, typename ToEHT>
struct cvt_resolver<int_qrk<sizeof(IT) * CHAR_BIT, FromEHT>, grouped_qrk<IT, ToEHT>>
{
    using type = grouper<IT>;
};
*/

template <typename FromT, typename IT>
struct cvt_resolver<FromT, grouped_qrk<IT>>
{
    using type = grouper<IT>;
};

}
