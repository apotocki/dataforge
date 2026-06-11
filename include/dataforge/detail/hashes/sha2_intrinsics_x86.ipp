/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA

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
    const __m128i SHUF_MASK = _mm_set_epi64x(0x0C0D0E0F08090A0BULL, 0x0405060700010203ULL);

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

    /* Rounds 0-3 */
    tmp0 = _mm_add_epi32(msg0, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K)));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
    msg0 = _mm_sha256msg1_epu32(msg0, msg1);

    /* Rounds 4-7 */
    tmp0 = _mm_add_epi32(msg1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + 4)));
    state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
    state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
    msg1 = _mm_sha256msg1_epu32(msg1, msg2);

    int i = 8;
    for (;;) {
        /* Rounds 8-11 */ /* Rounds 24-27 */ /* Rounds 40-43 */

        tmp0 = _mm_add_epi32(msg2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + i)));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        
        msg0 = _mm_sha256msg2_epu32(_mm_add_epi32(msg0, _mm_alignr_epi8(msg3, msg2, 4)), msg3);
        msg2 = _mm_sha256msg1_epu32(msg2, msg3);

        /* Rounds 12-15 */ /* Rounds 28-31 */ /* Rounds 44-47 */
        tmp0 = _mm_add_epi32(msg3, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + i + 4)));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));

        msg1 = _mm_sha256msg2_epu32(_mm_add_epi32(msg1, _mm_alignr_epi8(msg0, msg3, 4)), msg0);
        msg3 = _mm_sha256msg1_epu32(msg3, msg0);

        /* Rounds 16-19 */ /* Rounds 32-35 */ /* Rounds 48-51 */
        tmp0 = _mm_add_epi32(msg0, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + i + 8)));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));

        msg2 = _mm_sha256msg2_epu32(_mm_add_epi32(msg2, _mm_alignr_epi8(msg1, msg0, 4)), msg1);

        /* Rounds 20-23 */ /* Rounds 36-39 */ /* Rounds 52-55 */
        tmp0 = _mm_add_epi32(msg1, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + i + 12)));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));

        msg3 = _mm_sha256msg2_epu32(_mm_add_epi32(msg3, _mm_alignr_epi8(msg2, msg1, 4)), msg2);

        i += 16;
        if (i == 56) break; // last iteration doesn't need to compute msg0 and msg1 for next round
        msg0 = _mm_sha256msg1_epu32(msg0, msg1);
        msg1 = _mm_sha256msg1_epu32(msg1, msg2);
    }

    for (;; i += 4) {
        /* Rounds 56-59 */ /* Rounds 60-63 */
        tmp0 = _mm_add_epi32(msg2, _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + i)));
        state1 = _mm_sha256rnds2_epu32(state1, state0, tmp0);
        state0 = _mm_sha256rnds2_epu32(state0, state1, _mm_shuffle_epi32(tmp0, 0x0E));
        if (i == 60) break; // last iteration doesn't need to compute msg2 and msg3 for next round)
        msg2 = msg3;
    }

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

#endif
