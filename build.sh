#!/usr/bin/env bash
set -ex

mkdir -p build && cd build

conan install ..
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build .
