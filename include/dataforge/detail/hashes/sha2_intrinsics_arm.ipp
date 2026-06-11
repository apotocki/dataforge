/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA2

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC target("+sha2")
#elif defined(__clang__)
#pragma clang attribute push(__attribute__((target("arch=armv8-a+sha2"))), apply_to=function)
#endif

#include <arm_neon.h>

#if DATAFORGE_ACCEL_AUTODETECT && defined(__linux__) && (defined(__aarch64__) || defined(__arm__))
#include <sys/auxv.h>
#include <asm/hwcap.h>
#endif

namespace dataforge::sha2_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
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

inline void process_block_sha256_arm(uint32_t(&state)[8], const void* msg)
{
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    // state[] is {A,B,C,D,E,F,G,H};

    uint32x4_t state0 = vld1q_u32(&state[0]); /* ABCD */
    uint32x4_t state1 = vld1q_u32(&state[4]); /* EFGH */

    const uint32x4_t abcd = state0;
    const uint32x4_t efgh = state1;

    /* Load and byte-swap message words (big-endian -> host) */
#ifdef __aarch64__
    static const uint8x16_t rev_mask = { 3,2,1,0, 7,6,5,4, 11,10,9,8, 15,14,13,12 };
    uint32x4_t msg0 = vreinterpretq_u32_u8(vqtbl1q_u8(vld1q_u8(data +  0), rev_mask));
    uint32x4_t msg1 = vreinterpretq_u32_u8(vqtbl1q_u8(vld1q_u8(data + 16), rev_mask));
    uint32x4_t msg2 = vreinterpretq_u32_u8(vqtbl1q_u8(vld1q_u8(data + 32), rev_mask));
    uint32x4_t msg3 = vreinterpretq_u32_u8(vqtbl1q_u8(vld1q_u8(data + 48), rev_mask));
#else
    uint32x4_t msg0 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data +  0)));
    uint32x4_t msg1 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 16)));
    uint32x4_t msg2 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 32)));
    uint32x4_t msg3 = vreinterpretq_u32_u8(vrev32q_u8(vld1q_u8(data + 48)));
#endif

    /* K is aligned (alignas(16) in sha2_def_base<256>), indexed as uint32x4_t */
    const uint32_t* K = sha2_def_base<256>::K;

    uint32x4_t tmp, t;

    /* Rounds 0-3 */
    tmp = vaddq_u32(msg0, vld1q_u32(K));
    t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);
    msg0 = vsha256su0q_u32(msg0, msg1); /* begin W[4..7] schedule */

    /* Rounds 4-7 */
    tmp = vaddq_u32(msg1, vld1q_u32(K + 4));
    t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);
    msg1 = vsha256su0q_u32(msg1, msg2); /* begin W[8..11] schedule */

    /* Rounds 8..55: 3 iterations, each processing 16 rounds (4 groups of 4)
       msg schedule: su0 started previous iteration, su1 completes it here */
    for (int i = 8; i <= 40; i += 16) {
        /* Rounds +0..+3 */
        msg0 = vsha256su1q_u32(msg0, msg2, msg3); /* complete W[i..i+3] */
        tmp = vaddq_u32(msg2, vld1q_u32(K + i));
        t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);
        msg2 = vsha256su0q_u32(msg2, msg3);

        /* Rounds +4..+7 */
        msg1 = vsha256su1q_u32(msg1, msg3, msg0);
        tmp = vaddq_u32(msg3, vld1q_u32(K + i + 4));
        t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);
        msg3 = vsha256su0q_u32(msg3, msg0);

        /* Rounds +8..+11 */
        msg2 = vsha256su1q_u32(msg2, msg0, msg1);
        tmp = vaddq_u32(msg0, vld1q_u32(K + i + 8));
        t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);
        msg0 = vsha256su0q_u32(msg0, msg1);

        /* Rounds +12..+15 */
        msg3 = vsha256su1q_u32(msg3, msg1, msg2);
        tmp = vaddq_u32(msg1, vld1q_u32(K + i + 12));
        t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);

        if (i == 40) break; // after round 55: su0 not needed for next schedule
        msg1 = vsha256su0q_u32(msg1, msg2);
    }

    /* Rounds 56-59 */
    msg0 = vsha256su1q_u32(msg0, msg2, msg3); /* complete W[56..59] */
    tmp = vaddq_u32(msg2, vld1q_u32(K + 56));
    t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);

    /* Rounds 60-63 */
    msg1 = vsha256su1q_u32(msg1, msg3, msg0); /* complete W[60..63] */
    tmp = vaddq_u32(msg3, vld1q_u32(K + 60));
    t = state0; state0 = vsha256hq_u32(state0, state1, tmp); state1 = vsha256h2q_u32(state1, t, tmp);

    state0 = vaddq_u32(state0, abcd);
    state1 = vaddq_u32(state1, efgh);

    vst1q_u32(&state[0], state0);
    vst1q_u32(&state[4], state1);
}

}

#if defined(__clang__)
#pragma clang attribute pop
#endif

#endif
