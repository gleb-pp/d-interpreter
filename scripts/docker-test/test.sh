#!/bin/env sh

testfunc ()
{
    cd /build
    cmake /project -DTesting=ON -DInstallSDK=ON -DInstallInterpreter=ON -DCMAKE_BUILD_TYPE=Debug || return 1
    cmake --build . || return 1
    ctest -V . || return 1
    cmake --install . || return 1
    dinterp /sampleprogram.d || return 1
    cd /sampleproj
    mkdir build && cd build
    cmake .. || return 1
    cmake --build . || return 1
    ctest -V . || return 1
}

GOOD=""
BAD=""
BOLD=""
NORM=""

if [ -t 1 -a $(tput colors) -ge 8 ]; then
    GOOD=$(tput setaf 2)
    BAD=$(tput setaf 1)
    BOLD=$(tput setaf 4)
    NORM=$(tput sgr0)
fi

echo ${BOLD}=== TESTING START ===${NORM}
if testfunc; then
    echo ${GOOD}=== ALL GOOD!! ===${NORM}
else
    echo ${BAD}=== ERRORS FOUND ===${NORM}
fi
