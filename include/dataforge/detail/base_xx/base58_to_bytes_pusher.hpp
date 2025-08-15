/*=============================================================================
    Copyright (c) 2023 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include <utility>

#include "../hashes/sha2.hpp"

#include "generic_base_to_bytes_pusher.hpp"

namespace dataforge {

template <std::integral BufferElementT, std::integral DblBufferElementT, typename ErrorHandlerT>
class base58check_to_bytes_pusher : public generic_base_to_bytes_pusher<BufferElementT, DblBufferElementT, ErrorHandlerT>
{
    using base_t = generic_base_to_bytes_pusher<BufferElementT, DblBufferElementT, ErrorHandlerT>;

public:
    using base_t::base_t;

    template <typename ConsumerT>
    struct consumer_wrapper
    {
        sha2_detail::sha2_impl<sha2_type::sha256>& sha2;
        ConsumerT& cons;
        
        consumer_wrapper(sha2_detail::sha2_impl<sha2_type::sha256>& alg, ConsumerT& c)
            : sha2{ alg }, cons{ c }
        {}

        void operator()(char val) const
        {
            sha2.input(&val, 1);
            cons(val);
        }

        template <size_t E>
        void operator()(std::span<const char, E> s) const
        {
            sha2.input(s.data(), s.size());
            cons(s);
        }
    };

    template <typename ConsumerT>
    void finish(ConsumerT && cons)
    {
        base_t::handle_input();

        if (this->output.size() * sizeof(BufferElementT) < 4) {
            throw std::runtime_error("base58 check error, the data is too small");
        }

        sha2_detail::sha2_impl<sha2_type::sha256> sha2;

        char zval = 0;
        for (uint32_t zc = 0; zc < this->zeros_; ++zc) {
            sha2.input(&zval, 1);
        }

        static_assert(sizeof(BufferElementT) <= 4, "not implemented case");
        auto bit = this->output.begin() + 4 / sizeof(BufferElementT), it = this->output.end();;
        
        if (bit != it) {
            --it;
            base_t::finish_impl(bit, it, consumer_wrapper<ConsumerT>{ sha2, cons });
        }

        sha2.finalize();
        uint8_t digest[32];
        T_to_be(digest, sha2.digest_span().data(), 8);
        sha2.reset();
        sha2.input(&digest, 32);
        sha2.finalize();

        uint32_t caclc = sha2.digest_span()[0];
        uint32_t check = 0;
        bit = this->output.begin() + 4 / sizeof(BufferElementT);
        for (int i = 0; i < 4 / sizeof(BufferElementT); ++i) {
            --bit;
            check <<= (8 * sizeof(BufferElementT)) % sizeof(uint32_t);
            check |= *bit;
        }

        if (caclc != check) {
            throw std::runtime_error("base58 check error, wrong check code");
        }

        base_t::reset();
    }
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<base58_qrk<false, FromEHT>, ToQuarkT>
{
    using type = generic_base_to_bytes_pusher<uint32_t, uint64_t, FromEHT>;
};

template <typename FromEHT, IntegralBasedQuark<8> ToQuarkT>
struct cvt_resolver<base58_qrk<true, FromEHT>, ToQuarkT>
{
    using type = base58check_to_bytes_pusher<uint32_t, uint64_t, FromEHT>;
    //using type = base58check_to_bytes_pusher<uint16_t, uint32_t, FromEHT>;
    //using type = base58check_to_bytes_pusher<uint8_t, uint16_t, FromEHT>;
};

}
