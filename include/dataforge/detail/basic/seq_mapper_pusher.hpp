/*=============================================================================
    Copyright (c) 2025 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <memory>
#include <algorithm>

#include "../../push_converter.hpp"

namespace dataforge {

template <seq_change_action ActionV, typename CvtTupleT, typename ... Quarks>
class seq_mapper_pusher : public generic_pusher<void>
{
    cvt_tuple_wrapper<CvtTupleT> cvt_tuple_;
    size_t numerator_ {0};

public:
    using input_element_type = std::pair<size_t, typename std::tuple_element_t<0, CvtTupleT>::input_element_type>;
    using output_element_type = typename std::tuple_element_t<0, CvtTupleT>::input_element_type;

    template <typename SrcT>
    seq_mapper_pusher(SrcT const&, seq_mapper_qrk<ActionV, quark_chain<CvtTupleT, std::tuple<Quarks ...>>> const& smt)
        : cvt_tuple_{ smt.chain }
    {

    }

    template <typename T, typename ConsumerT>
    void push(std::pair<size_t, T> ival, ConsumerT cons)
    {
        tagged_converter conv{ *cvt_tuple_, numerator_, cons };
        if (ival.first != numerator_) {
            if constexpr (ActionV == seq_change_action::flush) {
                conv.flush();
            } else {
                conv.finish();
            }
            conv.tag = numerator_ = ival.first;
        }
        conv(ival.second);
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT cons)
    {
        tagged_converter conv{ *cvt_tuple_, numerator_, cons };
        conv.finish();
        reset();
    }

    inline void reset() noexcept { numerator_ = 0; }
};

template <typename FromEH, seq_change_action ActionV, typename CvtTupleT, typename ... Quarks>
struct cvt_resolver<enum_qrk<FromEH>, seq_mapper_qrk<ActionV, quark_chain<CvtTupleT, std::tuple<Quarks ...>>>>
{
    using type = seq_mapper_pusher<ActionV, CvtTupleT, Quarks...>;
};

}
