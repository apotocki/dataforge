/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <memory>
#include <utility>

#include "rc4.hpp"

namespace dataforge {

class rc4_crypter
    : public generic_pusher<void>
{
    struct algo_destroyer
    {
        void operator()(rc4_detail::rc4_impl * pobj) const
        {
            pobj->destroy();
        }
    };

    std::shared_ptr<rc4_detail::rc4_impl> algo;

public:
    using input_element_type = unsigned char;
    using output_element_type = unsigned char;

    template <typename SrcQrkT>
    rc4_crypter(SrcQrkT const&, rc4_qrk const& q)
    {
        algo.reset(rc4_detail::rc4_impl::create(q.key, q.n, q.skipsz), algo_destroyer{});
    }

    template <typename DestQrkT>
    rc4_crypter(rc4_qrk const& q, DestQrkT const&)
    {
        algo.reset(rc4_detail::rc4_impl::create(q.key, q.n, q.skipsz), algo_destroyer{});
    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&& cons)
    {
        algo->process_data(std::span{ reinterpret_cast<const unsigned char*>(ivals.data()), ivals.size() }, std::forward<ConsumerT>(cons));
    }

    template <Integral<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&& cons)
    {
        push(std::span{&ival, 1}, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    void finish(ConsumerT)
    {
        algo->reset();
    }

    output_element_type cache[16];
    size_t cache_sz = 0;
    template <typename ProviderT>
    std::span<const output_element_type> pull(std::span<const input_element_type>& input, ProviderT p)
    {
        for (;;) {
            if (input.empty()) {
                input = span_cast<const input_element_type>(p());
                if (input.empty()) {
                    return {};
                }
            }
            cache_sz = 0;
            auto thr = (std::min)(sizeof(cache), input.size());
            algo->process_data(std::span{ input.data(), thr }, [this](output_element_type b) {
                cache[cache_sz++] = b;
            });
            input = input.subspan(thr);
            return std::span{ cache, cache_sz };
        }
    }

    inline void reset() { algo->reset(); }
};

template <IntegralBasedQuark<8> FromQrkT>
struct cvt_resolver<FromQrkT, rc4_qrk>
{
    using type = rc4_crypter;
};

template <IntegralBasedQuark<8> ToQrkT>
struct cvt_resolver<rc4_qrk, ToQrkT>
{
    using type = rc4_crypter;
};

}
