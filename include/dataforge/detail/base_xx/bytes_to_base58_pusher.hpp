/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <utility>

#include "../hashes/sha2.hpp"

#include "bytes_to_generic_base_pusher.hpp"

namespace dataforge {

template <std::integral BufferElementT, std::integral DblBufferElementT>
class bytes_to_base58check_pusher : public bytes_to_generic_base_pusher<BufferElementT, DblBufferElementT>
{
    using base_t = bytes_to_generic_base_pusher<BufferElementT, DblBufferElementT>;
    sha2_detail::sha2_impl<sha2_type::sha256> sha2;

public:
    using base_t::base_t;
    
    template <SpanOfIntegrals<8> SpanT, typename ConsumerT>
    void push(SpanT ispan, ConsumerT && cons)
    {
        sha2.input(ispan.data(), ispan.size());
        base_t::push(ispan, std::forward<ConsumerT>(cons));
    }

    template <Integral<8> LEIT, typename ConsumerT>
    void push(const LEIT ival, ConsumerT && cons)
    {
        sha2.input(&ival, 1);
        base_t::push(ival, std::forward<ConsumerT>(cons));
    }

    template <typename ConsumerT>
    void finish(ConsumerT && cons)
    {
        sha2.finalize();
        uint8_t digest[32];
        T_to_be(digest, sha2.digest_span().data(), 8);
        sha2.reset();
        sha2.input(&digest, 32);
        sha2.finalize();
        if constexpr (std::endian::native == std::endian::big) {
            base_t::push(std::span{ reinterpret_cast<const uint8_t*>(&sha2.digest_span()[0]), 4 }, std::forward<ConsumerT>(cons));
        } else {
            for (int i = 3; i >= 0; --i) {
                base_t::push(static_cast<uint8_t>(sha2.digest_span()[0] >> i * 8), std::forward<ConsumerT>(cons));
            }
        }
        base_t::finish(std::forward<ConsumerT>(cons));
        sha2.reset();
    }

    void reset()
    {
        sha2.reset();
        base_t::reset();
    }
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, base58_qrk<false, ToEHT>>
{
    using type = bytes_to_generic_base_pusher<uint32_t, uint64_t>;
};

template <IntegralBasedQuark<8> FromQuarkT, typename ToEHT>
struct cvt_resolver<FromQuarkT, base58_qrk<true, ToEHT>>
{
    using type = bytes_to_base58check_pusher<uint32_t, uint64_t>;
    //using type = bytes_to_base58_pusher<uint16_t, uint32_t>;
    //using type = bytes_to_base58_pusher<uint8_t, uint16_t>;
};

}
