/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <bit>

namespace dataforge {

template <std::integral IT, size_t ByteCountV>
class i_to_le
    : public generic_pusher<void>
    , public basic_puller<i_to_le<IT, ByteCountV>, IT, char, ByteCountV>
{
public:
    template <typename SrcTagT, typename ErrorHandlerT>
    inline i_to_le(SrcTagT const&, le_qrk<ErrorHandlerT> const&) noexcept
    {}

    template <SpanOf<IT> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT cons)
    {
        for (IT ival : ivals) {
            push(ival, cons);
        }
    }

    template <typename ConsumerT>
    void push(const IT ival, ConsumerT cons)
    {
        if constexpr (std::endian::native == std::endian::little) {
            cons(std::span<const char, ByteCountV>{ reinterpret_cast<const char*>(&ival), ByteCountV });
        } else {
            char values[ByteCountV];
            for (int i = 0; i < ByteCountV; ++i) {
                values[i] = static_cast<char>(ival & 0xff);
                ival >>= 8;
            }
            cons(std::span<const char, ByteCountV>{ values, ByteCountV });
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept {}

    inline void reset() noexcept {}
};

template <std::integral IT>
class i_to_le<IT, 1> : public generic_pusher<void>
{
public:
    template <typename SrcTagT, typename ErrorHandlerT>
    inline i_to_le(SrcTagT const&, le_qrk<ErrorHandlerT> const&) noexcept
    {}

    template <SpanOf<IT> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT && cons)
    {
        std::forward<ConsumerT>(cons)(ivals);
    }

    template <typename ConsumerT>
    inline void push(const IT ival, ConsumerT && cons)
    {
        std::forward<ConsumerT>(cons)(ival);
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept {}

    inline void reset() noexcept {}

    using input_element_type = IT;
    using output_element_type = IT;

    template <typename ProviderT>
    std::span<const IT> pull(std::span<const IT>&, ProviderT p)
    {
        return span_cast<const IT>(p());
    }
};

template <SomeIntegralBasedQuark FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, le_qrk<ToEHT>>
{
    using type = i_to_le<typename FromQuarkT::integral_type, FromQuarkT::int_qrk_t::bits / 8>;
};

}
