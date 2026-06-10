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

inline void process_block_sha256_x86(uint32_t state[8], const void* msg)
{
    uint32_t W[64];
    const auto* data = reinterpret_cast<const uint8_t*>(msg);

    for (int t = 0; t < 16; ++t)
        W[t] = sha256_load_be32_x86(data + 4 * t);

    for (int t = 16; t < 64; ++t)
        W[t] = sha256_sigma1_x86(W[t - 2]) + W[t - 7] + sha256_sigma0_x86(W[t - 15]) + W[t - 16];

    __m128i state0 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[0]));
    __m128i state1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&state[4]));

    __m128i tmp = _mm_shuffle_epi32(state0, 0xB1);
    state1 = _mm_shuffle_epi32(state1, 0x1B);
    state0 = _mm_alignr_epi8(tmp, state1, 8);
    state1 = _mm_blend_epi16(state1, tmp, 0xF0);

    const __m128i abef_save = state0;
    const __m128i cdgh_save = state1;

    for (int t = 0; t < 64; t += 4)
    {
        __m128i wk = _mm_set_epi32(
            static_cast<int>(W[t + 3] + sha2_def_base<256>::K[t + 3]),
            static_cast<int>(W[t + 2] + sha2_def_base<256>::K[t + 2]),
            static_cast<int>(W[t + 1] + sha2_def_base<256>::K[t + 1]),
            static_cast<int>(W[t + 0] + sha2_def_base<256>::K[t + 0]));

        state1 = _mm_sha256rnds2_epu32(state1, state0, wk);
        wk = _mm_shuffle_epi32(wk, 0x0E);
        state0 = _mm_sha256rnds2_epu32(state0, state1, wk);
    }

    state0 = _mm_add_epi32(state0, abef_save);
    state1 = _mm_add_epi32(state1, cdgh_save);

    tmp = _mm_shuffle_epi32(state0, 0x1B);
    state1 = _mm_shuffle_epi32(state1, 0xB1);
    state0 = _mm_blend_epi16(tmp, state1, 0xF0);
    state1 = _mm_alignr_epi8(state1, tmp, 8);

    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[0]), state0);
    _mm_storeu_si128(reinterpret_cast<__m128i*>(&state[4]), state1);
}

}

#endif
