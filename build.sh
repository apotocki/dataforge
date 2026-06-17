#!/bin/bash
#set  -e

#apt-get install -y zlib1g-dev libicu-dev libbz2-dev liblz4-dev liblzma-dev libgtest-dev

HOST_ARC=$( uname -m )
THREAD_COUNT=$(nproc)
PROJECT_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo PROJECT_HOME = $PROJECT_HOME

if [[ ! -d $PROJECT_HOME/build ]]; then
	mkdir $PROJECT_HOME/build
fi

# -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake
#-DCMAKE_BUILD_TYPE=Release

cmake -S $PROJECT_HOME/tests -B build build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j $THREAD_COUNT
ctest --verbose --test-dir build
#build/dataforge_tests
#cmake -DCMAKE_TOOLCHAIN_FILE=$PROJECT_HOME/projects/cmake/android.toolchain.cmake  $PROJECT_HOME/projects/cmake/ -DBUILD_TYPE=SHARED -DNO_SFTP=1 -DNO_SSL=1 -DCMAKE_CXX_FLAGS="-Wno-shorten-64-to-32 -Wno-unknown-warning-option"
# -DCMAKE_XCODE_ARCHS="x86_64"
# -DCMAKE_IOS_INSTALL_COMBINED=False -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
#cmake --build . --config Release --target efs-core -j $THREAD_COUNT
#cmake --build . --config Release --target efs-tests-lib -j $THREAD_COUNT

