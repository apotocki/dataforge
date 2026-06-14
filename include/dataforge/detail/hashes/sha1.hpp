/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#include "dataforge/detail/config.hpp"

#include "../utility/digest_base.hpp"

#if DATAFORGE_TARGET_X86
#define DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1 1
#else
#define DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1 0
#endif

#if DATAFORGE_TARGET_ARM64
#define DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1 1
#else
#define DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1 0
#endif

namespace dataforge {

enum class sha1_type : int { sha1 };

namespace sha1_detail {

struct sha1_impl : digest_base<sha1_impl, 64>
{
    static constexpr size_t digest_length() { return 20; }
    static constexpr std::endian digest_endianness() { return std::endian::big; }
    static constexpr int input_length_size = 8;

    using word_type = uint_least32_t;
    using digest_word_type = word_type;
    static constexpr size_t digest_word_bit_count = 32;

    static constexpr size_t state_size = 5;

    sha1_impl();

    void reset();

    void process_block(const void* msg, size_t block_count);
    void store_bit_count(void* dst) const;
    
    static void process_block_scalar(word_type(&state)[state_size], const void* msg, size_t block_count) noexcept;

    inline std::span<digest_word_type, state_size> digest_span() { return H; }

private:
    static inline word_type Ch(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (~x & z);
    }

    static inline word_type Parity(word_type x, word_type y, word_type z)
    {
        return x ^ y ^ z;
    }

    static inline uint32_t Maj(word_type x, word_type y, word_type z)
    {
        return (x & y) ^ (x & z) ^ (y & z);
    }

    digest_word_type H[state_size];
};

}}

#include "sha1.ipp"
