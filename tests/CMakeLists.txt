cmake_minimum_required(VERSION 3.15)
project(dataforge_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Profile constants — mirror DATAFORGE_PROFILE_* values from config.hpp.
# Use these names everywhere below so the numeric values are never written
# directly in this file.
# ---------------------------------------------------------------------------
set(PROFILE_AUTO       -1)
set(PROFILE_SCALAR      0)
set(PROFILE_X86_SHA_NI  1)
set(PROFILE_X86_AVX512  2)
set(PROFILE_ARM_NEON    3)
set(PROFILE_ARM_CRYPTO  4)

# Enable folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable testing
include(CTest)
enable_testing()

# Collect all test sources
file(GLOB_RECURSE DATAFORGE_TEST_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

if (DATAFORGE_TEST_SOURCES STREQUAL "")
  message(FATAL_ERROR "No test sources found under ${CMAKE_CURRENT_SOURCE_DIR}")
endif()

# If we are using vcpkg, rely on CMake config packages
if(DEFINED ENV{VCPKG_ROOT})
    message(STATUS "Using vcpkg from: $ENV{VCPKG_ROOT}")
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

    find_package(ZLIB REQUIRED)
    find_package(ICU REQUIRED COMPONENTS i18n uc data)
    find_package(BZip2 REQUIRED)
    find_package(LZ4 CONFIG REQUIRED)
    find_package(LibLZMA REQUIRED)
    find_package(GTest CONFIG REQUIRED)

    set(DEPS
        ZLIB::ZLIB
        ICU::i18n
        ICU::uc
        ICU::data
        BZip2::BZip2
        LZ4::lz4
        LibLZMA::LibLZMA
        GTest::gtest
        GTest::gtest_main
    )

else()
    message(STATUS "No vcpkg detected. Using system packages.")

    # On macOS, Homebrew installs icu4c as keg-only (no symlinks into the
    # default prefix), so CMake's FindICU won't see it unless we point it
    # at the cellar. Ask brew for the actual prefix and set ICU_ROOT so that
    # FindICU picks up the right headers and libraries.
    if(APPLE)
        find_program(BREW brew)
        if(BREW)
            execute_process(
                COMMAND ${BREW} --prefix icu4c
                OUTPUT_VARIABLE _brew_icu_prefix
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
            )
            if(_brew_icu_prefix)
                message(STATUS "Homebrew ICU root: ${_brew_icu_prefix}")
                # Prepend to CMAKE_PREFIX_PATH so find_package searches here first.
                list(PREPEND CMAKE_PREFIX_PATH "${_brew_icu_prefix}")
                # Explicitly override the cached FindICU internal variables so that
                # a previous run that picked up Xcode SDK ICU does not win.
                set(ICU_INCLUDE_DIR "${_brew_icu_prefix}/include"
                    CACHE PATH "ICU include directory (Homebrew)" FORCE)
            endif()
        endif()
    endif()

    find_package(ZLIB REQUIRED)
    find_package(ICU REQUIRED COMPONENTS i18n uc data)
    # FindICU.cmake does not always populate INTERFACE_INCLUDE_DIRECTORIES on
    # its imported targets when the package lives outside the default prefix
    # (e.g. Homebrew keg-only on macOS). Use include_directories() as a
    # reliable fallback — ICU_INCLUDE_DIRS is always set by FindICU.
    if(ICU_INCLUDE_DIRS)
        include_directories(${ICU_INCLUDE_DIRS})
    endif()
    find_package(BZip2 REQUIRED)

    # LZ4 fallback search
    find_path(LZ4_INCLUDE_DIR NAMES lz4.h)
    find_library(LZ4_LIBRARY NAMES lz4)
    if(NOT LZ4_INCLUDE_DIR OR NOT LZ4_LIBRARY)
        message(FATAL_ERROR "LZ4 not found. Install liblz4-dev.")
    endif()

    # LibLZMA fallback search
    find_path(LIBLZMA_INCLUDE_DIR NAMES lzma.h)
    find_library(LIBLZMA_LIBRARY NAMES lzma)
    if(NOT LIBLZMA_INCLUDE_DIR OR NOT LIBLZMA_LIBRARY)
        message(FATAL_ERROR "LibLZMA not found. Install liblzma-dev.")
    endif()

    # GTest fallback search
    find_package(GTest REQUIRED)

    set(DEPS
        ZLIB::ZLIB
        ICU::i18n
        ICU::uc
        ICU::data
        BZip2::BZip2
        ${LZ4_LIBRARY}
        ${LIBLZMA_LIBRARY}
        GTest::GTest
        GTest::Main
    )

    include_directories(${LZ4_INCLUDE_DIR} ${LIBLZMA_INCLUDE_DIR})
endif()

# ---------------------------------------------------------------------------
# Helper: build one test binary for a given acceleration profile.
# PROFILE_NAME  — used as a suffix in the target/test name.
# PROFILE_VALUE — the DATAFORGE_PROFILE_* constant to pass to the compiler.
# ---------------------------------------------------------------------------
function(add_accel_test_target PROFILE_NAME PROFILE_VALUE)
    set(TARGET dataforge_tests_${PROFILE_NAME})
    add_executable(${TARGET} ${DATAFORGE_TEST_SOURCES})
    target_include_directories(${TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include)
    target_compile_definitions(${TARGET} PRIVATE
        DATAFORGE_ACCEL_PROFILE=${PROFILE_VALUE})
    target_link_libraries(${TARGET} PRIVATE ${DEPS})
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_compile_options(${TARGET} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
    add_test(NAME ${TARGET} COMMAND ${TARGET})
endfunction()

# ---------------------------------------------------------------------------
# Runtime CPU capability probes (executed at CMake configure time).
# Each probe program returns 0 if the CPU feature is present, 1 otherwise.
# Results are cached; re-run cmake to re-probe (e.g. after changing machines).
# ---------------------------------------------------------------------------
include(CheckCXXSourceRuns)

# Probe: x86 SHA-NI (CPUID leaf 7, EBX bit 29)
set(PROBE_X86_SHA_NI [=[
    #if defined(_MSC_VER)
    #  include <intrin.h>
    #else
    #  include <cpuid.h>
    #endif
    int main() {
    #if defined(_MSC_VER)
        int info[4]; __cpuidex(info, 7, 0);
        return ((info[1] >> 29) & 1) ? 0 : 1;
    #else
        unsigned a, b, c, d;
        if (!__get_cpuid_count(7, 0, &a, &b, &c, &d)) return 1;
        return ((b >> 29) & 1) ? 0 : 1;
    #endif
    }
]=])

# Probe: x86 AVX-512F + AVX-512VL + OS register-state support (XCR0 bits)
set(PROBE_X86_AVX512 [=[
    #if defined(_MSC_VER)
    #  include <intrin.h>
    #  include <immintrin.h>
    #else
    #  include <cpuid.h>
    #endif
    int main() {
    #if defined(_MSC_VER)
        int info[4]; __cpuidex(info, 7, 0);
        if (!((info[1] >> 16) & 1) || !((info[1] >> 31) & 1)) return 1;
        unsigned long long xcr0 = _xgetbv(0);
    #else
        unsigned a, b, c, d;
        if (!__get_cpuid_count(7, 0, &a, &b, &c, &d)) return 1;
        if (!((b >> 16) & 1) || !((b >> 31) & 1)) return 1;
        unsigned eax2, edx2;
        __asm__("xgetbv" : "=a"(eax2), "=d"(edx2) : "c"(0u));
        unsigned long long xcr0 = ((unsigned long long)edx2 << 32) | eax2;
    #endif
        // Bits 1-2: SSE, bits 5-7: AVX-512 opmask/ZMM-hi/ZMM-hi16
        return ((xcr0 & 0xE6u) == 0xE6u) ? 0 : 1;
    }
]=])

# Probe: AArch64 SHA2 crypto extension
set(PROBE_ARM_CRYPTO [=[
    #if defined(__APPLE__)
    #  include <sys/sysctl.h>
    int main() {
        int v = 0; size_t sz = sizeof(v);
        sysctlbyname("hw.optional.arm.FEAT_SHA256", &v, &sz, nullptr, 0);
        return v ? 0 : 1;
    }
    #elif defined(__linux__)
    #  include <sys/auxv.h>
    #  include <asm/hwcap.h>
    int main() { return (getauxval(AT_HWCAP) & HWCAP_SHA2) ? 0 : 1; }
    #else
    int main() { return 1; }
    #endif
]=])

# ---------------------------------------------------------------------------
# Register test targets — always + per-architecture probed variants.
# ---------------------------------------------------------------------------
message(STATUS "DataForge: configuring acceleration test targets")

# AUTO and SCALAR are always built and always run.
add_accel_test_target(auto   ${PROFILE_AUTO})
add_accel_test_target(scalar ${PROFILE_SCALAR})
message(STATUS "  [enabled] auto   (DATAFORGE_ACCEL_PROFILE=${PROFILE_AUTO})")
message(STATUS "  [enabled] scalar (DATAFORGE_ACCEL_PROFILE=${PROFILE_SCALAR})")

# x86 profiles
if(CMAKE_SYSTEM_PROCESSOR MATCHES "AMD64|x86_64|i[3-6]86|X86")

    check_cxx_source_runs("${PROBE_X86_SHA_NI}"  DATAFORGE_CPU_HAS_X86_SHA_NI)
    check_cxx_source_runs("${PROBE_X86_AVX512}"  DATAFORGE_CPU_HAS_X86_AVX512)

    if(DATAFORGE_CPU_HAS_X86_SHA_NI)
        add_accel_test_target(x86_sha_ni ${PROFILE_X86_SHA_NI})
        message(STATUS "  [enabled] x86_sha_ni (DATAFORGE_ACCEL_PROFILE=${PROFILE_X86_SHA_NI})")
    else()
        message(STATUS "  [skipped] x86_sha_ni — SHA-NI not available on this CPU")
    endif()

    if(DATAFORGE_CPU_HAS_X86_AVX512)
        add_accel_test_target(x86_avx512 ${PROFILE_X86_AVX512})
        message(STATUS "  [enabled] x86_avx512 (DATAFORGE_ACCEL_PROFILE=${PROFILE_X86_AVX512})")
    else()
        message(STATUS "  [skipped] x86_avx512 — AVX-512 not available on this CPU")
    endif()

endif()

# AArch64 profiles
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64|ARM64|AARCH64")

    # ARM_NEON: NEON is mandatory on all AArch64 — always build.
    add_accel_test_target(arm_neon ${PROFILE_ARM_NEON})
    message(STATUS "  [enabled] arm_neon (DATAFORGE_ACCEL_PROFILE=${PROFILE_ARM_NEON})")

    check_cxx_source_runs("${PROBE_ARM_CRYPTO}"  DATAFORGE_CPU_HAS_ARM_CRYPTO)

    if(DATAFORGE_CPU_HAS_ARM_CRYPTO)
        add_accel_test_target(arm_crypto ${PROFILE_ARM_CRYPTO})
        message(STATUS "  [enabled] arm_crypto (DATAFORGE_ACCEL_PROFILE=${PROFILE_ARM_CRYPTO})")
    else()
        message(STATUS "  [skipped] arm_crypto — SHA2 crypto extensions not available on this CPU")
    endif()

endif()
