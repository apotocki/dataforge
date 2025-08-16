/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <tuple>
#include <type_traits>
#include <memory>
#include <bitset>

#include "detail/quarks.hpp"
#include "detail/utility/concepts.hpp"

namespace dataforge {

template <typename ET, typename DataProviderT> struct last_provider;

template <typename ET, typename IteatorT>
requires(is_compatible_span_v<std::span<typename std::iterator_traits<IteatorT>::value_type>, ET>)
struct last_provider<ET, IteatorT>
{
    IteatorT first;
    IteatorT last;
    ET value;
    explicit last_provider(IteatorT f, IteatorT l)
        : first{ std::move(f) }
        , last{ std::move(l) }
    {}

    inline std::span<const ET> operator()()
    {
        if (first != last) {
            value = *first; ++first;
            return std::span{ &value, 1 };
        }
        return {};
    }
};

template <typename ET, typename IteatorT>
requires(is_compatible_span_v<decltype(std::span{std::declval<typename std::iterator_traits<IteatorT>::value_type const&>()}), ET>)
struct last_provider<ET, IteatorT>
{
    IteatorT first;
    IteatorT last;
    bool first_flag = true;

    explicit last_provider(IteatorT f, IteatorT l)
        : first{ std::move(f) }
        , last{ std::move(l) }
    {}

    inline std::span<const ET> operator()()
    {
        if (first_flag) {
            first_flag = false;
        } else if (first != last) {
            ++first;
        }
        return first != last ? span_cast<const ET>(std::span{*first}) : std::span<const ET>{};
    }
};

// just a compatible span
template <typename ET, CompatibleSpan<ET> SpanT>
struct last_provider<ET, SpanT>
{
    std::span<const ET> span;

    explicit last_provider(SpanT s): span{span_cast<const ET>(s)} {}

    last_provider & operator= (SpanT s)
    {
        span = span_cast<const ET>(s);
        return *this;
    }

    inline std::span<const ET> operator()()
    {
        auto rspan = span;
        span = {};
        return rspan;
    }
};

// range of elements convertible to compatible spans
template <typename ET, typename RangeT>
requires(is_compatible_span_v<decltype(std::span{*std::declval<RangeT>().begin()}), ET>)
struct last_provider<ET, RangeT> : last_provider<ET, decltype(std::declval<const RangeT>().begin())>
{
    using base_t = last_provider<ET, decltype(std::declval<const RangeT>().begin())>;
    explicit last_provider(RangeT const& rng) : base_t{ rng.begin(), rng.end() } {}

    last_provider& operator= (RangeT const& rng)
    {
        return static_cast<last_provider&>(base_t::operator = (base_t{ rng.begin(), rng.end() }));
    }
};

// range that isn't a span but can be converted to a compatible span
template <typename ET, std::ranges::contiguous_range RangeT>
requires(!is_compatible_span_v<RangeT, ET> && is_compatible_span_v<decltype(std::span{std::declval<const RangeT>()}), ET>)
struct last_provider<ET, RangeT> : last_provider<ET, decltype(std::span{ std::declval<const RangeT>() })>
{
    using base_t = last_provider<ET, decltype(std::span{ std::declval<const RangeT>() })> ;
    explicit last_provider(RangeT const& rng) : base_t{ std::span{rng} } {}

    last_provider & operator= (RangeT const& rng)
    {
        return static_cast<last_provider&>(base_t::operator= (std::span{rng}));
    }
};

// range that isn't a span but its elements can be ones of a compatible span
template <typename ET, typename RangeT>
requires(!is_compatible_span_v<RangeT, ET>
    && !std::ranges::contiguous_range<RangeT>
    && is_compatible_span_v<std::span<std::remove_cvref_t<decltype(*std::declval<const RangeT>().begin())>>, ET>)
struct last_provider<ET, RangeT> : last_provider<ET, decltype(std::declval<const RangeT>().begin())>
{
    using base_t = last_provider<ET, decltype(std::declval<const RangeT>().begin())>;
    explicit last_provider(RangeT const& rng) : base_t{ rng.begin(), rng.end() } {}

    last_provider& operator= (RangeT const& rng)
    {
        return static_cast<last_provider&>(base_t::operator = (base_t{ rng.begin(), rng.end() }));
    }
};

template <typename ConverterT, size_t I = ConverterT::chain_size - 1>
struct slice_pull_converter
{
    ConverterT & cvt_;
    explicit slice_pull_converter(ConverterT & cvt) noexcept : cvt_ { cvt } {}

    using cvt_tuple_type = typename ConverterT::cvt_tuple_type;
    using output_element_type = typename std::tuple_element_t<I, cvt_tuple_type>::output_element_type;

    inline std::span<const output_element_type> operator()() const
    {
        if (!cvt_.eof_flags.test(I)) {
            std::span<const output_element_type> r;
            if constexpr (I > 0) {
                r = std::get<I>(cvt_.chain()).pull(
                    std::get<I>(cvt_.inputs),
                    slice_pull_converter<ConverterT, I - 1>(cvt_)
                );
            } else {
                r = std::get<0>(cvt_.chain()).pull(
                    std::get<0>(cvt_.inputs),
                    std::ref(cvt_.provider)
                );
            }
            if (!r.empty()) return r;
            cvt_.eof_flags.set(I);
        }
        return {};
    }

    inline void reset() const
    {
        if constexpr (I > 0) {
            slice_pull_converter<ConverterT, I - 1>(cvt_).reset();
        }
        if constexpr (requires { std::get<I>(cvt_.chain()).reset(); }) {
            std::get<I>(cvt_.chain()).reset();
        }
    }
};

template <typename T> struct span_tuple;
template <typename ... Ts> struct span_tuple<std::tuple<Ts...>> { using type = std::tuple<std::span<const typename Ts::input_element_type> ...>; };

template <typename CvtTupleT, typename BaseIteratorT>
class pull_converter
{
    mutable quark_tuple_wrapper<CvtTupleT> cvt_tuple_;
    
public:
    using cvt_tuple_type = CvtTupleT;
    static constexpr size_t chain_size = std::tuple_size_v<CvtTupleT>;

    using input_element_type = typename std::tuple_element_t<0, CvtTupleT>::input_element_type;
    using output_element_type = typename std::tuple_element_t<chain_size - 1, CvtTupleT>::output_element_type;
    
    template <typename ... Quarks>
    pull_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>> const& chain, BaseIteratorT first, BaseIteratorT last)
        : cvt_tuple_{ chain }
        , provider{ std::move(first), std::move(last) }
    {}

    template <typename ... Quarks>
    pull_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>> const& chain, BaseIteratorT const& range)
        : cvt_tuple_{ chain }
        , provider{ range }
    {}

    auto pull()
    {
        return slice_pull_converter{*this}();
    }

    void reset(BaseIteratorT const& range)
    {
        provider = range;
        inputs = {};
        slice_pull_converter{ *this }.reset();
        eof_flags.reset();
    }

    inline CvtTupleT& chain() const { return *cvt_tuple_; }
    
    mutable typename span_tuple<CvtTupleT>::type inputs;
    mutable last_provider<input_element_type, BaseIteratorT> provider;
    mutable std::bitset<std::tuple_size_v<CvtTupleT>> eof_flags;
};

template <typename CvtTupleT, typename ... Quarks, typename RangeT>
pull_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&&, RangeT const&)-> pull_converter<CvtTupleT, RangeT>;

template <typename CvtTupleT, typename ... Quarks, typename IteratorT>
pull_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&&, IteratorT, IteratorT) -> pull_converter<CvtTupleT, IteratorT>;


template <typename ET>
struct polymorphic_pull_converter
{
    virtual ~polymorphic_pull_converter() = default;

    virtual std::span<ET> pull() = 0;
};

template <typename ET, typename ConverterT>
struct concrete_polymorphic_pull_converter : polymorphic_pull_converter<ET>
{
    explicit concrete_polymorphic_pull_converter(ConverterT&& cvt)
        : cvt_{ std::move(cvt) }
    {}

    std::span<ET> pull() override { return cvt_.pull(); }

private:
    ConverterT cvt_;
};

template <typename ET>
class dynamic_pull_converter
{
public:
    template <typename ConverterT>
    explicit dynamic_pull_converter(ConverterT && cvt)
        : cvt_{ std::make_shared<concrete_polymorphic_pull_converter<ET, ConverterT>>(std::forward<ConverterT>(cvt)) }
    {}

    template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
    dynamic_pull_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&& chain, BaseIteratorArgT&& it)
        : dynamic_pull_converter{ pull_converter{std::move(chain), std::forward<BaseIteratorArgT>(it)} }
    {}

    inline std::span<ET> pull() { return cvt_->pull(); }

private:
    std::shared_ptr<polymorphic_pull_converter<ET>> cvt_;
};

}
