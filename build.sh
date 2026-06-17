#!/bin/bash
#set  -e

#apt-get install -y zlib1g-dev libicu-dev libbz2-dev liblz4-dev liblzma-dev libgtest-dev

HOST_ARC=$( uname -m )

if [[ "$OSTYPE" == "darwin"* ]]; then
    THREAD_COUNT=$(sysctl -n hw.ncpu)
else
    THREAD_COUNT=$(nproc)
fi

PROJECT_HOME="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

echo PROJECT_HOME = $PROJECT_HOME

if [[ ! -d $PROJECT_HOME/build ]]; then
	mkdir $PROJECT_HOME/build
fi

cmake -S $PROJECT_HOME/tests -B build build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j $THREAD_COUNT
ctest --verbose --test-dir build
