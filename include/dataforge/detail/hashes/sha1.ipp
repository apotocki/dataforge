/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#include <cstring>

#if DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1
#   include "sha1_intrinsics_x86.ipp"
#elif DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1
#   include "sha1_intrinsics_arm.ipp"
#endif

namespace dataforge::sha1_detail {

inline const uint_least32_t init_values[5] =
{
    0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0
};

inline const uint_least32_t K[4] = {
    0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6
};

inline sha1_impl::sha1_impl()
{
    std::memcpy(H, init_values, sizeof(H));
}

inline void sha1_impl::reset()
{
    std::memcpy(H, init_values, sizeof(H));
    bit_count = 0;
}

inline void sha1_impl::process_blocks(const void* msg, size_t block_count)
{
#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
    using sha1_block_fn_t = void(*)(word_type(&)[state_size], const void*, size_t);
    static const sha1_block_fn_t process_blocks_impl = []() -> sha1_block_fn_t {
#   if DATAFORGE_TARGET_X86 && DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1
        if (sha1_runtime_has_sha1_accel())
            return process_blocks_sha1_x86;
        return &sha1_impl::process_blocks_scalar;
#   elif DATAFORGE_TARGET_ARM && DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1
        if (sha1_runtime_has_sha1_accel())
            return &process_blocks_sha1_arm;
        return &sha1_impl::process_blocks_scalar;
#   else
        return &sha1_impl::process_blocks_scalar;
#   endif
    }();
    process_blocks_impl(H, msg, block_count);
#elif DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_X86
    process_blocks_sha1_x86(H, msg, block_count);
#elif DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_ARM
#   if DATAFORGE_ACCEL_ARM_USE_CRYPTO && DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1
    process_blocks_sha1_arm(H, msg, block_count);
#   else
    process_blocks_scalar(H, msg, block_count);
#   endif
#else // DATAFORGE_ACCEL_NONE
    process_blocks_scalar(H, msg, block_count);
#endif
}

// processes one chunk of 64 bytes 
void sha1_impl::process_blocks_scalar(word_type(&state)[state_size], const void* msg, size_t block_count) noexcept
{
    word_type Ws[80];
    for (;;) {
        word_type* W = be_to_T<8, 32>(Ws, msg, block_size);

        for (int t = 16; t < 80; ++t)
            W[t] = left_rotate<32>(W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16], 1);

        word_type a = state[0];
        word_type b = state[1];
        word_type c = state[2];
        word_type d = state[3];
        word_type e = state[4];

        for (int t = 0; t < 20; ++t)
        {
            word_type T = left_rotate<32>(a, 5) + Ch(b, c, d) + e + K[0] + W[t];
            e = d;
            d = c;
            c = left_rotate<32>(b, 30);
            b = a;
            a = T;
        }
        for (int t = 20; t < 40; ++t)
        {
            word_type T = left_rotate<32>(a, 5) + Parity(b, c, d) + e + K[1] + W[t];
            e = d;
            d = c;
            c = left_rotate<32>(b, 30);
            b = a;
            a = T;
        }
        for (int t = 40; t < 60; ++t)
        {
            word_type T = left_rotate<32>(a, 5) + Maj(b, c, d) + e + K[2] + W[t];
            e = d;
            d = c;
            c = left_rotate<32>(b, 30);
            b = a;
            a = T;
        }
        for (int t = 60; t < 80; ++t)
        {
            word_type T = left_rotate<32>(a, 5) + Parity(b, c, d) + e + K[3] + W[t];
            e = d;
            d = c;
            c = left_rotate<32>(b, 30);
            b = a;
            a = T;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;

        if (--block_count == 0) break;
        msg = static_cast<const char*>(msg) + block_size;
    }
}

inline void sha1_impl::store_bit_count(void* dst) const
{
    bit_count.store_as_big_endian(dst, 1);
}

}

