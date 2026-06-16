/*=============================================================================
    Copyright (c) 2026 Alexander Pototskiy

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#pragma once

// Set to 1 (via -DDATAFORGE_ACCEL_NATIVE=1) when the project links the
// optional dataforge_accel library (accel/CMakeLists.txt).  The hash
// implementations will then call the extern "C" functions provided by that
// library instead of the built-in C++ backends.
#ifndef DATAFORGE_ACCEL_NATIVE
#define DATAFORGE_ACCEL_NATIVE 0
#endif

// ---------------------------------------------------------------------------
// Acceleration profile — the single user-facing knob.
//
// Set via -DDATAFORGE_ACCEL_PROFILE=<value> at compile time.
// Each profile selects a coherent ISA tier across ALL accelerated algorithms:
// adding acceleration for a new algorithm only requires mapping it to the
// existing profiles in its own header — no new top-level macros needed.
//
//   DATAFORGE_PROFILE_AUTO       (-1)  Runtime CPU detection (default).
//                                       Picks the best available backend at
//                                       first use; zero overhead after that.
//
//   DATAFORGE_PROFILE_SCALAR      (0)  Pure C++ — no ISA intrinsics.
//                                       Always compiles and runs everywhere.
//
//   DATAFORGE_PROFILE_X86_SHA_NI  (1)  x86: SHA-NI + SSE4.1 (forced, no
//                                       runtime check). Requires CPU support.
//
//   DATAFORGE_PROFILE_X86_AVX512  (2)  x86: SHA-NI + AVX-512F/VL (forced).
//                                       Enables AVX-512 message schedule for
//                                       SHA-512 *and* SHA-224/256.
//                                       Requires CPU support.
//
//   DATAFORGE_PROFILE_ARM_NEON    (3)  AArch64: NEON-vectorised SHA-512
//                                       message schedule (available on all
//                                       AArch64); SHA-1/224/256 use scalar.
//                                       Runs on any AArch64 CPU.
//
//   DATAFORGE_PROFILE_ARM_CRYPTO  (4)  AArch64: SHA2 crypto extension for
//                                       SHA-1/224/256, SHA-512 extension for
//                                       SHA-384/512 (with NEON fallback).
//                                       Requires CPU support (ARMv8+crypto).
// ---------------------------------------------------------------------------
#define DATAFORGE_PROFILE_AUTO        (-1)
#define DATAFORGE_PROFILE_SCALAR        0
#define DATAFORGE_PROFILE_X86_SHA_NI    1
#define DATAFORGE_PROFILE_X86_AVX512    2
#define DATAFORGE_PROFILE_ARM_NEON      3
#define DATAFORGE_PROFILE_ARM_CRYPTO    4

#ifndef DATAFORGE_ACCEL_PROFILE
#define DATAFORGE_ACCEL_PROFILE DATAFORGE_PROFILE_AUTO
#endif

// Catch renamed/removed macros and guide the user to the new API.
#ifdef DATAFORGE_ACCEL_FORCE
#  pragma message("DATAFORGE_ACCEL_FORCE is deprecated; use -DDATAFORGE_ACCEL_PROFILE=<value> instead")
#endif
#ifdef DATAFORGE_ACCEL_AUTODETECT
#  pragma message("DATAFORGE_ACCEL_AUTODETECT is deprecated; use -DDATAFORGE_ACCEL_PROFILE instead (DATAFORGE_PROFILE_AUTO enables autodetect)")
#endif
#ifdef DATAFORGE_ACCEL_X86_SHA256_USE_AVX512
#  pragma message("DATAFORGE_ACCEL_X86_SHA256_USE_AVX512 is deprecated; use -DDATAFORGE_ACCEL_PROFILE=DATAFORGE_PROFILE_X86_AVX512 instead")
#endif

// ---------------------------------------------------------------------------
// Target architecture detection (compile-time, not user-settable).
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// Internal IMPL codes — used inside .ipp files; not part of the public API.
// ---------------------------------------------------------------------------
#define DATAFORGE_ACCEL_NONE             0
#define DATAFORGE_ACCEL_X86              1
#define DATAFORGE_ACCEL_ARM              2
#define DATAFORGE_ACCEL_AUTODETECT_MODE  3

// Derive DATAFORGE_ACCEL_IMPL from the chosen profile.
// Profiles that target an architecture unavailable on this platform are
// silently downgraded to SCALAR — the .ipp guards (CAN_COMPILE_*) ensure
// no intrinsic code is emitted on unsupported targets.
#if DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_AUTO
#  define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_AUTODETECT_MODE
#elif DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_SCALAR
#  define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#elif DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_X86_SHA_NI
#  if DATAFORGE_TARGET_X86
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_X86
#  else
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#  endif
#elif DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_X86_AVX512
#  if DATAFORGE_TARGET_X86
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_X86
#  else
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#  endif
#elif DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_ARM_NEON
#  if DATAFORGE_TARGET_ARM64
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_ARM
#  else
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#  endif
#elif DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_ARM_CRYPTO
#  if DATAFORGE_TARGET_ARM
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_ARM
#  else
#    define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#  endif
#else
#  pragma message("Unknown DATAFORGE_ACCEL_PROFILE value; falling back to scalar")
#  define DATAFORGE_ACCEL_IMPL  DATAFORGE_ACCEL_NONE
#endif

// Internal flag: enable ARM SHA2/SHA-512 crypto extension intrinsics.
// ARM_NEON profile keeps DATAFORGE_ACCEL_IMPL=ARM but uses NEON for SHA-512
// and scalar for SHA-1/224/256 — no crypto extension required.
// ARM_CRYPTO profile additionally enables SHA2/SHA-512 extension intrinsics.
#if DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_ARM_CRYPTO
#  define DATAFORGE_ACCEL_ARM_USE_CRYPTO  1
#else
#  define DATAFORGE_ACCEL_ARM_USE_CRYPTO  0
#endif

// Internal flag: enable AVX-512 message schedule for SHA-224/256.
// Unlike SHA-512 (where AVX-512 is the only vectorised path), SHA-224/256
// has SHA-NI which is faster per block. AVX-512 for SHA-224/256 is therefore
// not enabled in AUTO mode — only when the X86_AVX512 profile is explicitly
// selected. Algorithm headers check this flag; do not set it directly.
#if DATAFORGE_ACCEL_PROFILE == DATAFORGE_PROFILE_X86_AVX512
#  define DATAFORGE_ACCEL_X86_USE_AVX512  1
#else
#  define DATAFORGE_ACCEL_X86_USE_AVX512  0
#endif

// ---------------------------------------------------------------------------
// Compiler helpers
// ---------------------------------------------------------------------------
#if defined(__GNUC__) || defined(__clang__)
#   define DATAFORGE_FORCEINLINE    inline __attribute__((always_inline))
#else
#   define DATAFORGE_FORCEINLINE    __forceinline
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
#else
#   define DATAFORGE_SHA_TARGET
#   define DATAFORGE_AVX512_TARGET
#   define DATAFORGE_SSE41_TARGET
#endif
#endif
