/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#pragma once

#include <cassert>
#include <stdexcept>
#include <utility>
#include <type_traits>
#include <concepts>
#include <span>
#include <algorithm>

#include "error_handler.hpp"

#include "utility/concepts.hpp"

#define DECLARE_CVT_QUARK(name)                 \
template <typename ErrorHandlerT>               \
struct name##_qrk : cvt_qrk<ErrorHandlerT>      \
{                                               \
    using cvt_qrk_t = cvt_qrk<ErrorHandlerT>;   \
    using cvt_qrk_t::cvt_qrk_t;                 \
};                                              \
inline name##_qrk<throw_error_handler> name;

#define DECLARE_INTEGRAL_CVT_QUARK(name, type, bits) \
template <typename ErrorHandlerT>                    \
struct name##_qrk : int_qrk<bits, ErrorHandlerT>     \
{                                                    \
    using integral_type = type;                      \
};                                                   \
inline name##_qrk<throw_error_handler> name;

#define DECLARE_INTEGRAL_CVT_QUARK_NOEH(name, type, bits) \
struct name##_qrk : int_qrk<bits, void>                   \
{                                                         \
    using integral_type = type;                           \
};                                                        \
inline name##_qrk name;

namespace dataforge {

template <typename ...> constexpr bool dependent_false = false;

struct quark_base {};

template <typename ErrorHandlerT>
struct cvt_qrk : quark_base
{
    template <typename ... EHArgTs>
    requires (!std::is_void_v<ErrorHandlerT> || 0 == sizeof ... (EHArgTs))
    explicit cvt_qrk(EHArgTs&& ... ehargs) : errh_{ std::forward<EHArgTs>(ehargs) ... } {}

    ErrorHandlerT errh_;
};

template <> struct cvt_qrk<void> : quark_base {};

template <typename StartT, typename FinishT>
struct compound_qrk : cvt_qrk<void>
{
    StartT start;
    FinishT finish;

    compound_qrk() = default;

    template <typename SArgT>
    compound_qrk(SArgT&& s, nullptr_t)
        : start{ std::forward<SArgT>(s) }
    {}

    template <typename SArgT, typename FArgT>
    compound_qrk(SArgT && s, FArgT && f)
        : start{std::forward<SArgT>(s)}, finish{std::forward<FArgT>(f)}
    {}
};

template <typename Quark> struct cvt_start_quark { using type = Quark; };
template <typename StartT, typename FinishT> struct cvt_start_quark<compound_qrk<StartT, FinishT>> { using type = StartT; };
template <typename Quark> using cvt_start_quark_t = typename cvt_start_quark<Quark>::type;

template <typename Quark> struct cvt_finish_quark { using type = Quark; };
template <typename StartT, typename FinishT> struct cvt_finish_quark<compound_qrk<StartT, FinishT>> { using type = FinishT; };
template <typename Quark> using cvt_finish_quark_t = typename cvt_finish_quark<Quark>::type;

template <typename QuarkT>
inline QuarkT const& as_start(QuarkT const& quark) noexcept { return quark; }

template <typename StartT, typename FinishT>
inline StartT const& as_start(compound_qrk<StartT, FinishT> const& quark) noexcept { return quark.start; }

template <typename QuarkT>
inline QuarkT const& as_finish(QuarkT const& quark) noexcept { return quark; }

template <typename StartT, typename FinishT>
inline FinishT const& as_finish(compound_qrk<StartT, FinishT> const& quark) noexcept { return quark.finish; }

///////// basic routines
using cchar_span_t = std::span<const char>;
using cbyte_span_t = std::span<const unsigned char>;
inline cbyte_span_t operator "" _bs(const char* str, std::size_t n)
{
    return std::span{reinterpret_cast<const unsigned char*>(str), n};
}

inline auto operator "" _sp(const char* str, std::size_t n)
{
    return std::span{str, n};
}

inline auto operator "" _sp(const char8_t* str, std::size_t n)
{
    return std::span{str, n};
}

inline auto operator "" _sp(const char16_t* str, std::size_t n)
{
    return std::span{ str, n };
}

inline auto operator "" _sp(const char32_t* str, std::size_t n)
{
    return std::span{ str, n };
}

inline auto operator "" _sp(const wchar_t* str, std::size_t n)
{
    return std::span{str, n};
}

template <typename T, typename ST, size_t E>
requires(sizeof(T) == sizeof(ST))
std::span<T> span_cast(std::span<ST, E> s)
{
    return { reinterpret_cast<T*>(s.data()), s.size() };
}
///////// declare basic tags

template <size_t BitSzV, typename ErrorHandlerT>
struct int_qrk : cvt_qrk<ErrorHandlerT>, select_int<BitSzV>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;
    using cvt_quark_t::cvt_quark_t;
    using int_qrk_t = int_qrk;
    using integral_type = typename int_qrk_t::unsigned_type;
    constexpr static size_t bits = BitSzV;
};
inline int_qrk<8, throw_error_handler> int8;
inline int_qrk<16, throw_error_handler> int16;
inline int_qrk<32, throw_error_handler> int32;
inline int_qrk<64, throw_error_handler> int64;

DECLARE_CVT_QUARK(le)
DECLARE_CVT_QUARK(be)

template <typename ErrorHandlerT>
struct enum_qrk : cvt_qrk<ErrorHandlerT>
{
    using cvt_quark_t = cvt_qrk<ErrorHandlerT>;
    using cvt_quark_t::cvt_quark_t;
};
inline enum_qrk<throw_error_handler> enumerated;

template <typename T, size_t BitSzV> struct is_int_qrk : std::false_type {};
template <typename ErrorHandlerT, size_t BitSzV> struct is_int_qrk<int_qrk<BitSzV, ErrorHandlerT>, BitSzV> : std::true_type {};
template <typename T, size_t BitSzV> constexpr bool is_int_qrk_v = is_int_qrk<T, BitSzV>::value;


template <typename T, size_t BitSzV>
concept IntegralBasedQuark = is_int_qrk_v<typename T::int_qrk_t, BitSzV>;

template <typename T>
concept SomeIntegralBasedQuark = sizeof(typename T::int_qrk_t) > 0;

template <typename ErrorHandlerT>
class generic_pusher : protected ErrorHandlerT
{
protected:
    explicit generic_pusher(cvt_qrk<ErrorHandlerT> const& arg)
        : ErrorHandlerT{ arg.errh_ }
    {}

public:
    template <typename ... ArgsT>
    inline void on_error(ArgsT&& ... args) const
    {
        static_cast<ErrorHandlerT const&>(*this)(std::forward<ArgsT>(args) ...);
    }
};

template <>
class generic_pusher<void>
{
protected:
    generic_pusher() = default;
    explicit generic_pusher(cvt_qrk<void> const&) {}

public:
    template <typename ... ArgsT>
    void on_error(ArgsT&& ...) const
    {
        throw std::runtime_error("unhandled error");
    }
};

template <typename DerivedT, typename InputT, typename OutputT, size_t CasheSz>
class basic_puller
{
public:
    using input_element_type = InputT;
    using output_element_type = OutputT;

    output_element_type values_[CasheSz];

    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        size_t osz;
        auto provider_fn = [this, &osz](auto spanorval) {
            if constexpr (std::is_same_v<output_element_type, decltype(spanorval)>) {
                assert(osz < sizeof(values_));
                values_[osz] = spanorval;
                ++osz;
            } else {
                assert(osz + spanorval.size() <= sizeof(values_));
                std::copy(spanorval.begin(), spanorval.end(), values_ + osz);
                osz += spanorval.size();
            }
        };

        for (;;) {
            if (input.empty()) {
                input = span_cast<const input_element_type>(p());
                if (input.empty()) {
                    osz = 0;
                    static_cast<DerivedT*>(this)->finish(provider_fn);
                    return { values_, osz };
                }
            }
            input_element_type ival = input.front();
            input = input.subspan(1);
            osz = 0;
            static_cast<DerivedT*>(this)->push(ival, provider_fn);
            if (osz) return { values_, osz };
        }
    }
};

template <typename CvtTupleT, typename QuarkTupleT>
struct quark_chain
{
    using cvt_tuple_type = CvtTupleT;
    using quark_tuple_type = QuarkTupleT;
    QuarkTupleT quarks;
};

template <typename CvtTupleT, typename QuarkTupleT>
auto make_quark_chain(QuarkTupleT t) { return quark_chain<CvtTupleT, QuarkTupleT>{ std::move(t) }; }

template <typename From, typename To>
struct cvt_resolver
{
    static_assert(dependent_false<From, To>, "No converter found between the given quarks.");
};

template <typename CvtTupleT>
struct quark_tuple_wrapper
{
    CvtTupleT tuple;

    template <size_t ... I, typename Tuple>
    quark_tuple_wrapper(std::index_sequence<I ...>, Tuple const& t)
        : tuple{ std::tuple_element_t<I, CvtTupleT>{ as_finish(std::get<I>(t)), as_start(std::get<I + 1>(t)) } ... }
    {}

    template <typename ... Quarks>
    explicit quark_tuple_wrapper(quark_chain<CvtTupleT, std::tuple<Quarks ...>> const& chain)
        : quark_tuple_wrapper(std::make_index_sequence<(sizeof ... (Quarks)) - 1>(), chain.quarks)
    {}

    CvtTupleT& operator*() { return tuple; }
};

/* this indirection is only intended for a more friendly compiler error message if no converter was found */
struct cvt_builder
{
    template <typename From, typename To>
    struct apply : cvt_resolver<cvt_finish_quark_t<From>, cvt_start_quark_t<To>> {};
};

template <typename StartQuarkT, typename NextQuarkT>
requires(std::is_base_of_v<quark_base, std::remove_cvref_t<StartQuarkT>>&& std::is_base_of_v<quark_base, std::remove_cvref_t<NextQuarkT>>)
auto operator | (StartQuarkT s, NextQuarkT n)
{
    using cvt_tuple_t = std::tuple<typename cvt_builder::apply<StartQuarkT, NextQuarkT>::type>;
    return make_quark_chain<cvt_tuple_t>(std::tuple{ std::move(s), std::move(n) });
}

template <typename CvtTupleT, typename QuarkTupleT, typename NextQuarkT>
requires(std::is_base_of_v<quark_base, std::remove_cvref_t<NextQuarkT>>)
auto operator | (quark_chain<CvtTupleT, QuarkTupleT> && s, NextQuarkT n)
{
    using next_cvt_tuple_t = std::tuple<
        typename cvt_builder::apply<
            std::tuple_element_t<std::tuple_size_v<QuarkTupleT> - 1, QuarkTupleT>,
            NextQuarkT
        >::type>;
    using built_cvt_tuple_t = decltype(std::tuple_cat(std::declval<CvtTupleT>(), std::declval<next_cvt_tuple_t>()) );
    return make_quark_chain<built_cvt_tuple_t>(std::tuple_cat(s.quarks, std::tuple{n}));
}

template <typename StartQuarkT, typename NextQuarkT>
requires(std::is_base_of_v<quark_base, std::remove_cvref_t<StartQuarkT>> && std::is_base_of_v<quark_base, std::remove_cvref_t<NextQuarkT>>)
auto operator / (StartQuarkT s, NextQuarkT n)
{
    return compound_qrk<StartQuarkT, NextQuarkT>{ std::move(s), std::move(n) };
}

template <typename StartQuarkT, typename SomeQuarkT, typename NextQuarkT>
requires(std::is_base_of_v<quark_base, std::remove_cvref_t<NextQuarkT>>)
auto operator / (compound_qrk<StartQuarkT, SomeQuarkT> s, NextQuarkT n)
{
    return compound_qrk<StartQuarkT, NextQuarkT>{ std::move(s.start), std::move(n) };
}

}

#define RETURN_CALL_OR_DEFAULT_ON_VOID(fcall, dv) \
if constexpr(std::is_void_v<decltype(fcall)>) {   \
    fcall; return dv;                             \
} else {                                          \
    return fcall;                                 \
}

#include "basic/i_to_le.hpp"
#include "basic/le_to_i.hpp"
#include "basic/i_to_be.hpp"
#include "basic/be_to_i.hpp"
#include "basic/enumerator_pusher.hpp"
