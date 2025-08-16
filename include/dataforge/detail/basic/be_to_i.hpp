/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

template <std::integral IT, size_t ByteCountV, typename ErrorHandlerT>
class be_to_i
    : public generic_pusher<ErrorHandlerT>
    , public basic_puller<be_to_i<IT, ByteCountV, ErrorHandlerT>, char, IT, 1>
{
    using base_t = generic_pusher<ErrorHandlerT>;
    using base_t::on_error;

    using it_t = typename select_int<ByteCountV * 8>::unsigned_type;

    it_t data : (sizeof(it_t) - 1) * 8;
    it_t num : 8;

public:
    template <typename DestT>
    be_to_i(be_qrk<ErrorHandlerT> const& quark, DestT const&)
        : base_t{ quark }, data{0}, num{0}
    {}

    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ivals, ConsumerT && cons)
    {
        for (auto ival : ivals) {
            push(ival, std::forward<ConsumerT>(cons));
        }
    }

    template <IntegralAtLeast<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT cons)
    {
        it_t result = data;
        result <<= 8;
        result |= static_cast<it_t>(ival) & 0xff;
        
        if (num < ByteCountV - 1) {
            data = result;
            ++num;
        } else {
            cons(static_cast<IT>(result));
            data = num = 0;
        }
    }

    template <typename ConsumerT>
    inline void finish(ConsumerT)
    {
        if (num) {
            on_error("unfinished be sequence", *this);
        }
    }

    inline void reset() noexcept { num = 0; data = 0; }
};

// trivial coverter
template <std::integral IT, typename ErrorHandlerT>
class be_to_i<IT, 1, ErrorHandlerT> : public generic_pusher<ErrorHandlerT>
{
    using base_t = generic_pusher<ErrorHandlerT>;

public:
    template <typename DestT>
    explicit be_to_i(be_qrk<ErrorHandlerT> const& quark, DestT const&)
        : base_t{ quark }
    {}

    template <typename EIT, size_t EV, typename ConsumerT>
    void push(std::span<EIT, EV> ivals, ConsumerT cons)
    {
        cons(std::span<const IT, EV>{reinterpret_cast<const IT*>(ivals.data()), ivals.size()});
    }

    template <typename EIT, typename ConsumerT>
    void push(const EIT ival, ConsumerT cons)
    {
        cons(static_cast<IT>(ival));
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

template <typename FromEHT, SomeIntegralBasedQuark ToQuarkT>
struct cvt_resolver<be_qrk<FromEHT>, ToQuarkT>
{
    using type = be_to_i<typename ToQuarkT::integral_type, ToQuarkT::int_qrk_t::bits / 8, FromEHT>;
};

}
