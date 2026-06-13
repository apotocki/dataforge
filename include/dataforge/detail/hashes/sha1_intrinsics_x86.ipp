/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1

#include <immintrin.h>

#if DATAFORGE_ACCEL_AUTODETECT
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif
#endif

#if defined(__GNUC__) || defined(__clang__)
#   define DATAFORGE_SHA1_TARGET     __attribute__((target("sha,sse4.1")))
#else
#   define DATAFORGE_SHA1_TARGET
#endif

namespace dataforge::sha1_detail {

#if DATAFORGE_ACCEL_IMPL == DATAFORGE_ACCEL_AUTODETECT_MODE
// CPUID leaf 7, EBX bit 29 -> Intel SHA Extensions (covers SHA-1 and SHA-256).
inline bool sha1_runtime_has_sha1_accel()
{
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
    int regs[4] = { 0, 0, 0, 0 };
    __cpuidex(regs, 7, 0);
    return (regs[1] & (1 << 29)) != 0;
#elif (defined(__GNUC__) || defined(__clang__)) && (defined(__x86_64__) || defined(__i386__))
    unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx) == 0)
        return false;
    return (ebx & (1u << 29)) != 0;
#else
    return true;
#endif
}
#endif // DATAFORGE_ACCEL_AUTODETECT_MODE

// --------------------------------------------------------------------------
// SHA-NI backend
// --------------------------------------------------------------------------
//
// Register conventions (SHA-1 NI):
//   abcd: { A, B, C, D } with A at bits[127:96], D at bits[31:0]
//   e:    { E, 0, 0, 0 } with E at bits[127:96]
//
// Memory order for state[] is { A, B, C, D, E } (index 0..4), so after
// _mm_loadu_si128 the XMM has D at [127:96] and A at [31:0] — the reverse
// of what SHA-NI expects. A single _mm_shuffle_epi32 with imm8=0x1B fixes
// this on load and must be applied again on store.
// --------------------------------------------------------------------------
DATAFORGE_SHA1_TARGET
inline void process_block_sha1_x86(uint_least32_t(&state)[5], const void* msg)
{
    // Byte-swap mask: reverses byte order within each 32-bit lane so that
    // big-endian SHA-1 message words are loaded correctly from memory.
    const __m128i SHUF_MASK = _mm_set_epi64x(
        0x0001020304050607ULL, 0x08090a0b0c0d0e0fULL);

    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    // Load and reorder state: memory A,B,C,D -> XMM[127:96]=A, [31:0]=D
    __m128i abcd = _mm_shuffle_epi32(
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(state)), 0x1B);
    __m128i e0 = _mm_set_epi32(static_cast<int>(state[4]), 0, 0, 0);

    const __m128i abcd_save = abcd;
    const __m128i e0_save   = e0;

    // Load 64-byte message block: 4 x 16-byte registers, byte-swapped to
    // big-endian 32-bit words.
    __m128i msg0 = _mm_shuffle_epi8(
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(data     )), SHUF_MASK);
    __m128i msg1 = _mm_shuffle_epi8(
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16)), SHUF_MASK);
    __m128i msg2 = _mm_shuffle_epi8(
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32)), SHUF_MASK);
    __m128i msg3 = _mm_shuffle_epi8(
        _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48)), SHUF_MASK);

    __m128i e1;

    // Each group below performs 4 SHA-1 rounds and advances the message
    // schedule by 4 words. The msg registers rotate through msg0-msg3 and
    // accumulate partial XORs via sha1msg1 / xor / sha1msg2 so that three
    // groups later the next scheduled word block is ready.

    /* Rounds 0-3, W[0..3] = msg0 */
    e0   = _mm_sha1nexte_epu32(e0, msg0);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 0);
    msg0 = _mm_sha1msg1_epu32(msg0, msg1);

    /* Rounds 4-7, W[4..7] = msg1 */
    e1   = _mm_sha1nexte_epu32(e1, msg1);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 0);
    msg1 = _mm_sha1msg1_epu32(msg1, msg2);
    msg0 = _mm_xor_si128(msg0, msg2);

    /* Rounds 8-11, W[8..11] = msg2 */
    e0   = _mm_sha1nexte_epu32(e0, msg2);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 0);
    msg2 = _mm_sha1msg1_epu32(msg2, msg3);
    msg1 = _mm_xor_si128(msg1, msg3);
    msg0 = _mm_sha1msg2_epu32(msg0, msg3);   /* msg0 = W[16..19] */

    /* Rounds 12-15, W[12..15] = msg3 */
    e1   = _mm_sha1nexte_epu32(e1, msg3);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 0);
    msg3 = _mm_sha1msg1_epu32(msg3, msg0);
    msg2 = _mm_xor_si128(msg2, msg0);
    msg1 = _mm_sha1msg2_epu32(msg1, msg0);   /* msg1 = W[20..23] */

    /* Rounds 16-19, W[16..19] = msg0 */
    e0   = _mm_sha1nexte_epu32(e0, msg0);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 0);
    msg0 = _mm_sha1msg1_epu32(msg0, msg1);
    msg3 = _mm_xor_si128(msg3, msg1);
    msg2 = _mm_sha1msg2_epu32(msg2, msg1);   /* msg2 = W[24..27] */

    /* Rounds 20-23, W[20..23] = msg1 */
    e1   = _mm_sha1nexte_epu32(e1, msg1);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 1);
    msg1 = _mm_sha1msg1_epu32(msg1, msg2);
    msg0 = _mm_xor_si128(msg0, msg2);
    msg3 = _mm_sha1msg2_epu32(msg3, msg2);   /* msg3 = W[28..31] */

    /* Rounds 24-27, W[24..27] = msg2 */
    e0   = _mm_sha1nexte_epu32(e0, msg2);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 1);
    msg2 = _mm_sha1msg1_epu32(msg2, msg3);
    msg1 = _mm_xor_si128(msg1, msg3);
    msg0 = _mm_sha1msg2_epu32(msg0, msg3);   /* msg0 = W[32..35] */

    /* Rounds 28-31, W[28..31] = msg3 */
    e1   = _mm_sha1nexte_epu32(e1, msg3);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 1);
    msg3 = _mm_sha1msg1_epu32(msg3, msg0);
    msg2 = _mm_xor_si128(msg2, msg0);
    msg1 = _mm_sha1msg2_epu32(msg1, msg0);   /* msg1 = W[36..39] */

    /* Rounds 32-35, W[32..35] = msg0 */
    e0   = _mm_sha1nexte_epu32(e0, msg0);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 1);
    msg0 = _mm_sha1msg1_epu32(msg0, msg1);
    msg3 = _mm_xor_si128(msg3, msg1);
    msg2 = _mm_sha1msg2_epu32(msg2, msg1);   /* msg2 = W[40..43] */

    /* Rounds 36-39, W[36..39] = msg1 */
    e1   = _mm_sha1nexte_epu32(e1, msg1);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 1);
    msg1 = _mm_sha1msg1_epu32(msg1, msg2);
    msg0 = _mm_xor_si128(msg0, msg2);
    msg3 = _mm_sha1msg2_epu32(msg3, msg2);   /* msg3 = W[44..47] */

    /* Rounds 40-43, W[40..43] = msg2 */
    e0   = _mm_sha1nexte_epu32(e0, msg2);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 2);
    msg2 = _mm_sha1msg1_epu32(msg2, msg3);
    msg1 = _mm_xor_si128(msg1, msg3);
    msg0 = _mm_sha1msg2_epu32(msg0, msg3);   /* msg0 = W[48..51] */

    /* Rounds 44-47, W[44..47] = msg3 */
    e1   = _mm_sha1nexte_epu32(e1, msg3);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 2);
    msg3 = _mm_sha1msg1_epu32(msg3, msg0);
    msg2 = _mm_xor_si128(msg2, msg0);
    msg1 = _mm_sha1msg2_epu32(msg1, msg0);   /* msg1 = W[52..55] */

    /* Rounds 48-51, W[48..51] = msg0 */
    e0   = _mm_sha1nexte_epu32(e0, msg0);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 2);
    msg0 = _mm_sha1msg1_epu32(msg0, msg1);
    msg3 = _mm_xor_si128(msg3, msg1);
    msg2 = _mm_sha1msg2_epu32(msg2, msg1);   /* msg2 = W[56..59] */

    /* Rounds 52-55, W[52..55] = msg1 */
    e1   = _mm_sha1nexte_epu32(e1, msg1);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 2);
    msg1 = _mm_sha1msg1_epu32(msg1, msg2);
    msg0 = _mm_xor_si128(msg0, msg2);
    msg3 = _mm_sha1msg2_epu32(msg3, msg2);   /* msg3 = W[60..63] */

    /* Rounds 56-59, W[56..59] = msg2 */
    e0   = _mm_sha1nexte_epu32(e0, msg2);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 2);
    msg2 = _mm_sha1msg1_epu32(msg2, msg3);
    msg1 = _mm_xor_si128(msg1, msg3);
    msg0 = _mm_sha1msg2_epu32(msg0, msg3);   /* msg0 = W[64..67] */

    /* Rounds 60-63, W[60..63] = msg3 */
    e1   = _mm_sha1nexte_epu32(e1, msg3);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 3);
    msg3 = _mm_sha1msg1_epu32(msg3, msg0);
    msg2 = _mm_xor_si128(msg2, msg0);
    msg1 = _mm_sha1msg2_epu32(msg1, msg0);   /* msg1 = W[68..71] */

    /* Rounds 64-67, W[64..67] = msg0 */
    e0   = _mm_sha1nexte_epu32(e0, msg0);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 3);
    msg3 = _mm_xor_si128(msg3, msg1);
    msg2 = _mm_sha1msg2_epu32(msg2, msg1);   /* msg2 = W[72..75] */

    /* Rounds 68-71, W[68..71] = msg1 */
    e1   = _mm_sha1nexte_epu32(e1, msg1);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 3);
    msg3 = _mm_sha1msg2_epu32(msg3, msg2);   /* msg3 = W[76..79] */

    /* Rounds 72-75, W[72..75] = msg2 */
    e0   = _mm_sha1nexte_epu32(e0, msg2);
    e1   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e0, 3);

    /* Rounds 76-79, W[76..79] = msg3 */
    e1   = _mm_sha1nexte_epu32(e1, msg3);
    e0   = abcd;
    abcd = _mm_sha1rnds4_epu32(abcd, e1, 3);

    // Accumulate into saved initial state.
    e0   = _mm_sha1nexte_epu32(e0, e0_save);
    abcd = _mm_add_epi32(abcd, abcd_save);

    // Reverse dword order back to memory layout (A at index 0) and store.
    _mm_storeu_si128(reinterpret_cast<__m128i*>(state),
                     _mm_shuffle_epi32(abcd, 0x1B));
    state[4] = static_cast<uint_least32_t>(_mm_extract_epi32(e0, 3));
}

} // namespace dataforge::sha1_detail

#endif // DATAFORGE_ACCEL_CAN_COMPILE_X86_SHA1
