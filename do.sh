#!/bin/sh

if [ "$1" != "" ]; then
    echo "Compiling $1"
    ./pcl < $1 > a.ll || exit 1
    llc a.ll -o a.s
    clang a.s lib.a -o a.out
fi
