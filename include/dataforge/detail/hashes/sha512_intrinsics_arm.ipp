/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA512

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC target("+sha3")
#elif defined(__clang__)
#pragma clang attribute push(__attribute__((target("arch=armv8.2-a+sha3"))), apply_to=function)
#endif

#include <arm_neon.h>

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
#  if defined(__linux__) && defined(__aarch64__)
#    include <sys/auxv.h>
#    include <asm/hwcap.h>
#  elif defined(__APPLE__) && defined(__aarch64__)
#    include <sys/types.h>
#    include <sys/sysctl.h>
#  endif
#endif

namespace dataforge::sha2_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
inline bool sha512_runtime_has_sha512_accel()
{
#if defined(__linux__) && defined(__aarch64__)
    // SHA-512 (ARMv8.2-A) is reported via AT_HWCAP bit 21, not AT_HWCAP2.
#  ifndef HWCAP_SHA512
#    define HWCAP_SHA512 (1UL << 21)
#  endif
#  ifndef AT_HWCAP
#    define AT_HWCAP 16
#  endif
    return (getauxval(AT_HWCAP) & HWCAP_SHA512) != 0;
#elif defined(__APPLE__) && defined(__aarch64__)
    int val = 0;
    size_t sz = sizeof(val);
    return sysctlbyname("hw.optional.armv8_2_sha512", &val, &sz, nullptr, 0) == 0 && val != 0;
#else
    return false;
#endif
}
#endif // DATAFORGE_ACCEL_AUTODETECT_MODE

// --------------------------------------------------------------------------
// SHA-512 ARMv8.2 Crypto Extension backend.
//
// State layout: state[] = {A,B,C,D,E,F,G,H} (8 x uint64).
// Packed as four uint64x2_t: ab={A,B}, cd={C,D}, ef={E,F}, gh={G,H}.
//
// Message schedule: eight uint64x2_t w[0..7], each holding one word pair
// {W[2k], W[2k+1]}. Rounds 0-15 use loaded words directly. Rounds 16-79
// are 4 outer x 8 inner loops; each iteration updates w[i&7] in-place
// before use:
//   su0 = vsha512su0q_u64(w[i&7], w[(i+1)&7])
//     su0[0] = W[2i]   + sigma0(W[2i+1])
//     su0[1] = W[2i+1] + sigma0(W[2i+2])
//   w[i&7] = vsha512su1q_u64(su0, w[(i+7)&7], vextq_u64(w[(i+4)&7], w[(i+5)&7], 1))
//     where Vn=w[(i+7)&7] supplies sigma1(W[t-2]/W[t-1]) and
//     Vm=vextq_u64(w[(i+4)&7],w[(i+5)&7],1) supplies the W[t-7] addend.
//
// Two compression rounds per wk pair:
//   wk = vextq_u64(w[i] + K[2i], w[i] + K[2i], 1) + gh  // swap lanes, add gh
//   t  = vsha512hq_u64(wk, vextq_u64(ef, gh, 1), vextq_u64(cd, ef, 1))
//   t1 = vsha512h2q_u64(t, cd, ab)
//   gh=ef; ef=cd+t; cd=ab; ab=t1
// --------------------------------------------------------------------------

inline void process_blocks_sha512_arm(uint64_t(&state)[8], const void* msg, size_t block_count)
{
    const uint8x16_t BSWAP = { 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 };
    const uint64_t* K = sha2_def_base<512>::K;
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    uint64x2_t ab = vld1q_u64(&state[0]);
    uint64x2_t cd = vld1q_u64(&state[2]);
    uint64x2_t ef = vld1q_u64(&state[4]);
    uint64x2_t gh = vld1q_u64(&state[6]);
    for (;;) {
        const uint64x2_t ab0 = ab, cd0 = cd, ef0 = ef, gh0 = gh;

        // Load and byte-swap 128-byte message block into 8 x uint64x2_t
        uint64x2_t w[8];
        for (int i = 0; i < 8; ++i)
            w[i] = vreinterpretq_u64_u8(vqtbl1q_u8(vld1q_u8(data + 16*i), BSWAP));

        // Rounds 0-15 (pairs 0-7): loaded words, no schedule update needed
        for (int i = 0; i < 8; ++i) {
            uint64x2_t t = vaddq_u64(w[i], vld1q_u64(K + 2*i));
            t = vaddq_u64(vextq_u64(t, t, 1), gh);
            t = vsha512hq_u64(t, vextq_u64(ef, gh, 1), vextq_u64(cd, ef, 1));
            uint64x2_t t1 = vsha512h2q_u64(t, cd, ab);
            gh = ef; ef = vaddq_u64(cd, t); cd = ab; ab = t1;
        }

        const uint64_t* k_var = K + 16;
        for (int i = 0; i < 4; i++)
        {
            for (int i = 0; i < 8; ++i) {
                w[i & 7] = vsha512su1q_u64(vsha512su0q_u64(w[i & 7], w[(i + 1) & 7]), w[(i + 7) & 7], vextq_u64(w[(i + 4) & 7], w[(i + 5) & 7], 1));
                uint64x2_t t = vaddq_u64(w[i & 7], vld1q_u64(k_var + 2*i));
                t = vaddq_u64(vextq_u64(t, t, 1), gh);
                t = vsha512hq_u64(t, vextq_u64(ef, gh, 1), vextq_u64(cd, ef, 1));
                uint64x2_t t1 = vsha512h2q_u64(t, cd, ab);
                gh = ef; ef = vaddq_u64(cd, t); cd = ab; ab = t1;
            }
            k_var += 16;
        }

        ab = vaddq_u64(ab, ab0);
        cd = vaddq_u64(cd, cd0);
        ef = vaddq_u64(ef, ef0);
        gh = vaddq_u64(gh, gh0);

        if (--block_count == 0) break;
        data += 128;
    }

    vst1q_u64(&state[0], ab);
    vst1q_u64(&state[2], cd);
    vst1q_u64(&state[4], ef);
    vst1q_u64(&state[6], gh);
}

} // namespace dataforge::sha2_detail

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#elif defined(__clang__)
#pragma clang attribute pop
#endif

#endif // DATAFORGE_ACCEL_CAN_COMPILE_ARM_SHA512
