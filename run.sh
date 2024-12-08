#!/bin/sh
set -e # Exit early if any commands fail

(
  cd "$(dirname "$0")" 
  cmake -B build/linux -S .
  cmake --build ./build/linux -j 10
)
exec $(dirname $0)/build/linux/interpreter "$@"
