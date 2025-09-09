#!/bin/env bash
buildfunc ()
{
    cd build
    cmake ../src -DCompileTests=OFF || return 1
    cmake --build . || return 1
    ctest -V . || return 1
}

if [ ! -d build ]; then
    mkdir build
    echo "/build directory created"
fi

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
echo ${BOLD}=== BUILD START ===${NORM}
if buildfunc; then
    echo ${GOOD}=== ALL GOOD!! ===${NORM}
else
    echo ${BAD}=== ERRORS FOUND ===${NORM}
fi
