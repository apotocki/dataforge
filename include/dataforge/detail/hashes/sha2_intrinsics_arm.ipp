/*=============================================================================
    Copyright (c) Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_SHA256_ACCEL_CAN_COMPILE_ARM_SHA2

#include <arm_neon.h>

#if DATAFORGE_SHA256_ACCEL_AUTODETECT && defined(__linux__) && (defined(__aarch64__) || defined(__arm__))
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif

namespace dataforge::sha2_detail {

#if DATAFORGE_SHA256_ACCEL_IMPL == DATAFORGE_SHA256_ACCEL_AUTODETECT_MODE
inline bool sha256_runtime_has_sha256_accel()
{
#if defined(__linux__) && (defined(__aarch64__) || defined(__arm__))
#ifndef HWCAP_SHA2
#define HWCAP_SHA2 (1UL << 3)
#endif
#ifndef AT_HWCAP
#define AT_HWCAP 16
#endif
    return (getauxval(AT_HWCAP) & HWCAP_SHA2) != 0;
#else
    return true;
#endif
}
#endif

inline uint32_t sha256_rotr_arm(uint32_t value, int shift)
{
    return (value >> shift) | (value << (32 - shift));
}

inline uint32_t sha256_sigma0_arm(uint32_t value)
{
    return sha256_rotr_arm(value, 7) ^ sha256_rotr_arm(value, 18) ^ (value >> 3);
}

inline uint32_t sha256_sigma1_arm(uint32_t value)
{
    return sha256_rotr_arm(value, 17) ^ sha256_rotr_arm(value, 19) ^ (value >> 10);
}

inline uint32_t sha256_load_be32_arm(const uint8_t* src)
{
    return (static_cast<uint32_t>(src[0]) << 24)
        | (static_cast<uint32_t>(src[1]) << 16)
        | (static_cast<uint32_t>(src[2]) << 8)
        | static_cast<uint32_t>(src[3]);
}

inline void process_block_sha256_arm(uint32_t state[8], const void* msg)
{
    uint32_t W[64];
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    for (int t = 0; t < 16; ++t)
        W[t] = sha256_load_be32_arm(data + 4 * t);

    for (int t = 16; t < 64; ++t)
        W[t] = sha256_sigma1_arm(W[t - 2]) + W[t - 7] + sha256_sigma0_arm(W[t - 15]) + W[t - 16];

    uint32x4_t state0 = { state[0], state[1], state[2], state[3] };
    uint32x4_t state1 = { state[4], state[5], state[6], state[7] };
    const uint32x4_t state0_save = state0;
    const uint32x4_t state1_save = state1;

    for (int t = 0; t < 64; t += 4)
    {
        uint32_t wk_words[4] = {
            W[t] + sha2_def_base<256>::K[t],
            W[t + 1] + sha2_def_base<256>::K[t + 1],
            W[t + 2] + sha2_def_base<256>::K[t + 2],
            W[t + 3] + sha2_def_base<256>::K[t + 3]
        };

        const uint32x4_t wk = vld1q_u32(wk_words);
        const uint32x4_t tmp = state0;

        state0 = vsha256hq_u32(state0, state1, wk);
        state1 = vsha256h2q_u32(state1, tmp, wk);
    }

    state0 = vaddq_u32(state0, state0_save);
    state1 = vaddq_u32(state1, state1_save);

    vst1q_u32(&state[0], state0);
    vst1q_u32(&state[4], state1);
}

}

#endif
