/*=============================================================================
    Copyright (c) Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/

#if DATAFORGE_SHA256_ACCEL_CAN_COMPILE_X86_SHA

#include <immintrin.h>

#if DATAFORGE_SHA256_ACCEL_AUTODETECT
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
#include <cpuid.h>
#endif
#endif

namespace dataforge::sha2_detail {

#if DATAFORGE_SHA256_ACCEL_IMPL == DATAFORGE_SHA256_ACCEL_AUTODETECT_MODE
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

inline uint32_t sha256_rotr_x86(uint32_t value, int shift)
{
    return (value >> shift) | (value << (32 - shift));
}

inline uint32_t sha256_sigma0_x86(uint32_t value)
{
    return sha256_rotr_x86(value, 7) ^ sha256_rotr_x86(value, 18) ^ (value >> 3);
}

inline uint32_t sha256_sigma1_x86(uint32_t value)
{
    return sha256_rotr_x86(value, 17) ^ sha256_rotr_x86(value, 19) ^ (value >> 10);
}

inline uint32_t sha256_load_be32_x86(const uint8_t* src)
{
    return (static_cast<uint32_t>(src[0]) << 24)
        | (static_cast<uint32_t>(src[1]) << 16)
        | (static_cast<uint32_t>(src[2]) << 8)
        | static_cast<uint32_t>(src[3]);
}

inline void process_block_sha256_x86(uint32_t state[8], const void* msg)
{
    __m128i state0;
    __m128i state1;
    __m128i abef_save;
    __m128i cdgh_save;
    __m128i msgv;
    __m128i msg0;
    __m128i msg1;
    __m128i msg2;
    __m128i msg3;
    __m128i tmp;

    const auto* data = reinterpret_cast<const uint8_t*>(msg);
    const __m128i shuf_mask = _mm_set_epi8(
        12, 13, 14, 15,
        8, 9, 10, 11,
        4, 5, 6, 7,
        0, 1, 2, 3);

    tmp = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[0]));
    state1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[4]));

    state0 = _mm_shuffle_epi32(tmp, 0xB1);
    state1 = _mm_shuffle_epi32(state1, 0x1B);
    state0 = _mm_alignr_epi8(state0, state1, 8);
    state1 = _mm_blend_epi16(state1, tmp, 0xF0);

    abef_save = state0;
    cdgh_save = state1;

#define DATAFORGE_SHA256_RNDS4(MSGX, KIDX) \
    msgv = _mm_add_epi32((MSGX), _mm_loadu_si128(reinterpret_cast<const __m128i*>(sha2_def_base<256>::K + (KIDX)))); \
    state1 = _mm_sha256rnds2_epu32(state1, state0, msgv); \
    msgv = _mm_shuffle_epi32(msgv, 0x0E); \
    state0 = _mm_sha256rnds2_epu32(state0, state1, msgv)

    msg0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 0));
    msg1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 16));
    msg2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 32));
    msg3 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(data + 48));

    msg0 = _mm_shuffle_epi8(msg0, shuf_mask);
    msg1 = _mm_shuffle_epi8(msg1, shuf_mask);
    msg2 = _mm_shuffle_epi8(msg2, shuf_mask);
    msg3 = _mm_shuffle_epi8(msg3, shuf_mask);

    DATAFORGE_SHA256_RNDS4(msg0, 0);
    DATAFORGE_SHA256_RNDS4(msg1, 4);
    DATAFORGE_SHA256_RNDS4(msg2, 8);
    DATAFORGE_SHA256_RNDS4(msg3, 12);

    msg0 = _mm_sha256msg1_epu32(msg0, msg1);
    tmp = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    DATAFORGE_SHA256_RNDS4(msg0, 16);

    msg1 = _mm_sha256msg1_epu32(msg1, msg2);
    tmp = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    DATAFORGE_SHA256_RNDS4(msg1, 20);

    msg2 = _mm_sha256msg1_epu32(msg2, msg3);
    tmp = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    DATAFORGE_SHA256_RNDS4(msg2, 24);

    msg3 = _mm_sha256msg1_epu32(msg3, msg0);
    tmp = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    DATAFORGE_SHA256_RNDS4(msg3, 28);

    msg0 = _mm_sha256msg1_epu32(msg0, msg1);
    tmp = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    DATAFORGE_SHA256_RNDS4(msg0, 32);

    msg1 = _mm_sha256msg1_epu32(msg1, msg2);
    tmp = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    DATAFORGE_SHA256_RNDS4(msg1, 36);

    msg2 = _mm_sha256msg1_epu32(msg2, msg3);
    tmp = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    DATAFORGE_SHA256_RNDS4(msg2, 40);

    msg3 = _mm_sha256msg1_epu32(msg3, msg0);
    tmp = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    DATAFORGE_SHA256_RNDS4(msg3, 44);

    msg0 = _mm_sha256msg1_epu32(msg0, msg1);
    tmp = _mm_alignr_epi8(msg3, msg2, 4);
    msg0 = _mm_add_epi32(msg0, tmp);
    msg0 = _mm_sha256msg2_epu32(msg0, msg3);
    DATAFORGE_SHA256_RNDS4(msg0, 48);

    msg1 = _mm_sha256msg1_epu32(msg1, msg2);
    tmp = _mm_alignr_epi8(msg0, msg3, 4);
    msg1 = _mm_add_epi32(msg1, tmp);
    msg1 = _mm_sha256msg2_epu32(msg1, msg0);
    DATAFORGE_SHA256_RNDS4(msg1, 52);

    msg2 = _mm_sha256msg1_epu32(msg2, msg3);
    tmp = _mm_alignr_epi8(msg1, msg0, 4);
    msg2 = _mm_add_epi32(msg2, tmp);
    msg2 = _mm_sha256msg2_epu32(msg2, msg1);
    DATAFORGE_SHA256_RNDS4(msg2, 56);

    msg3 = _mm_sha256msg1_epu32(msg3, msg0);
    tmp = _mm_alignr_epi8(msg2, msg1, 4);
    msg3 = _mm_add_epi32(msg3, tmp);
    msg3 = _mm_sha256msg2_epu32(msg3, msg2);
    DATAFORGE_SHA256_RNDS4(msg3, 60);

    state0 = _mm_add_epi32(state0, abef_save);
    state1 = _mm_add_epi32(state1, cdgh_save);

    tmp = _mm_shuffle_epi32(state0, 0x1B);
    state1 = _mm_shuffle_epi32(state1, 0xB1);
    state0 = _mm_blend_epi16(tmp, state1, 0xF0);
    state1 = _mm_alignr_epi8(state1, tmp, 8);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[0]), state0);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[4]), state1);

#undef DATAFORGE_SHA256_RNDS4
}

}

#endif
