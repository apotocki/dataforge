/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

#ifndef DATAFORGE_ACCEL_AUTODETECT
#define DATAFORGE_ACCEL_AUTODETECT 1
#endif

#ifndef DATAFORGE_ACCEL_FORCE
#define DATAFORGE_ACCEL_FORCE -1
#endif

#ifndef DATAFORGE_ACCEL_NONE
#define DATAFORGE_ACCEL_NONE 0
#endif

#ifndef DATAFORGE_ACCEL_X86
#define DATAFORGE_ACCEL_X86 1
#endif

#ifndef DATAFORGE_ACCEL_ARM
#define DATAFORGE_ACCEL_ARM 2
#endif

#ifndef DATAFORGE_ACCEL_AUTODETECT_MODE
#define DATAFORGE_ACCEL_AUTODETECT_MODE 3
#endif

// Opt-in: use the x86 AVX-512 message-schedule backend for SHA-224/SHA-256
// instead of SHA-NI. SHA-NI is faster for a single block, so this is off by
// default; it only makes sense on parts where AVX-512 is preferable for the
// caller's workload. Has no effect unless an x86 AVX-512 capable CPU is present
// (auto-detect) or the build targets AVX-512 (forced x86).
#ifndef DATAFORGE_ACCEL_X86_SHA256_USE_AVX512
#define DATAFORGE_ACCEL_X86_SHA256_USE_AVX512 0
#endif

#if defined(__x86_64__) || defined(__i386__) || defined(_M_X64) || defined(_M_IX86)
#define DATAFORGE_TARGET_X86 1
#else
#define DATAFORGE_TARGET_X86 0
#endif

#if defined(__aarch64__) || defined(__arm__) || defined(_M_ARM64)
#define DATAFORGE_TARGET_ARM 1
#else
#define DATAFORGE_TARGET_ARM 0
#endif

#if defined(__aarch64__) || defined(_M_ARM64)
#define DATAFORGE_TARGET_ARM64 1
#else
#define DATAFORGE_TARGET_ARM64 0
#endif

#if DATAFORGE_ACCEL_FORCE >= 0
#define DATAFORGE_ACCEL_IMPL DATAFORGE_ACCEL_FORCE
#elif DATAFORGE_ACCEL_AUTODETECT
#define DATAFORGE_ACCEL_IMPL DATAFORGE_ACCEL_AUTODETECT_MODE
#else
#define DATAFORGE_ACCEL_IMPL DATAFORGE_ACCEL_NONE
#endif

#if DATAFORGE_TARGET_X86 == 1
// Per-function ISA selection. On GCC/Clang each accelerated routine carries its
// own target attribute, so the file compiles even when the global -m flags do
// not enable SHA / AVX-512; the run-time dispatcher only ever calls a routine on
// a CPU that actually supports it. On MSVC the intrinsics are always available.
// NB: CPU-detection helpers deliberately get NO target attribute — they must run
// on the baseline ISA.
#if defined(__GNUC__) || defined(__clang__)
#   define DATAFORGE_SHA_TARGET     __attribute__((target("sha,sse4.1")))
#   define DATAFORGE_AVX512_TARGET  __attribute__((target("avx512f,avx512vl,sse4.1")))
#   define DATAFORGE_SSE41_TARGET   __attribute__((target("sse4.1")))
#   define DATAFORGE_FORCEINLINE    inline __attribute__((always_inline))
#else
#   define DATAFORGE_SHA_TARGET
#   define DATAFORGE_AVX512_TARGET
#   define DATAFORGE_SSE41_TARGET
#   define DATAFORGE_FORCEINLINE    __forceinline
#endif
#endif
