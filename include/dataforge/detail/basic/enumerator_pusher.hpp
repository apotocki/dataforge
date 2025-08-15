/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <typename T>
class enumerator_pusher : public generic_pusher<void>
{
    size_t numerator_{ 0 };

public:
    using input_element_type = T;

    template <typename SrcTagT, typename ErrorHandlerT>
    inline enumerator_pusher(SrcTagT const&, enum_qrk<ErrorHandlerT> const&) noexcept
    {}

    template <CompatibleSpan<T> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT cons)
    {
        for (const auto ival : ivals) {
            cons(std::pair{ numerator_++, ival });
        }
    }

    template <typename ConsumerT>
    inline void push(const T ival, ConsumerT cons)
    {
        cons(std::pair{ numerator_++, ival });
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept { reset(); }

    inline void reset() noexcept { numerator_ = 0; }
};

class denumerator_pusher : public generic_pusher<void>
{
public:
    template <typename ErrorHandlerT, typename DestTagT>
    inline denumerator_pusher(enum_qrk<ErrorHandlerT> const&, DestTagT const&) noexcept
    {}

    template <typename T, SpanOf<std::pair<size_t, T>> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT cons)
    {
        for (const auto ival : ivals) {
            cons(ival);
        }
    }

    template <typename T, typename ConsumerT>
    inline void push(const std::pair<size_t, T> ival, ConsumerT cons)
    {
        cons(ival.second);
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT) noexcept { }

    inline void reset() noexcept { }
};

template <SomeIntegralBasedQuark FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, enum_qrk<ToEHT>>
{
    using type = enumerator_pusher<typename FromQuarkT::integral_type>;
};

template <typename FromEHT, typename DestTagT>
struct cvt_resolver<enum_qrk<FromEHT>, DestTagT>
{
    using type = denumerator_pusher;
};

}
