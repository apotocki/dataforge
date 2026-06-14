/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1

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

namespace dataforge::sha1_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
inline bool sha1_runtime_has_sha1_accel()
{
#if defined(__linux__) && (defined(__aarch64__) || defined(__arm__))
#ifndef HWCAP_SHA1
#define HWCAP_SHA1 (1UL << 2)
#endif
#ifndef AT_HWCAP
#define AT_HWCAP 16
#endif
    return (getauxval(AT_HWCAP) & HWCAP_SHA1) != 0;
#else
    return true;
#endif
}
#endif // DATAFORGE_ACCEL_AUTODETECT_MODE

// --------------------------------------------------------------------------
// SHA-1 ARMv8 Crypto Extension backend.
//
// State layout:  state[] = {A, B, C, D, E}  (5 x uint32).
// ARM intrinsics expect:
//   abcd  = { D, C, B, A }  in lane order [0..3]  (vld1q_u32(&state[0])
//             gives this directly since AArch64 is little-endian)
//   e     = state[4] held in lane 0 of a uint32x4_t (upper lanes unused)
//
// Message words: SHA-1 is big-endian; we byte-swap each 32-bit word on load.
//
// vsha1h_u32(e)  -> rotate e left by 30  (produces next E)
// vsha1cq_u32 / vsha1pq_u32 / vsha1mq_u32  -> four rounds each
// vsha1su0q_u32 / vsha1su1q_u32  -> message schedule
// --------------------------------------------------------------------------
inline void process_block_sha1_arm(uint_least32_t(&state)[5], const void* msg, size_t block_count)
{
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    /* Load state */
    uint32x4_t abcd = vld1q_u32(&state[0]);   /* {A,B,C,D} */
    uint32_t   e_scalar = state[4];

    uint32x4_t tmp;
    uint32_t   e0, e1;

    for (;;) {
        const uint32x4_t abcd_save = abcd;
        const uint32_t   e_save    = e_scalar;

        /* Byte-swap message words (big-endian input) */
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

        /* Round constants */
        const uint32x4_t K0 = vdupq_n_u32(0x5a827999u); /* rounds  0-19 */
        const uint32x4_t K1 = vdupq_n_u32(0x6ed9eba1u); /* rounds 20-39 */
        const uint32x4_t K2 = vdupq_n_u32(0x8f1bbcdcu); /* rounds 40-59 */
        const uint32x4_t K3 = vdupq_n_u32(0xca62c1d6u); /* rounds 60-79 */

        /* Rounds 0-3: Ch, no schedule yet */
        tmp  = vaddq_u32(msg0, K0);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1cq_u32(abcd, e_scalar, tmp);

        /* Rounds 4-7 */
        tmp  = vaddq_u32(msg1, K0);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1cq_u32(abcd, e0, tmp);
        msg0 = vsha1su0q_u32(msg0, msg1, msg2);

        /* Rounds 8-11 */
        tmp  = vaddq_u32(msg2, K0);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1cq_u32(abcd, e1, tmp);
        msg0 = vsha1su1q_u32(msg0, msg3);
        msg1 = vsha1su0q_u32(msg1, msg2, msg3);

        /* Rounds 12-15 */
        tmp  = vaddq_u32(msg3, K0);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1cq_u32(abcd, e0, tmp);
        msg1 = vsha1su1q_u32(msg1, msg0);
        msg2 = vsha1su0q_u32(msg2, msg3, msg0);

        /* Rounds 16-19 */
        tmp  = vaddq_u32(msg0, K0);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1cq_u32(abcd, e1, tmp);
        msg2 = vsha1su1q_u32(msg2, msg1);
        msg3 = vsha1su0q_u32(msg3, msg0, msg1);

        /* Rounds 20-23: Parity */
        tmp  = vaddq_u32(msg1, K1);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);
        msg3 = vsha1su1q_u32(msg3, msg2);
        msg0 = vsha1su0q_u32(msg0, msg1, msg2);

        /* Rounds 24-27 */
        tmp  = vaddq_u32(msg2, K1);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e1, tmp);
        msg0 = vsha1su1q_u32(msg0, msg3);
        msg1 = vsha1su0q_u32(msg1, msg2, msg3);

        /* Rounds 28-31 */
        tmp  = vaddq_u32(msg3, K1);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);
        msg1 = vsha1su1q_u32(msg1, msg0);
        msg2 = vsha1su0q_u32(msg2, msg3, msg0);

        /* Rounds 32-35 */
        tmp  = vaddq_u32(msg0, K1);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e1, tmp);
        msg2 = vsha1su1q_u32(msg2, msg1);
        msg3 = vsha1su0q_u32(msg3, msg0, msg1);

        /* Rounds 36-39 */
        tmp  = vaddq_u32(msg1, K1);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);
        msg3 = vsha1su1q_u32(msg3, msg2);
        msg0 = vsha1su0q_u32(msg0, msg1, msg2);

        /* Rounds 40-43: Maj */
        tmp  = vaddq_u32(msg2, K2);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1mq_u32(abcd, e1, tmp);
        msg0 = vsha1su1q_u32(msg0, msg3);
        msg1 = vsha1su0q_u32(msg1, msg2, msg3);

        /* Rounds 44-47 */
        tmp  = vaddq_u32(msg3, K2);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1mq_u32(abcd, e0, tmp);
        msg1 = vsha1su1q_u32(msg1, msg0);
        msg2 = vsha1su0q_u32(msg2, msg3, msg0);

        /* Rounds 48-51 */
        tmp  = vaddq_u32(msg0, K2);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1mq_u32(abcd, e1, tmp);
        msg2 = vsha1su1q_u32(msg2, msg1);
        msg3 = vsha1su0q_u32(msg3, msg0, msg1);

        /* Rounds 52-55 */
        tmp  = vaddq_u32(msg1, K2);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1mq_u32(abcd, e0, tmp);
        msg3 = vsha1su1q_u32(msg3, msg2);
        msg0 = vsha1su0q_u32(msg0, msg1, msg2);

        /* Rounds 56-59 */
        tmp  = vaddq_u32(msg2, K2);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1mq_u32(abcd, e1, tmp);
        msg0 = vsha1su1q_u32(msg0, msg3);
        msg1 = vsha1su0q_u32(msg1, msg2, msg3);

        /* Rounds 60-63: Parity */
        tmp  = vaddq_u32(msg3, K3);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);
        msg1 = vsha1su1q_u32(msg1, msg0);
        msg2 = vsha1su0q_u32(msg2, msg3, msg0);

        /* Rounds 64-67 */
        tmp  = vaddq_u32(msg0, K3);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e1, tmp);
        msg2 = vsha1su1q_u32(msg2, msg1);
        msg3 = vsha1su0q_u32(msg3, msg0, msg1);

        /* Rounds 68-71 */
        tmp  = vaddq_u32(msg1, K3);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);
        msg3 = vsha1su1q_u32(msg3, msg2);

        /* Rounds 72-75 */
        tmp  = vaddq_u32(msg2, K3);
        e0   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e1, tmp);

        /* Rounds 76-79 */
        tmp  = vaddq_u32(msg3, K3);
        e1   = vsha1h_u32(vgetq_lane_u32(abcd, 0));
        abcd = vsha1pq_u32(abcd, e0, tmp);

        /* Accumulate */
        abcd = vaddq_u32(abcd, abcd_save);
        e1  += e_save;

        if (--block_count == 0) break;
        data += 64;
    }

    vst1q_u32(&state[0], abcd);
    state[4] = e1;
}

} // namespace dataforge::sha1_detail

#if defined(__clang__)
#pragma clang attribute pop
#endif

#endif // DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA1
