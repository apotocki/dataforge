cmake_minimum_required(VERSION 3.15)
project(dataforge LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Root CMakeLists — developer entry point.
# Adds the test suite and the optional assembly acceleration library.
#
# The library itself is header-only; this file is NOT needed to use DataForge
# in your own project — just add include/ to your include path.
# ---------------------------------------------------------------------------

add_subdirectory(tests)
add_subdirectory(accel)
