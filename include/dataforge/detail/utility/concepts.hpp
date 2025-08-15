/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <type_traits>
#include <concepts>
#include <cstdint>

namespace dataforge {

template <size_t N> struct select_int;
template <> struct select_int<8>
{
    using char_type = char;
    using signed_type = int_least8_t;
    using unsigned_type = uint_least8_t;
};

template <> struct select_int<16>
{
    using char_type = char16_t;
    using signed_type = int_least16_t;
    using unsigned_type = uint_least16_t;
};

template <> struct select_int<32>
{
    using char_type = char32_t;
    using signed_type = int_least32_t;
    using unsigned_type = uint_least32_t;
};

template <> struct select_int<64>
{
    using char_type = void;
    using signed_type = int_least64_t;
    using unsigned_type = uint_least64_t;
};

template <typename T, size_t SzV>
concept Integral = std::is_integral_v<T> && sizeof(typename select_int<SzV>::unsigned_type) == sizeof(T);

template <typename T, size_t SzV>
concept IntegralAtLeast = sizeof(T) * CHAR_BIT >= SzV && std::is_integral_v<T>;

template <typename T> struct is_span : std::false_type {};
template <typename T, size_t E> struct is_span<std::span<T, E>> : std::true_type {};
template <typename T> constexpr bool is_span_v = is_span<T>::value;
template <typename T> concept Span = is_span_v<T>;

template <typename T, typename ET> struct is_span_of : std::false_type {};
template <typename ET, size_t E> struct is_span_of<std::span<ET, E>, ET> : std::true_type {};
template <typename ET, size_t E> struct is_span_of<std::span<const ET, E>, ET> : std::true_type {};
template <typename T, typename ET> constexpr bool is_span_of_v = is_span_of<T, ET>::value;
template <typename T, typename ET> concept SpanOf = is_span_of_v<T, ET>;

template <typename T, size_t BitsV> struct is_span_of_integrals : std::false_type {};
template <size_t BitsV, IntegralAtLeast<BitsV> T, size_t E> struct is_span_of_integrals<std::span<T, E>, BitsV> : std::true_type {};
template <typename T, size_t BitsV> constexpr bool is_span_of_integrals_v = is_span_of_integrals<T, BitsV>::value;
template <typename T, size_t BitsV> concept SpanOfIntegrals = is_span_of_integrals_v<T, BitsV>;

template <typename T, typename EqT> struct is_compatible_span : std::false_type {};
template <std::integral ET, size_t E, std::integral EqT> struct is_compatible_span<std::span<ET, E>, EqT> : std::bool_constant<sizeof(ET) == sizeof(EqT)> {};
template <typename T, typename EqT> constexpr bool is_compatible_span_v = is_compatible_span<T, EqT>::value;
template <typename T, typename EqT> concept CompatibleSpan = is_compatible_span_v<T, EqT>;

template <typename T>
concept SpanConvertible = requires(T const& a)
{
    { std::span{ a } };
};

template <typename T> struct is_span_convertible : std::false_type {};
template <SpanConvertible T> struct is_span_convertible<T> : std::true_type {};
template <typename T> constexpr bool is_span_convertible_v = is_span_convertible<T>::value;

template <typename T> struct is_reference_wrapper : std::false_type {};
template <typename T> struct is_reference_wrapper<std::reference_wrapper<T>> : std::true_type {};
template <typename T> constexpr bool is_reference_wrapper_v = is_reference_wrapper<T>::value;

}
