/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <iterator>

#include "pull_converter.hpp"

namespace dataforge {

template <typename ConverterT>
class quark_pull_iterator
{
public:
    using value_type = std::span<const typename ConverterT::output_element_type>;
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    explicit quark_pull_iterator(ConverterT&& cvt)
        : cvt_{ std::move(cvt) }
    {}

    template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
    quark_pull_iterator(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&& chain, BaseIteratorArgT&& it)
        : cvt_{ std::move(chain), std::forward<BaseIteratorArgT>(it) }
    {
        
    }

    quark_pull_iterator(quark_pull_iterator&&) = default;
    quark_pull_iterator& operator=(quark_pull_iterator&&) = default;

    value_type operator*()
    {
        if (value_.empty()) {
            value_ = cvt_.pull();
        }
        return value_;
    }

    quark_pull_iterator& operator++()
    {
        value_ = cvt_.pull();
        return *this;
    }

    template <typename BaseIteratorArgT >
    void reset(BaseIteratorArgT&& it)
    {
        cvt_.reset(std::forward<BaseIteratorArgT>(it));
        value_ = {};
    }

private:
    ConverterT cvt_;
    value_type value_;
};

template <typename ConverterT>
quark_pull_iterator(ConverterT&& cvt)->quark_pull_iterator<ConverterT>;

template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
quark_pull_iterator(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&&, BaseIteratorArgT&&)
    ->quark_pull_iterator<pull_converter<CvtTupleT, std::remove_cvref_t<BaseIteratorArgT>>>;

}
