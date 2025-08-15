/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "../detail/quarks.hpp"
#include "../push_converter.hpp"

namespace dataforge {

template <typename QuarkT, QuarkT QuarkV, typename ChainT> // ChainT = quark_chain<CvtChainT, std::tuple<Quarks ...>>
struct tchain
{
    using chain_type = ChainT;
    static constexpr QuarkT quark = QuarkV;
    chain_type chain;
    explicit tchain(ChainT&& c) : chain{ std::move(c) } { }
};

template <typename T> struct is_quarked_chain : std::false_type {};
template <typename QuarkT, QuarkT QuarkV, typename ChainT> struct is_quarked_chain<tchain<QuarkT, QuarkV, ChainT>> : std::true_type {};

template <typename T, typename QuarkT> struct is_particular_quarked_chain : std::false_type {};
template <typename QuarkT, QuarkT QuarkV, typename ChainT> struct is_particular_quarked_chain<tchain<QuarkT, QuarkV, ChainT>, QuarkT> : std::true_type {};

template <typename T>
concept quarked_chain = is_quarked_chain<T>::value;

template <typename T, typename QuarkT>
concept particular_quarked_chain = is_particular_quarked_chain<T, QuarkT>::value;

template <auto VT, typename ChainT>
tchain<decltype(VT), VT, std::remove_cvref_t<ChainT>> mapper_case(ChainT&& c) { return tchain<decltype(VT), VT, std::remove_cvref_t<ChainT>>(std::move(c)); }


template <typename QuarkT, particular_quarked_chain<QuarkT> ... QuarkedChainsT>
struct tagged_mapper : cvt_qrk<void>
{
    using cvt_quark_t = cvt_qrk<void>;
    using cvt_quark_t::cvt_quark_t;

    std::tuple<typename QuarkedChainsT::chain_type...> cases;

    inline explicit tagged_mapper(QuarkedChainsT && ... args) : cases{ std::move(args.chain) ... } {}
};

template <typename T, T ... VT, typename ... ChainT>
tagged_mapper(tchain<T, VT, ChainT> && ...) -> tagged_mapper<T, tchain<T, VT, ChainT>...>;

enum class seq_change_action
{
    flush, finish
};

template <seq_change_action ActionV, typename ChainT>
struct seq_mapper_qrk : cvt_qrk<void>
{
    ChainT chain;
    explicit seq_mapper_qrk(ChainT && c)
        : chain{ std::move(c) }
    {
        
    }
};

template <seq_change_action AV = seq_change_action::flush, typename ChainT>
inline seq_mapper_qrk<AV, ChainT> seq_mapper(ChainT && c)
{
    return seq_mapper_qrk<AV, ChainT>(std::forward<ChainT>(c));
}

template <typename QuarkT, typename ChainT, typename ConsumerT>
struct tagged_converter
{
    ConsumerT& cons_;
    ChainT& chain_;
    QuarkT quark;

    tagged_converter(ChainT& ch, QuarkT t, ConsumerT& c) : cons_{ c }, chain_{ ch }, quark{ t } {}

    static constexpr size_t chain_size = std::tuple_size_v<ChainT>;
    inline ChainT& chain() const { return chain_; }
    inline tagged_converter& consumer() { return *this; }

    inline tagged_converter& operator*() { return *this; }
    inline void operator++() {}

    template <typename T>
    inline void operator=(T&& fragment)
    {
        cons_(std::pair{ quark, std::forward<T>(fragment) });
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

template <typename QuarkT, QuarkT TV, particular_quarked_chain<QuarkT> TC0, particular_quarked_chain<QuarkT> ... QuarkedChainsT>
struct find_chain : std::conditional_t<TC0::quark == TV, std::type_identity<typename TC0::chain_type::cvt_tuple_type>, find_chain<QuarkT, TV, QuarkedChainsT ...>> {};

template <typename QuarkT, QuarkT TV, particular_quarked_chain<QuarkT> TC0>
struct find_chain<QuarkT, TV, TC0> : std::conditional_t<TC0::quark == TV, std::type_identity<typename TC0::chain_type::cvt_tuple_type>, std::type_identity<void>> {};

template <auto TV, particular_quarked_chain<decltype(TV)> ... QuarkedChainsT>
using find_chain_t = typename find_chain<decltype(TV), TV, QuarkedChainsT...>::type;

template <typename QuarkT, QuarkT TV, size_t PosV, particular_quarked_chain<QuarkT> TC0, particular_quarked_chain<QuarkT> ... QuarkedChainsT>
struct find_chain_index : std::conditional_t<TC0::quark == TV, std::integral_constant<size_t, PosV>, find_chain_index<QuarkT, TV, PosV + 1, QuarkedChainsT ...>> {};

template <typename QuarkT, QuarkT TV, size_t PosV, particular_quarked_chain<QuarkT> TC0>
struct find_chain_index<QuarkT, TV, PosV, TC0> : std::conditional_t<TC0::quark == TV, std::integral_constant<size_t, PosV>, std::type_identity<void>> {};

template <auto TV, particular_quarked_chain<decltype(TV)> ... QuarkedChainsT>
constexpr size_t find_chain_index_v = find_chain_index<decltype(TV), TV, 0, QuarkedChainsT...>::value;

}

#include "../detail/basic/seq_mapper_pusher.hpp"
