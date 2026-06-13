/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_X86_AVX512

#include <immintrin.h>
#include <cstdint>

// Per-function ISA selection (see sha2_intrinsics_x86.ipp). There are no SHA-NI
// instructions for the 64-bit SHA-2 variants. Two hardware paths exist:
//
//  AVX-512VL: uses vprorq for single-instruction 64-bit lane rotation.
//  SSE4.1:    same XMM message-schedule structure but decomposes each vprorq
//             into a shift-or pair; selected on CPUs without AVX-512 (e.g.
//             Raptor Lake desktop with both P- and E-cores active).
//
// Both paths share the 8-way unrolled scalar compression rounds via the
// DATAFORGE_SHA512_ROUND macro, which eliminates the 7 register-move
// instructions that the naïve loop emits per round.
#if defined(__GNUC__) || defined(__clang__)
#   define DATAFORGE_AVX512_TARGET  __attribute__((target("avx512f,avx512vl,sse4.1")))
#   define DATAFORGE_SSE41_TARGET   __attribute__((target("sse4.1")))
#   define DATAFORGE_FORCEINLINE    inline __attribute__((always_inline))
#else
#   define DATAFORGE_AVX512_TARGET
#   define DATAFORGE_SSE41_TARGET
#   define DATAFORGE_FORCEINLINE    __forceinline
#endif

namespace dataforge::sha2_detail {

// ---------------------------------------------------------------------------
// Scalar 64-bit rotation — no target attribute needed, inlines into callers.
// ---------------------------------------------------------------------------
DATAFORGE_FORCEINLINE uint64_t sha512_rotr(uint64_t x, unsigned n)
{
    return (x >> n) | (x << (64 - n));
}

// ---------------------------------------------------------------------------
// AVX-512VL sigma helpers (vprorq = 1 instruction per rotate)
// ---------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4752) // AVX-512 used without /arch:AVX512 (intentional: gated at run time)
#endif

DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha512_sigma0(__m128i x)
{
    // sigma0(x) = ror(x,1) ^ ror(x,8) ^ (x >> 7)
    return _mm_xor_si128(_mm_xor_si128(_mm_ror_epi64(x, 1), _mm_ror_epi64(x, 8)),
                         _mm_srli_epi64(x, 7));
}

DATAFORGE_AVX512_TARGET DATAFORGE_FORCEINLINE
__m128i avx512_sha512_sigma1(__m128i x)
{
    // sigma1(x) = ror(x,19) ^ ror(x,61) ^ (x >> 6)
    return _mm_xor_si128(_mm_xor_si128(_mm_ror_epi64(x, 19), _mm_ror_epi64(x, 61)),
                         _mm_srli_epi64(x, 6));
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// SSE4.1 sigma helpers (rotate = shift-right | shift-left, 3 insns each)
// ---------------------------------------------------------------------------
DATAFORGE_SSE41_TARGET DATAFORGE_FORCEINLINE
__m128i sse41_sha512_sigma0(__m128i x)
{
    // sigma0(x) = ror(x,1) ^ ror(x,8) ^ (x >> 7)
    return _mm_xor_si128(
        _mm_xor_si128(
            _mm_or_si128(_mm_srli_epi64(x,  1), _mm_slli_epi64(x, 63)),
            _mm_or_si128(_mm_srli_epi64(x,  8), _mm_slli_epi64(x, 56))
        ),
        _mm_srli_epi64(x, 7));
}

DATAFORGE_SSE41_TARGET DATAFORGE_FORCEINLINE
__m128i sse41_sha512_sigma1(__m128i x)
{
    // sigma1(x) = ror(x,19) ^ ror(x,61) ^ (x >> 6)
    return _mm_xor_si128(
        _mm_xor_si128(
            _mm_or_si128(_mm_srli_epi64(x, 19), _mm_slli_epi64(x, 45)),
            _mm_or_si128(_mm_srli_epi64(x, 61), _mm_slli_epi64(x,  3))
        ),
        _mm_srli_epi64(x, 6));
}

// ---------------------------------------------------------------------------
// 8-way unrolled compression-round macro.
//
// Variables a..h are passed by name so each call site permutes the argument
// list, rotating the logical state vector at compile time.  No register moves
// are emitted between rounds — the compiler assigns each name to a fixed
// physical register for the whole 80-round sequence.
//
// wk[] is the pre-computed W[t]+K[t] table; t is the round index (0..79).
// ---------------------------------------------------------------------------
#define DATAFORGE_SHA512_ROUND(a,b,c,d,e,f,g,h,wk,t)                     \
do {                                                                       \
    uint64_t _S1 = sha512_rotr(e,14) ^ sha512_rotr(e,18) ^ sha512_rotr(e,41); \
    uint64_t _T1 = (h) + _S1 + ((e & f) ^ (~(e) & g)) + (wk)[t];        \
    uint64_t _S0 = sha512_rotr(a,28) ^ sha512_rotr(a,34) ^ sha512_rotr(a,39); \
    (d) += _T1;                                                            \
    (h) = _T1 + _S0 + ((a & b) ^ (a & c) ^ (b & c));                     \
} while(0)

// ---------------------------------------------------------------------------
// AVX-512VL backend: SIMD message schedule + 8-unrolled scalar compression
// ---------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4752)
#endif

DATAFORGE_AVX512_TARGET
inline void process_block_sha512_x86_avx512(uint64_t(&state)[8], const void* msg)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x08090A0B0C0D0E0FULL, 0x0001020304050607ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const auto* Kvec = reinterpret_cast<const __m128i*>(sha2_def_base<512>::K);

    // Message schedule: two 64-bit words per XMM vector (40 vectors = 80 words).
    // W[t]   = sigma1(W[t-2]) + W[t-7]  + sigma0(W[t-15]) + W[t-16]
    // W[t+1] = sigma1(W[t-1]) + W[t-6]  + sigma0(W[t-14]) + W[t-15]
    // The sigma1 look-back is exactly the previous vector, so there is no
    // intra-vector dependency to break up.
    __m128i w[40];
    for (int i = 0; i < 8; ++i)
        w[i] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16 * i)), SHUF_MASK);

    for (int j = 8; j < 40; ++j) {
        const __m128i s0in = _mm_alignr_epi8(w[j - 7], w[j - 8], 8); // {W[t-15], W[t-14]}
        const __m128i wm7  = _mm_alignr_epi8(w[j - 3], w[j - 4], 8); // {W[t-7],  W[t-6]}
        __m128i v = _mm_add_epi64(w[j - 8], avx512_sha512_sigma0(s0in));
        v = _mm_add_epi64(v, wm7);
        v = _mm_add_epi64(v, avx512_sha512_sigma1(w[j - 1]));
        w[j] = v;
    }

    // Fold round constants in one pass (aligned loads from the 64-byte-aligned K
    // table) so the compression loop only needs a single load per round.
    alignas(64) uint64_t wk[80];
    for (int j = 0; j < 40; ++j)
        _mm_store_si128(reinterpret_cast<__m128i*>(wk + 2 * j),
                        _mm_add_epi64(w[j], _mm_load_si128(Kvec + j)));

    uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

    // 8-way unroll: argument permutation rotates the logical state vector,
    // eliminating all inter-round register moves.
    for (int t = 0; t < 80; t += 8) {
        DATAFORGE_SHA512_ROUND(a,b,c,d,e,f,g,h, wk, t+0);
        DATAFORGE_SHA512_ROUND(h,a,b,c,d,e,f,g, wk, t+1);
        DATAFORGE_SHA512_ROUND(g,h,a,b,c,d,e,f, wk, t+2);
        DATAFORGE_SHA512_ROUND(f,g,h,a,b,c,d,e, wk, t+3);
        DATAFORGE_SHA512_ROUND(e,f,g,h,a,b,c,d, wk, t+4);
        DATAFORGE_SHA512_ROUND(d,e,f,g,h,a,b,c, wk, t+5);
        DATAFORGE_SHA512_ROUND(c,d,e,f,g,h,a,b, wk, t+6);
        DATAFORGE_SHA512_ROUND(b,c,d,e,f,g,h,a, wk, t+7);
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// SSE4.1 backend: identical structure, but sigma0/sigma1 use shift-or pairs
// instead of vprorq, so it runs on any x86-64 CPU (Raptor Lake desktop,
// Alder Lake, Skylake, etc.) without requiring AVX-512.
// ---------------------------------------------------------------------------
DATAFORGE_SSE41_TARGET
inline void process_block_sha512_x86_sse41(uint64_t(&state)[8], const void* msg)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x08090A0B0C0D0E0FULL, 0x0001020304050607ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const auto* Kvec = reinterpret_cast<const __m128i*>(sha2_def_base<512>::K);

    __m128i w[40];
    for (int i = 0; i < 8; ++i)
        w[i] = _mm_shuffle_epi8(_mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16 * i)), SHUF_MASK);

    for (int j = 8; j < 40; ++j) {
        const __m128i s0in = _mm_alignr_epi8(w[j - 7], w[j - 8], 8);
        const __m128i wm7  = _mm_alignr_epi8(w[j - 3], w[j - 4], 8);
        __m128i v = _mm_add_epi64(w[j - 8], sse41_sha512_sigma0(s0in));
        v = _mm_add_epi64(v, wm7);
        v = _mm_add_epi64(v, sse41_sha512_sigma1(w[j - 1]));
        w[j] = v;
    }

    alignas(64) uint64_t wk[80];
    for (int j = 0; j < 40; ++j)
        _mm_store_si128(reinterpret_cast<__m128i*>(wk + 2 * j),
                        _mm_add_epi64(w[j], _mm_load_si128(Kvec + j)));

    uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

    for (int t = 0; t < 80; t += 8) {
        DATAFORGE_SHA512_ROUND(a,b,c,d,e,f,g,h, wk, t+0);
        DATAFORGE_SHA512_ROUND(h,a,b,c,d,e,f,g, wk, t+1);
        DATAFORGE_SHA512_ROUND(g,h,a,b,c,d,e,f, wk, t+2);
        DATAFORGE_SHA512_ROUND(f,g,h,a,b,c,d,e, wk, t+3);
        DATAFORGE_SHA512_ROUND(e,f,g,h,a,b,c,d, wk, t+4);
        DATAFORGE_SHA512_ROUND(d,e,f,g,h,a,b,c, wk, t+5);
        DATAFORGE_SHA512_ROUND(c,d,e,f,g,h,a,b, wk, t+6);
        DATAFORGE_SHA512_ROUND(b,c,d,e,f,g,h,a, wk, t+7);
    }

    state[0] += a; state[1] += b; state[2] += c; state[3] += d;
    state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

#undef DATAFORGE_SHA512_ROUND

}

#undef DATAFORGE_AVX512_TARGET
#undef DATAFORGE_SSE41_TARGET
#undef DATAFORGE_FORCEINLINE

#endif
