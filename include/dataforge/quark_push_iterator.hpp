/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <iterator>

#include "push_converter.hpp"

namespace dataforge {

template <typename ConverterT>
class quark_push_iterator
{
public:
    using value_type = void; // std::span<typename ConverterT::input_element_type>;
    using iterator_category = std::output_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    using converter_type = ConverterT;

    explicit quark_push_iterator(ConverterT&& cvt)
        : cvt_{ std::move(cvt) }
    {}

    template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
    quark_push_iterator(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&& chain, BaseIteratorArgT&& it)
        : cvt_{ std::move(chain), std::forward<BaseIteratorArgT>(it) }
    {}

    quark_push_iterator(quark_push_iterator&&) = default;
    quark_push_iterator& operator=(quark_push_iterator&&) = default;

    template <typename E>
    inline quark_push_iterator& operator=(E && val)
    {
        if constexpr(is_span_convertible_v<std::remove_cvref_t<E>>) {
            cvt_.push(std::span{ std::forward<E>(val) });
        } else {
            cvt_.push(std::forward<E>(val));
        }
        return *this;
    }

    template <typename E>
    inline quark_push_iterator& operator<< (E const& val) {
        return this->operator=(val);
    }

    quark_push_iterator& operator*() noexcept { return *this; }

    quark_push_iterator& operator++() noexcept { return *this; }

    quark_push_iterator operator++(int) { return *this; }
    
    void flush()
    {
        cvt_.flush();
    }

    void finish()
    {
        cvt_.finish();
    }

private:
    ConverterT cvt_;
};

template <typename ConverterT>
quark_push_iterator(ConverterT&& cvt)->quark_push_iterator<ConverterT>;

template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
quark_push_iterator(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&&, BaseIteratorArgT&&)
    ->quark_push_iterator<push_converter<CvtTupleT, std::remove_cvref_t<BaseIteratorArgT>>>;

}
