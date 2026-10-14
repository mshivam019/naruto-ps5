#!/usr/bin/env bash
set -euo pipefail
root=$(cd -- "$(dirname -- "$0")/.." && pwd)
src="$root/upstream/naruto-rise-recomp"
expected=226af94268de3660c3de24061f9a6bf7ae982d3c
[[ $(git -C "$src" rev-parse HEAD) == "$expected" ]] || { echo 'Unexpected Naruto source revision' >&2; exit 1; }
git -C "$src" submodule update --init --recursive
[[ $(git -C "$src/sdk" rev-parse HEAD) == c94f5ebdcb3c9d1a460ca48e04f9758448f8d518 ]] || { echo "Unexpected SDK revision" >&2; exit 1; }
for patch in "$src"/patches/sdk/*.patch; do
  if ! git -C "$src/sdk" apply --reverse --check "$patch" 2>/dev/null; then
    git -C "$src/sdk" apply --check "$patch"
    git -C "$src/sdk" apply "$patch"
  fi
done
cmake -S "$src/sdk" -B "$root/build/host" -G Ninja \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_C_FLAGS=-march=x86-64-v2 -DCMAKE_CXX_FLAGS=-march=x86-64-v2 \
  -DCMAKE_BUILD_TYPE=Release -DREXGLUE_ENABLE_TRACY=OFF \
  -DREXGLUE_ENABLE_FIDELITYFX=OFF -DREXGLUE_ENABLE_PERF_COUNTERS=OFF
cmake --build "$root/build/host" --target rexglue -j "${NARUTO_BUILD_JOBS:-4}"
