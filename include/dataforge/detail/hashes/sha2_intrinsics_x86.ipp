/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC target("sha,sse4.1")
#elif defined(__clang__)
#pragma clang attribute push(__attribute__((target("sha,sse4.1"))), apply_to=function)
#endif

#include <immintrin.h>

#if DATAFORGE_ACCEL_AUTODETECT
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif
#endif

#include <cstring>

namespace dataforge::sha2_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
inline bool sha256_runtime_has_sha256_accel()
{
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    int regs[4] = { 0, 0, 0, 0 };
    __cpuidex(regs, 7, 0);
    return (regs[1] & (1 << 29)) != 0;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    unsigned int eax = 0;
    unsigned int ebx = 0;
    unsigned int ecx = 0;
    unsigned int edx = 0;
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) == 0)
        return false;
    return (ebx & (1u << 29)) != 0;
#else
    return true;
#endif
}
#endif

inline void process_block_sha256_x86(uint32_t(&state)[8], const void* msg)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x0C0D0E0F08090A0BULL, 0x0405060700010203ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    // state[] is {A,B,C,D,E,F,G,H};

    __m128i tmp0 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(state)), 0xB1);                /* DCBA -> CDAB */
    __m128i tmp1 = _mm_shuffle_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(state + 4)), 0x1B);             /* HGFE -> EFGH */
    __m128i state0 = _mm_alignr_epi8(tmp0, tmp1, 8);             /* ABEF */
    __m128i state1 = _mm_blend_epi16(tmp1, tmp0, 0xF0);          /* CDGH */

    __m128i abef = state0;
    __m128i cdgh = state1;

    __m128i msg0 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 0)), SHUF_MASK);
    __m128i msg1 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), SHUF_MASK);
    __m128i msg2 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32)), SHUF_MASK);
    __m128i msg3 = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48)), SHUF_MASK);

    const auto* K = reinterpret_cast<const __m128i*>(sha2_def_base<256>::K);

    /* Rounds 0-3 */
    tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
    msg0 = _mm_sha256msg1_epu32(msg0, msg1);

    /* Rounds 4-7 */
    tmp0 = _mm_add_epi32(msg1, _mm_load_si128(K + 1));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
    msg1 = _mm_sha256msg1_epu32(msg1, msg2);

    /* Rounds 8..55: 3 iterations, each processing 16 rounds (4 groups of 4) */
    for (int i = 2; i <= 10; i += 4) {
        /* Rounds +0..+3 */
        tmp0 = _mm_add_epi32(msg2, _mm_load_si128(K + i));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        msg0 = _mm_sha256msg2_epu32(_mm_add_epi32(msg0, _mm_alignr_epi8(msg3, msg2, 4)), msg3);
        msg2 = _mm_sha256msg1_epu32(msg2, msg3);

        /* Rounds +4..+7 */
        tmp0 = _mm_add_epi32(msg3, _mm_load_si128(K + i + 1));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        msg1 = _mm_sha256msg2_epu32(_mm_add_epi32(msg1, _mm_alignr_epi8(msg0, msg3, 4)), msg0);
        msg3 = _mm_sha256msg1_epu32(msg3, msg0);

        /* Rounds +8..+11 */
        tmp0 = _mm_add_epi32(msg0, _mm_load_si128(K + i + 2));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        msg2 = _mm_sha256msg2_epu32(_mm_add_epi32(msg2, _mm_alignr_epi8(msg1, msg0, 4)), msg1);

        /* Rounds +12..+15 */
        tmp0 = _mm_add_epi32(msg1, _mm_load_si128(K + i + 3));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        msg3 = _mm_sha256msg2_epu32(_mm_add_epi32(msg3, _mm_alignr_epi8(msg2, msg1, 4)), msg2);

        if (i == 10) break; // after round 55: msg0/msg1 not needed for next schedule
        msg0 = _mm_sha256msg1_epu32(msg0, msg1);
        msg1 = _mm_sha256msg1_epu32(msg1, msg2);
    }

    /* Rounds 56-59 */
    tmp0 = _mm_add_epi32(msg2, _mm_load_si128(K + 14));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));

    /* Rounds 60-63 */
    tmp0 = _mm_add_epi32(msg3, _mm_load_si128(K + 15));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));

    state0 = _mm_add_epi32(state0, abef);
    state1 = _mm_add_epi32(state1, cdgh);

    /* Repack ABEF / CDGH back to {A..D} / {E..H} */
    tmp0 = _mm_shuffle_epi32(state0, 0x1B);    /* FEBA */
    tmp1 = _mm_shuffle_epi32(state1, 0xB1);    /* DCHG */
    state0 = _mm_blend_epi16(tmp0, tmp1, 0xF0); /* DCBA */
    state1 = _mm_alignr_epi8(tmp1, tmp0, 8);    /* HGFE */

    _mm_storeu_si128((__m128i*) & state[0], state0);
    _mm_storeu_si128((__m128i*) & state[4], state1);
}

}

#if defined(__clang__)
#pragma clang attribute pop
#endif

#endif
