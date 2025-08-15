cmake_minimum_required(VERSION 3.15)
project(dataforge_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Enable folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable testing
include(CTest)
enable_testing()

# Collect all test sources
file(GLOB_RECURSE DATAFORGE_TEST_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp)
  
if (DATAFORGE_TEST_SOURCES STREQUAL "")
  message(FATAL_ERROR "No test sources found under ${CMAKE_CURRENT_SOURCE_DIR}/tests")
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

    find_package(ZLIB REQUIRED)
    find_package(ICU REQUIRED COMPONENTS i18n uc data)
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

# Define the test executable
add_executable(dataforge_tests ${DATAFORGE_TEST_SOURCES})
target_include_directories(dataforge_tests PRIVATE include)

# Nice warnings for GCC/Clang
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(dataforge_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Link dependencies
target_link_libraries(dataforge_tests PRIVATE ${DEPS})

# Add the test to CTest
add_test(NAME dataforge_tests COMMAND dataforge_tests)
