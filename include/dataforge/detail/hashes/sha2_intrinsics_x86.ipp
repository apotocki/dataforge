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

#include <cstdint>
#include <cstring>

// Per-function ISA selection. On GCC/Clang each accelerated routine carries its
// own target attribute, so the file compiles even when the global -m flags do
// not enable SHA / AVX-512; the run-time dispatcher only ever calls a routine on
// a CPU that actually supports it. On MSVC the intrinsics are always available.
// NB: CPU-detection helpers deliberately get NO target attribute — they must run
// on the baseline ISA.
#if defined(__GNUC__) || defined(__clang__)
#   define DATAFORGE_SHA_TARGET     __attribute__((target("sha,sse4.1")))
#   define DATAFORGE_AVX512_TARGET  __attribute__((target("avx512f,avx512vl,sse4.1")))
#   define DATAFORGE_FORCEINLINE    inline __attribute__((always_inline))
#else
#   define DATAFORGE_SHA_TARGET
#   define DATAFORGE_AVX512_TARGET
#   define DATAFORGE_FORCEINLINE    __forceinline
#endif

namespace dataforge::sha2_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
// CPUID leaf 7, EBX bit 29 -> Intel SHA Extensions (SHA-NI).
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

// AVX-512 Foundation (F) + Vector Length (VL). VL is required because the
// vectorized message schedules operate on 128-bit (xmm) EVEX operands. We also
// verify the OS has enabled the full AVX-512 register state via XCR0, otherwise
// the instructions would #UD even though the CPUID feature bits are set.
// Shared by the SHA-256 (opt-in) and SHA-512 (default) AVX-512 backends.
inline bool x86_runtime_has_avx512()
{
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    int regs[4] = { 0, 0, 0, 0 };
    __cpuid(regs, 0);
    if (regs[0] < 7)
        return false;
    __cpuidex(regs, 1, 0);
    const bool osxsave = (regs[2] & (1 << 27)) != 0;
    if (!osxsave)
        return false;
    // XCR0 bits: 1 SSE, 2 AVX, 5 opmask, 6 ZMM_Hi256, 7 Hi16_ZMM -> mask 0xE6.
    const unsigned long long xcr0 = _xgetbv(0);
    if ((xcr0 & 0xE6ull) != 0xE6ull)
        return false;
    __cpuidex(regs, 7, 0);
    const bool avx512f  = (regs[1] & (1 << 16)) != 0; // EBX bit 16
    const bool avx512vl = (regs[1] & (1u << 31)) != 0; // EBX bit 31
    return avx512f && avx512vl;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    // __builtin_cpu_supports folds in the required XGETBV/OS-enablement check.
    return __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl");
#else
    return false;
#endif
}
#endif // AUTODETECT_MODE

// --------------------------------------------------------------------------
// SHA-NI backend
// --------------------------------------------------------------------------
DATAFORGE_SHA_TARGET
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

// --------------------------------------------------------------------------
// AVX-512 backend (opt-in: DATAFORGE_ACCEL_X86_SHA256_USE_AVX512)
//
// For a single 64-byte block there are no AVX-512 SHA-256 round instructions
// (those are SHA-NI only), so the win comes from computing the 64-word message
// schedule with vector code while the compression rounds stay scalar — the
// classic AVX/AVX2 SHA-256 layout, here using the AVX-512VL vector-rotate
// (vprord) for sigma0/sigma1. SHA-NI is faster for a single block, so this is
// NOT used by plain auto-detect; it is selected only when the caller opts in
// via DATAFORGE_ACCEL_X86_SHA256_USE_AVX512. The round constants are reused from
// sha2_def_base<256>::K (no duplicated table) and folded into the schedule with
// aligned vector loads, which is why K is 64-byte aligned.
// --------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4752) // AVX-512 used without /arch:AVX512 (intentional: gated at run time)
#endif

DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha256_sigma0(__m128i x)
{
    // sigma0(x) = ror(x,7) ^ ror(x,18) ^ (x >> 3)
    return _mm_xor_si128(_mm_xor_si128(_mm_ror_epi32(x, 7), _mm_ror_epi32(x, 18)),
                         _mm_srli_epi32(x, 3));
}

DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha256_sigma1(__m128i x)
{
    // sigma1(x) = ror(x,17) ^ ror(x,19) ^ (x >> 10)
    return _mm_xor_si128(_mm_xor_si128(_mm_ror_epi32(x, 17), _mm_ror_epi32(x, 19)),
                         _mm_srli_epi32(x, 10));
}

// Software equivalent of _mm_sha256msg1_epu32:
//   a = {w0,w1,w2,w3}, b = {w4,...} -> {w0+s0(w1), w1+s0(w2), w2+s0(w3), w3+s0(w4)}
DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha256_msg1(__m128i a, __m128i b)
{
    return _mm_add_epi32(a, avx512_sha256_sigma0(_mm_alignr_epi8(b, a, 4)));
}

// Software equivalent of _mm_sha256msg2_epu32:
//   a = partial sums {x0,x1,x2,x3}, b = {w12,w13,w14,w15}
//   out0 = x0 + s1(w14); out1 = x1 + s1(w15); out2 = x2 + s1(out0); out3 = x3 + s1(out1)
// The last two outputs depend on the first two, so sigma1 is applied in two
// stages (low pair, then high pair).
DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha256_msg2(__m128i a, __m128i b)
{
    const __m128i s1_lo = avx512_sha256_sigma1(_mm_shuffle_epi32(b, _MM_SHUFFLE(3, 2, 3, 2))); // s1 of {w14,w15,w14,w15}
    const __m128i lo    = _mm_add_epi32(a, s1_lo);                                              // out0,out1 in lanes 0,1
    const __m128i s1_hi = avx512_sha256_sigma1(_mm_shuffle_epi32(lo, _MM_SHUFFLE(1, 0, 1, 0))); // s1 of {out0,out1,out0,out1}
    const __m128i hi    = _mm_add_epi32(a, s1_hi);                                              // out2,out3 in lanes 2,3
    return _mm_blend_epi16(lo, hi, 0xF0);                                                       // {out0,out1,out2,out3}
}

DATAFORGE_FORCEINLINE uint32_t avx512_sha256_rotr(uint32_t x, unsigned n)
{
    return (x >> n) | (x << (32 - n));
}

DATAFORGE_AVX512_TARGET
inline void process_block_sha256_x86_avx512(uint32_t(&state)[8], const void* msg)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x0C0D0E0F08090A0BULL, 0x0405060700010203ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const auto* Kvec = reinterpret_cast<const __m128i*>(sha2_def_base<256>::K);

    // Message schedule, four 32-bit words per vector (16 vectors == 64 words).
    __m128i w[16];
    w[0] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data +  0)), SHUF_MASK);
    w[1] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), SHUF_MASK);
    w[2] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32)), SHUF_MASK);
    w[3] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48)), SHUF_MASK);

    for (int k = 4; k < 16; ++k) {
        const __m128i part = _mm_add_epi32(avx512_sha256_msg1(w[k - 4], w[k - 3]),
                                           _mm_alignr_epi8(w[k - 1], w[k - 2], 4)); // + {W[t-7..t-4]}
        w[k] = avx512_sha256_msg2(part, w[k - 1]);
    }

    // Fold in the round constants once (aligned vector loads) and keep W+K for
    // the scalar compression loop.
    alignas(64) uint32_t wk[64];
    for (int k = 0; k < 16; ++k)
        _mm_store_si128(reinterpret_cast<__m128i*>(wk + 4 * k),
                        _mm_add_epi32(w[k], _mm_load_si128(Kvec + k)));

    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int t = 0; t < 64; ++t) {
        const uint32_t S1 = avx512_sha256_rotr(e, 6) ^ avx512_sha256_rotr(e, 11) ^ avx512_sha256_rotr(e, 25);
        const uint32_t ch = (e & f) ^ (~e & g);
        const uint32_t T1 = h + S1 + ch + wk[t];
        const uint32_t S0 = avx512_sha256_rotr(a, 2) ^ avx512_sha256_rotr(a, 13) ^ avx512_sha256_rotr(a, 22);
        const uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        const uint32_t T2 = S0 + maj;
        h = g; g = f; f = e; e = d + T1; d = c; c = b; b = a; a = T1 + T2;
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

}

#undef DATAFORGE_SHA_TARGET
#undef DATAFORGE_AVX512_TARGET
#undef DATAFORGE_FORCEINLINE

#endif
