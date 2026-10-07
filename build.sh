#!/usr/bin/env bash
# Configure and build Genexis.
#
# Environment:
#   BUILD_DIR   build directory, relative to the repo root (default: build)
#   BUILD_TYPE  CMake build type (default: Debug)
#   JOBS        parallel build jobs (default: number of online CPUs)
#
# Extra arguments are passed to the CMake configure step, for example:
#   BUILD_TYPE=Release ./build.sh -DBUILD_TESTING=OFF
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_DIR=${BUILD_DIR:-build}
BUILD_TYPE=${BUILD_TYPE:-Debug}
JOBS=${JOBS:-$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)}

# Prefer Ninja for new build directories; an existing one keeps its generator.
generator=()
if [[ ! -f "$BUILD_DIR/CMakeCache.txt" ]] && command -v ninja >/dev/null 2>&1; then
  generator=(-G Ninja)
fi

cmake -S . -B "$BUILD_DIR" ${generator[@]+"${generator[@]}"} \
  -DCMAKE_BUILD_TYPE="$BUILD_TYPE" "$@"
cmake --build "$BUILD_DIR" --parallel "$JOBS"
