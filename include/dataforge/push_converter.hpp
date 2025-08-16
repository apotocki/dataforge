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

#include "detail/quarks.hpp"

namespace dataforge {

template <typename IteatorT>
struct last_consumer
{
    IteatorT& base;
    explicit last_consumer(IteatorT& it) : base{ it } {}

    template <typename DataT>
    inline void operator()(DataT val) const
    {
        if constexpr (requires{
            *base = val;
        }) {
            *base = val;
            ++base;
        }
        else if constexpr (requires{
            base(val);
        }) {
            base(val);
        } else {
            for (auto d : val) {
                *base = d;
                ++base;
            }
        }
    }
};

template <typename ConverterT, size_t I = 0>
struct slice_push_converter
{
    ConverterT & cvt_;
    explicit slice_push_converter(ConverterT & cvt) noexcept : cvt_ { cvt } {}

    template <typename DataT>
    void operator()(DataT data)
    {
        if constexpr (I + 1 < ConverterT::chain_size) {
            std::get<I>(cvt_.chain()).push(
                std::move(data),
                slice_push_converter<ConverterT, I + 1>(cvt_)
            );
        } else {
            std::get<I>(cvt_.chain()).push(
                std::move(data),
                last_consumer{ cvt_.consumer() }
            );
        }
    }

    void flush()
    {
        if constexpr (I + 1 < ConverterT::chain_size) {
            slice_push_converter<ConverterT, I + 1> sc(cvt_);
            if constexpr (requires { std::get<I>(cvt_.chain()).flush(sc); }) {
                std::get<I>(cvt_.chain()).flush(sc);
            }
            sc.flush();
        } else {
            if constexpr (requires { std::get<I>(cvt_.chain()).flush(last_consumer{ cvt_.consumer() }); }) {
                std::get<I>(cvt_.chain()).flush(last_consumer{ cvt_.consumer() });
            }
        }
    }

    void finish()
    {
        if constexpr (I + 1 < ConverterT::chain_size) {
            std::get<I>(cvt_.chain()).finish(
                slice_push_converter<ConverterT, I + 1>(cvt_)
            );
            slice_push_converter<ConverterT, I + 1>(cvt_).finish();
        } else {
            std::get<I>(cvt_.chain()).finish(last_consumer{ cvt_.consumer() });
        }
    }

    void reset()
    {
        if constexpr (I + 1 < ConverterT::chain_size) {
            slice_push_converter<ConverterT, I + 1>(cvt_).reset();
        }
        if constexpr (requires { std::get<I>(cvt_.chain()).reset(); }) {
            std::get<I>(cvt_.chain()).reset();
        }
    }
};

template <typename CvtTupleT, typename BaseIteratorT>
class push_converter
{
    mutable quark_tuple_wrapper<CvtTupleT> cvt_tuple_;
    mutable BaseIteratorT base;

    using consumer_iterator_t = typename std::conditional_t<
        is_reference_wrapper_v<BaseIteratorT>,
        BaseIteratorT,
        std::type_identity<BaseIteratorT>
    >::type;

public:
    static constexpr size_t chain_size = std::tuple_size_v<CvtTupleT>;
    using input_element_type = typename std::tuple_element_t<0, CvtTupleT>::input_element_type;
    using output_element_type = typename std::tuple_element_t<chain_size - 1, CvtTupleT>::output_element_type;

    template <typename ... Quarks, typename BaseIteratorArgT>
    push_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>> const& chain, BaseIteratorArgT&& it)
        : cvt_tuple_{ chain }
        , base{ std::forward<BaseIteratorArgT>(it) }
    {}

    template <typename InputDataT>
    void push(InputDataT s)
    {
        slice_push_converter{*this}(std::move(s));
    }

    void flush()
    {
        slice_push_converter{ *this }.flush();
    }

    void finish()
    {
        slice_push_converter{*this}.finish();
    }

    void reset()
    {
        slice_push_converter{ *this }.reset();
    }

    inline CvtTupleT& chain() const { return *cvt_tuple_; }
    inline consumer_iterator_t& consumer() const
    {
        if constexpr (is_reference_wrapper_v<BaseIteratorT>) {
            return base.get();
        } else {
            return base;
        }
    }
};

template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
push_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&&, BaseIteratorArgT&&)->push_converter<CvtTupleT, std::remove_cvref_t<BaseIteratorArgT>>;

template <typename ET>
struct polymorphic_push_converter
{
    virtual ~polymorphic_push_converter() = default;

    virtual void push(ET elem) = 0;
    virtual void push(std::span<const ET> elems) = 0;
    virtual void flush() = 0;
    virtual void finish() = 0;

    template <size_t E>
    void push(std::span<const ET, E> elems)
    {
        push( std::span{elems.data(), elems.size()} );
    }
};

template <typename ET, typename ConverterT>
struct concrete_polymorphic_push_converter : polymorphic_push_converter<ET>
{
    explicit concrete_polymorphic_push_converter(ConverterT&& cvt)
        : cvt_{ std::move(cvt) }
    {}

    void push(ET elem) override { cvt_.push(elem); }
    void push(std::span<const ET> elems) override { cvt_.push(elems); }
    void flush() override { cvt_.flush(); }
    void finish() override { cvt_.finish(); }

private:
    ConverterT cvt_;
};

template <typename ET>
class dynamic_push_converter
{
public:
    template <typename ConverterT>
    explicit dynamic_push_converter(ConverterT && cvt)
        : cvt_{ std::make_shared<concrete_polymorphic_push_converter<ET, ConverterT>>(std::forward<ConverterT>(cvt)) }
    {}

    template <typename CvtTupleT, typename ... Quarks, typename BaseIteratorArgT>
    dynamic_push_converter(quark_chain<CvtTupleT, std::tuple<Quarks ...>>&& chain, BaseIteratorArgT&& it)
        : dynamic_push_converter{ push_converter{std::move(chain), std::forward<BaseIteratorArgT>(it)} }
    {}

    inline void push(ET elem) { cvt_->push(elem); }
    inline void push(std::span<const ET> elems) { cvt_->push(elems); }
    inline void flush() { cvt_->flush(); }
    inline void finish() { cvt_->finish(); }

private:
    std::shared_ptr<polymorphic_push_converter<ET>> cvt_;
};

}
