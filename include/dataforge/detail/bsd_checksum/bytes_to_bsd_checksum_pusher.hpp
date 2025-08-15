/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

namespace dataforge {

class bytes_to_bsd_checksum_pusher
    : public generic_pusher<void>
{
    uint_least32_t checksum{ 0 };

public:
    using input_element_type = unsigned char;
    using output_element_type = uint_least32_t;

    template <typename SrcTagT>
    bytes_to_bsd_checksum_pusher(SrcTagT const&, bsd_checksum_qrk const&)
    {

    }

    template <CompatibleSpan<char> SpanT, typename ConsumerT>
    inline void push(SpanT ivals, ConsumerT&& cons)
    {
        for (auto ival : ivals) {
            push(ival, cons);
        }
    }

    template <std::integral LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT&&)
    {
        checksum = (checksum >> 1) + ((checksum & 1) << 15);
        checksum += ival;
        checksum &= 0xffff;
    }

    template <typename ConsumerT>
    void finish(ConsumerT cons)
    {
        cons(checksum);
        checksum = 0;
    }

    inline void reset() noexcept
    {
        checksum = 0;
    }
};

template <typename FromEHT>
struct cvt_resolver<int_qrk<8, FromEHT>, bsd_checksum_qrk>
{
    using type = bytes_to_bsd_checksum_pusher;
};

}
