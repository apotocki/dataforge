#!/bin/bash
cmake -S tests -B build build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j 3
ctest --verbose --test-dir build
