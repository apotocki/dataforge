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
// Message schedule: eight uint64x2_t w[0..7] cover two words each (16 total
// loaded per block). Each slot w[k] stores {W[2k], W[2k+1]} in {D[0], D[1]}.
// Slots are reused in-place with a rolling window: for pair i>=8, w[i%8] is
// updated to hold {W[2i], W[2i+1]} before use.
//
// vsha512su0q_u64(w[s], w[(s+1)&7]):
//   result[0] = w[s][0] + sigma0(w[s][1])
//   result[1] = w[s][1] + sigma0(w[(s+1)&7][0])
//
// vsha512su1q_u64(su0, Vn, Vm) — note the CROSS-LANE sigma1:
//   result[0] = su0[0] + sigma1(Vn[1]) + Vm[0]
//   result[1] = su0[1] + sigma1(Vn[0]) + Vm[1]
// Therefore the "W[t-2], W[t-1]" pair must be passed as {W[t-1], W[t-2]}
// (lanes swapped) so sigma1 reaches the correct word for each result lane.
// vextq_u64(w[(s+7)&7], w[(s+7)&7], 1) swaps D[0]/D[1] of that slot.
//
// Two compression rounds per vsha512hq / vsha512h2q call:
//   t1 = vsha512hq_u64(ef, gh, wk)
//   t0 = vsha512h2q_u64(t1, cd, ab)
//   gh=ef; ef=cd+t1; cd=ab; ab=t0
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
