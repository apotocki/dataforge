/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_ARM_NEON_SHA512

#include <arm_neon.h>

namespace dataforge::sha2_detail {

// ---------------------------------------------------------------------------
// AArch64 NEON SHA-512 backend — rolling window, 8-round groups.
//
// Processes 8 compression rounds per group using a tiny wk[8] buffer (64B),
// so total stack is w[8]=128B + wk[8]=64B = 192B vs the old w[40]+wk[80]=1280B.
// Schedule update and wk computation are done together in a 4-iter inner loop
// that the compiler fully unrolls; the 8-way compression unroll eliminates
// inter-round register moves. After every 8 rounds the state rotation is 0
// (mod 8), so the argument permutation in the macro resets automatically.
//
// sigma0(x) = ror(x,1)  ^ ror(x,8)  ^ (x>>7)
// sigma1(x) = ror(x,19) ^ ror(x,61) ^ (x>>6)
//   implemented via vsriq_n_u64(vshlq_n_u64(x, 64-n), x, n)  [= ror(x,n)]
// ---------------------------------------------------------------------------

DATAFORGE_FORCEINLINE uint64_t neon_sha512_rotr(uint64_t x, unsigned n)
{
    return (x >> n) | (x << (64 - n));
}

DATAFORGE_FORCEINLINE uint64x2_t neon_sha512_sigma0(uint64x2_t x)
{
    // ror(x,1) ^ ror(x,8) ^ (x>>7)
    return veorq_u64(veorq_u64(
        vsriq_n_u64(vshlq_n_u64(x, 63), x,  1),
        vsriq_n_u64(vshlq_n_u64(x, 56), x,  8)),
        vshrq_n_u64(x, 7));
}

DATAFORGE_FORCEINLINE uint64x2_t neon_sha512_sigma1(uint64x2_t x)
{
    // ror(x,19) ^ ror(x,61) ^ (x>>6)
    return veorq_u64(veorq_u64(
        vsriq_n_u64(vshlq_n_u64(x, 45), x, 19),
        vsriq_n_u64(vshlq_n_u64(x,  3), x, 61)),
        vshrq_n_u64(x, 6));
}

// 8-way unrolled compression round.  Argument permutation rotates the logical
// state vector at compile time — no register moves between rounds.
#define DATAFORGE_SHA512_ROUND_NEON(a,b,c,d,e,f,g,h,wk,t)                        \
do {                                                                               \
    uint64_t _S1 = neon_sha512_rotr(e,14) ^ neon_sha512_rotr(e,18) ^ neon_sha512_rotr(e,41); \
    uint64_t _T1 = (h) + _S1 + ((e & f) ^ (~(e) & g)) + (wk)[t];                \
    uint64_t _S0 = neon_sha512_rotr(a,28) ^ neon_sha512_rotr(a,34) ^ neon_sha512_rotr(a,39); \
    (d) += _T1;                                                                    \
    (h) = _T1 + _S0 + ((a & b) ^ (a & c) ^ (b & c));                             \
} while(0)

inline void process_blocks_sha512_arm_neon(uint64_t(&state)[8], const void* msg, size_t block_count)
{
    const uint8x16_t BSWAP = { 7,6,5,4,3,2,1,0, 15,14,13,12,11,10,9,8 };
    const uint64_t* K = sha2_def_base<512>::K;
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    for (;;) {
        // Load and byte-swap 128-byte block: 8 vectors, 2 words each
        uint64x2_t w[8];
        for (int i = 0; i < 8; ++i)
            w[i] = vreinterpretq_u64_u8(vqtbl1q_u8(vld1q_u8(data + 16*i), BSWAP));

        uint64_t a = state[0], b = state[1], c = state[2], d = state[3];
        uint64_t e = state[4], f = state[5], g = state[6], h = state[7];

        // 10 groups of 8 rounds each (80 rounds total).
        // Each group fills wk[8] from 4 schedule slots, then runs 8-way unrolled
        // compression. After 8 rounds the state rotation is 0 mod 8, so the
        // permutation resets and the same unroll pattern repeats every group.
        for (int i = 0; i < 40; i += 4) {
            alignas(16) uint64_t wk[8];

            for (int j = 0; j < 4; ++j) {
                const int iter = i + j;
                const int s    = iter & 7;
                if (iter >= 8) {
                    // W[t] = sigma1(W[t-2]) + W[t-7] + sigma0(W[t-15]) + W[t-16]
                    // Packed two words at a time via the rolling window:
                    //   w[s]        = {W[t-16], W[t-15]}
                    //   w[(s+1)&7]  = {W[t-14], W[t-13]}  → sigma0 input: vextq gives {W[t-15],W[t-14]}
                    //   w[(s+4)&7]  = {W[t-8],  W[t-7]}   → wm7 input:    vextq gives {W[t-7], W[t-6]}
                    //   w[(s+7)&7]  = {W[t-2],  W[t-1]}   → sigma1 input
                    const uint64x2_t s0in = vextq_u64(w[s],       w[(s+1)&7], 1);
                    const uint64x2_t wm7  = vextq_u64(w[(s+4)&7], w[(s+5)&7], 1);
                    w[s] = vaddq_u64(vaddq_u64(
                               vaddq_u64(w[s], neon_sha512_sigma0(s0in)),
                               wm7),
                               neon_sha512_sigma1(w[(s+7)&7]));
                }
                vst1q_u64(wk + 2*j, vaddq_u64(w[s], vld1q_u64(K + 2*iter)));
            }

            DATAFORGE_SHA512_ROUND_NEON(a,b,c,d,e,f,g,h, wk, 0);
            DATAFORGE_SHA512_ROUND_NEON(h,a,b,c,d,e,f,g, wk, 1);
            DATAFORGE_SHA512_ROUND_NEON(g,h,a,b,c,d,e,f, wk, 2);
            DATAFORGE_SHA512_ROUND_NEON(f,g,h,a,b,c,d,e, wk, 3);
            DATAFORGE_SHA512_ROUND_NEON(e,f,g,h,a,b,c,d, wk, 4);
            DATAFORGE_SHA512_ROUND_NEON(d,e,f,g,h,a,b,c, wk, 5);
            DATAFORGE_SHA512_ROUND_NEON(c,d,e,f,g,h,a,b, wk, 6);
            DATAFORGE_SHA512_ROUND_NEON(b,c,d,e,f,g,h,a, wk, 7);
        }

        state[0] += a; state[1] += b; state[2] += c; state[3] += d;
        state[4] += e; state[5] += f; state[6] += g; state[7] += h;

        if (--block_count == 0) break;
        data += 128;
    }
}

#undef DATAFORGE_SHA512_ROUND_NEON

} // namespace dataforge::sha2_detail

#endif // DATAFORGE_ACCEL_CAN_COMPILE_ARM_NEON_SHA512
