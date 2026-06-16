cmake_minimum_required(VERSION 3.15)
project(dataforge_accel LANGUAGES CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# dataforge_accel — optional assembly acceleration library.
#
# When linked into a project that also includes DataForge headers, define
# DATAFORGE_ACCEL_NATIVE=1 to enable the extern "C" dispatch hooks in
# sha1.ipp / sha2.ipp that call the functions provided by this library
# instead of the built-in C++ backends.
#
# Usage in consuming project:
#   add_subdirectory(<path>/accel)
#   target_link_libraries(myapp PRIVATE dataforge_accel)
#   target_compile_definitions(myapp PRIVATE DATAFORGE_ACCEL_NATIVE=1)
# ---------------------------------------------------------------------------

include(CheckLanguage)
check_language(ASM)

set(ACCEL_SOURCES)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    # AArch64 assembly stubs (to be filled with optimised .S files)
    # list(APPEND ACCEL_SOURCES asm/aarch64/sha1_aarch64.S)
    # list(APPEND ACCEL_SOURCES asm/aarch64/sha512_aarch64.S)
    message(STATUS "dataforge_accel: AArch64 target — assembly stubs pending")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|i686|i386")
    # x86 assembly stubs (to be filled with optimised .S / .asm files)
    # list(APPEND ACCEL_SOURCES asm/x86/sha1_x86.S)
    # list(APPEND ACCEL_SOURCES asm/x86/sha512_x86.S)
    message(STATUS "dataforge_accel: x86 target — assembly stubs pending")
else()
    message(STATUS "dataforge_accel: unknown architecture, no assembly sources")
endif()

if(ACCEL_SOURCES)
    add_library(dataforge_accel STATIC ${ACCEL_SOURCES})
else()
    # Empty interface target so consumers can always link against it
    add_library(dataforge_accel INTERFACE)
endif()

target_include_directories(dataforge_accel INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/../include)
