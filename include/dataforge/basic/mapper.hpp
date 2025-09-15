/*=============================================================================
    Copyright (c) 2025 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"
#include "../push_converter.hpp"

namespace dataforge {

template <typename TagT, TagT TagV, typename ChainT> // ChainT = quark_chain<CvtChainT, std::tuple<Quarks ...>>
struct tchain
{
    using chain_type = ChainT;
    static constexpr TagT tag = TagV;
    chain_type chain;
    explicit tchain(ChainT&& c) : chain{ std::move(c) } { }
};

template <typename T> struct is_tagged_chain : std::false_type {};
template <typename TagT, TagT TagV, typename ChainT> struct is_tagged_chain<tchain<TagT, TagV, ChainT>> : std::true_type {};

template <typename T, typename TagT> struct is_particular_tagged_chain : std::false_type {};
template <typename TagT, TagT TagV, typename ChainT> struct is_particular_tagged_chain<tchain<TagT, TagV, ChainT>, TagT> : std::true_type {};

template <typename T>
concept tagged_chain = is_tagged_chain<T>::value;

template <typename T, typename TagT>
concept particular_tagged_chain = is_particular_tagged_chain<T, TagT>::value;

template <auto TV, typename ChainT>
tchain<decltype(TV), TV, std::remove_cvref_t<ChainT>> mapper_case(ChainT&& c) { return tchain<decltype(TV), TV, std::remove_cvref_t<ChainT>>(std::move(c)); }


template <typename TagT, particular_tagged_chain<TagT> ... TaggedChainsT>
struct tagged_mapper : cvt_qrk<void>
{
    using cvt_quark_t = cvt_qrk<void>;
    using cvt_quark_t::cvt_quark_t;

    std::tuple<typename TaggedChainsT::chain_type...> cases;

    inline explicit tagged_mapper(TaggedChainsT && ... args) : cases{ std::move(args.chain) ... } {}
};

template <typename TagT, TagT ... TV, typename ... ChainT>
tagged_mapper(tchain<TagT, TV, ChainT> && ...) -> tagged_mapper<TagT, tchain<TagT, TV, ChainT>...>;

enum class seq_change_action
{
    flush, finish
};

template <seq_change_action ActionV, typename ChainT>
struct seq_mapper_qrk : cvt_qrk<void>
{
    ChainT chain;
    inline explicit seq_mapper_qrk(ChainT && c) noexcept
        : chain{ std::move(c) }
    {}
};

template <seq_change_action AV = seq_change_action::flush, typename ChainT>
inline seq_mapper_qrk<AV, ChainT> seq_mapper(ChainT && c)
{
    return seq_mapper_qrk<AV, ChainT>(std::forward<ChainT>(c));
}

template <typename TagT, typename ChainT, typename ConsumerT>
struct tagged_converter
{
    ConsumerT& cons_;
    ChainT& chain_;
    TagT tag;

    inline tagged_converter(ChainT& ch, TagT t, ConsumerT& c) : cons_{ c }, chain_{ ch }, tag{ t } {}

    static constexpr size_t chain_size = std::tuple_size_v<ChainT>;
    inline ChainT& chain() const noexcept { return chain_; }
    inline tagged_converter& consumer() noexcept { return *this; }

    inline tagged_converter& operator*() noexcept { return *this; }
    inline void operator++() noexcept {}

    template <typename T>
    inline void operator=(T&& fragment)
    {
        cons_(std::pair{ tag, std::forward<T>(fragment) });
    }

    template <typename DataT>
    inline void operator()(DataT && data)
    {
        slice_push_converter{ *this }(std::forward<DataT>(data));
    }

    void flush()
    {
        slice_push_converter{ *this }.flush();
    }

    void finish()
    {
        slice_push_converter{ *this }.finish();
    }
};

template <typename TagT, TagT TV, particular_tagged_chain<TagT> TC0, particular_tagged_chain<TagT> ... TaggedChainsT>
struct find_chain : std::conditional_t<TC0::tag == TV, std::type_identity<typename TC0::chain_type::cvt_tuple_type>, find_chain<TagT, TV, TaggedChainsT ...>> {};

template <typename TagT, TagT TV, particular_tagged_chain<TagT> TC0>
struct find_chain<TagT, TV, TC0> : std::conditional_t<TC0::tag == TV, std::type_identity<typename TC0::chain_type::cvt_tuple_type>, std::type_identity<void>> {};

template <auto TV, particular_tagged_chain<decltype(TV)> ... TaggedChainsT>
using find_chain_t = typename find_chain<decltype(TV), TV, TaggedChainsT...>::type;

template <typename TagT, TagT TV, size_t PosV, particular_tagged_chain<TagT> TC0, particular_tagged_chain<TagT> ... TaggedChainsT>
struct find_chain_index : std::conditional_t<TC0::tag == TV, std::integral_constant<size_t, PosV>, find_chain_index<TagT, TV, PosV + 1, TaggedChainsT ...>> {};

template <typename TagT, TagT TV, size_t PosV, particular_tagged_chain<TagT> TC0>
struct find_chain_index<TagT, TV, PosV, TC0> : std::conditional_t<TC0::tag == TV, std::integral_constant<size_t, PosV>, std::type_identity<void>> {};

template <auto TV, particular_tagged_chain<decltype(TV)> ... TaggedChainsT>
constexpr size_t find_chain_index_v = find_chain_index<decltype(TV), TV, 0, TaggedChainsT...>::value;

}

#include "../detail/basic/seq_mapper_pusher.hpp"
