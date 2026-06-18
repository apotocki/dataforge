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
// Both paths share the same merged schedule+compression loop structure:
//   - w[16] circular buffer (was w[40]) — 256 bytes vs 640 bytes
//   - no wk[80] intermediate array    — 0 bytes   vs 640 bytes
//   - 4 schedule vectors computed then immediately consumed as 8 rounds
// This eliminates the two-pass store→load round-trip that cost ~30% vs OpenSSL.

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
// Compression-round macro.
//
// Variables a..h are passed by name so each call-site permutes the argument
// list, rotating the logical state vector at compile time.  No register moves
// are emitted between rounds — the compiler assigns each name to a fixed
// physical register for the whole 80-round sequence.
//
// wkval is the precomputed W[t]+K[t] scalar for this round.
// ---------------------------------------------------------------------------
#define DATAFORGE_SHA512_ROUND(a,b,c,d,e,f,g,h,wkval)                    \
do {                                                                       \
    uint64_t _S1 = sha512_rotr(e,14) ^ sha512_rotr(e,18) ^ sha512_rotr(e,41); \
    uint64_t _T1 = (h) + _S1 + ((e & f) ^ (~(e) & g)) + (uint64_t)(wkval); \
    uint64_t _S0 = sha512_rotr(a,28) ^ sha512_rotr(a,34) ^ sha512_rotr(a,39); \
    (d) += _T1;                                                            \
    (h) = _T1 + _S0 + ((a & b) ^ (a & c) ^ (b & c));                     \
} while(0)

// Two consecutive rounds from the low and high 64-bit lanes of one XMM vector
// holding {W[t]+K[t], W[t+1]+K[t+1]}.  The 16 argument names encode the
// compile-time state-vector rotation for rounds t and t+1 respectively.
// _mm_cvtsi128_si64 extracts the low lane (SSE2); _mm_extract_epi64 the high
// lane (SSE4.1) — both are no-op moves on all modern micro-architectures.
#define DATAFORGE_SHA512_ROUNDS2(a0,b0,c0,d0,e0,f0,g0,h0,                \
                                  a1,b1,c1,d1,e1,f1,g1,h1, vec)          \
do {                                                                       \
    DATAFORGE_SHA512_ROUND(a0,b0,c0,d0,e0,f0,g0,h0,                      \
        (uint64_t)_mm_cvtsi128_si64(vec));                                \
    DATAFORGE_SHA512_ROUND(a1,b1,c1,d1,e1,f1,g1,h1,                      \
        (uint64_t)_mm_extract_epi64(vec, 1));                             \
} while(0)

// Eight rounds from four consecutive W+K vectors v0..v3, starting with the
// state rotation appropriate when round index t is a multiple of 8.
// Each ROUNDS2 call advances t by 2; four calls return the rotation to
// (a,b,c,d,e,f,g,h), so the same sequence repeats every 8 rounds.
#define DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3)                          \
do {                                                                       \
    DATAFORGE_SHA512_ROUNDS2(a,b,c,d,e,f,g,h, h,a,b,c,d,e,f,g, v0);     \
    DATAFORGE_SHA512_ROUNDS2(g,h,a,b,c,d,e,f, f,g,h,a,b,c,d,e, v1);     \
    DATAFORGE_SHA512_ROUNDS2(e,f,g,h,a,b,c,d, d,e,f,g,h,a,b,c, v2);     \
    DATAFORGE_SHA512_ROUNDS2(c,d,e,f,g,h,a,b, b,c,d,e,f,g,h,a, v3);     \
} while(0)

// ---------------------------------------------------------------------------
// AVX-512VL backend: merged schedule+compression, circular w[16] buffer.
//
// w[j] holds {W[2j], W[2j+1]}.  For j >= 8 (rounds 16-79) the recurrence is:
//   w[j] = sigma1(w[j-1]) + alignr(w[j-3],w[j-4],8) + sigma0(alignr(w[j-7],w[j-8],8)) + w[j-8]
// We compute 4 vectors per outer-loop iteration and immediately fold K and
// run 8 compression rounds, so no intermediate wk[] buffer is needed.
// ---------------------------------------------------------------------------
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4752)
#endif

DATAFORGE_AVX512_TARGET
inline void process_blocks_sha512_x86_avx512(uint64_t(&state)[8], const void* msg, size_t block_count)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x08090A0B0C0D0E0FULL, 0x0001020304050607ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const auto* K    = reinterpret_cast<const __m128i*>(sha2_def_base<512>::K);

    // Circular buffer: 16 XMM vectors = 32 words.  A window of [j-8, j-1]
    // is always available; slot j&15 is safe to overwrite once j >= 16
    // (its last read was at j-8, which is already past).
    __m128i w[16];

    for (;;) {
        // Load and byte-swap 16 words (8 XMM vectors = one SHA-512 block).
        for (int i = 0; i < 8; ++i)
            w[i] = _mm_shuffle_epi8(
                _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16 * i)),
                SHUF_MASK);

        uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

        // Rounds 0-7 (j=0..3): fold K into loaded data, then 8 rounds.
        {
            __m128i v0 = _mm_add_epi64(w[0], K[0]);
            __m128i v1 = _mm_add_epi64(w[1], K[1]);
            __m128i v2 = _mm_add_epi64(w[2], K[2]);
            __m128i v3 = _mm_add_epi64(w[3], K[3]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }
        // Rounds 8-15 (j=4..7): same, second half of the initial 16 words.
        {
            __m128i v0 = _mm_add_epi64(w[4], K[4]);
            __m128i v1 = _mm_add_epi64(w[5], K[5]);
            __m128i v2 = _mm_add_epi64(w[6], K[6]);
            __m128i v3 = _mm_add_epi64(w[7], K[7]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }

        // Rounds 16-79 (j=8..39, step 4): expand 4 vectors, fold K, run 8 rounds.
        // The outer step is always a multiple of 4, so the round index entering
        // each 8ROUNDS block is always a multiple of 8 — no rotation offset needed.
        for (int j = 8; j < 40; j += 4) {
            // j+0
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-7)&15], w[(j-8)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-3)&15], w[(j-4)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-8)&15], avx512_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, avx512_sha512_sigma1(w[(j-1)&15]));
                w[j & 15] = v;
            }
            // j+1
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-6)&15], w[(j-7)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-2)&15], w[(j-3)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-7)&15], avx512_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, avx512_sha512_sigma1(w[j & 15]));
                w[(j+1) & 15] = v;
            }
            // j+2
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-5)&15], w[(j-6)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-1)&15], w[(j-2)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-6)&15], avx512_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, avx512_sha512_sigma1(w[(j+1) & 15]));
                w[(j+2) & 15] = v;
            }
            // j+3
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-4)&15], w[(j-5)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[j & 15],   w[(j-1)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-5)&15], avx512_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, avx512_sha512_sigma1(w[(j+2) & 15]));
                w[(j+3) & 15] = v;
            }

            __m128i v0 = _mm_add_epi64(w[j       & 15], K[j  ]);
            __m128i v1 = _mm_add_epi64(w[(j+1)   & 15], K[j+1]);
            __m128i v2 = _mm_add_epi64(w[(j+2)   & 15], K[j+2]);
            __m128i v3 = _mm_add_epi64(w[(j+3)   & 15], K[j+3]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;

        if (--block_count == 0) break;
        data += 128;
    }
}

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// SSE4.1 backend: identical structure, sigma0/sigma1 use shift-or pairs
// instead of vprorq — runs on any x86-64 CPU since ~2008.
// ---------------------------------------------------------------------------
DATAFORGE_SSE41_TARGET
inline void process_blocks_sha512_x86_sse41(uint64_t(&state)[8], const void* msg, size_t block_count)
{
    static const __m128i SHUF_MASK = _mm_set_epi64x(0x08090A0B0C0D0E0FULL, 0x0001020304050607ULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const auto* K    = reinterpret_cast<const __m128i*>(sha2_def_base<512>::K);

    __m128i w[16];

    for (;;) {
        for (int i = 0; i < 8; ++i)
            w[i] = _mm_shuffle_epi8(
                _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16 * i)),
                SHUF_MASK);

        uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

        {
            __m128i v0 = _mm_add_epi64(w[0], K[0]);
            __m128i v1 = _mm_add_epi64(w[1], K[1]);
            __m128i v2 = _mm_add_epi64(w[2], K[2]);
            __m128i v3 = _mm_add_epi64(w[3], K[3]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }
        {
            __m128i v0 = _mm_add_epi64(w[4], K[4]);
            __m128i v1 = _mm_add_epi64(w[5], K[5]);
            __m128i v2 = _mm_add_epi64(w[6], K[6]);
            __m128i v3 = _mm_add_epi64(w[7], K[7]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }

        for (int j = 8; j < 40; j += 4) {
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-7)&15], w[(j-8)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-3)&15], w[(j-4)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-8)&15], sse41_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, sse41_sha512_sigma1(w[(j-1)&15]));
                w[j & 15] = v;
            }
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-6)&15], w[(j-7)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-2)&15], w[(j-3)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-7)&15], sse41_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, sse41_sha512_sigma1(w[j & 15]));
                w[(j+1) & 15] = v;
            }
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-5)&15], w[(j-6)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[(j-1)&15], w[(j-2)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-6)&15], sse41_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, sse41_sha512_sigma1(w[(j+1) & 15]));
                w[(j+2) & 15] = v;
            }
            {
                const __m128i s0 = _mm_alignr_epi8(w[(j-4)&15], w[(j-5)&15], 8);
                const __m128i s7 = _mm_alignr_epi8(w[j & 15],   w[(j-1)&15], 8);
                __m128i v = _mm_add_epi64(w[(j-5)&15], sse41_sha512_sigma0(s0));
                v = _mm_add_epi64(v, s7);
                v = _mm_add_epi64(v, sse41_sha512_sigma1(w[(j+2) & 15]));
                w[(j+3) & 15] = v;
            }

            __m128i v0 = _mm_add_epi64(w[j       & 15], K[j  ]);
            __m128i v1 = _mm_add_epi64(w[(j+1)   & 15], K[j+1]);
            __m128i v2 = _mm_add_epi64(w[(j+2)   & 15], K[j+2]);
            __m128i v3 = _mm_add_epi64(w[(j+3)   & 15], K[j+3]);
            DATAFORGE_SHA512_8ROUNDS(v0, v1, v2, v3);
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;

        if (--block_count == 0) break;
        data += 128;
    }
}

#undef DATAFORGE_SHA512_8ROUNDS
#undef DATAFORGE_SHA512_ROUNDS2
#undef DATAFORGE_SHA512_ROUND

}

#endif
