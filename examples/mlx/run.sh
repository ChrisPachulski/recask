#!/bin/bash
# Apple's MLX 0.32.4 on an Intel Mac. MLX's build refuses x86-64 macOS unless MLX_ENABLE_X64_MAC is set, and its Metal
# backend needs an Apple GPU. This builds MLX from Apple's source for arm64 with the CPU backend, builds spiral.cpp
# against it, translates the program, and compares the training run with expected.txt, recorded on an Apple-silicon Mac.
# Usage: examples/mlx/run.sh [work directory]; needs cmake and git; exits 1 if the output differs.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
work="${1:-$here/work}"
py="$root/sbtenv/bin/python"
mlx_url=https://github.com/ml-explore/mlx.git
mlx_tag=v0.32.4
mlx_commit=36969c094354086e15d2a327a9c50feb20b9cf11  # the v0.32.4 tag
command -v cmake > /dev/null || { echo "FAILED: cmake is not on PATH"; exit 1; }

mkdir -p "$work" && cd "$work"
[ -d src ] || git clone -q --depth 1 --branch "$mlx_tag" "$mlx_url" src
[ "$(git -C src rev-parse HEAD)" = "$mlx_commit" ] || { echo "FAILED: src is not MLX $mlx_tag ($mlx_commit)"; exit 1; }
sdk="$(xcrun --show-sdk-path)"
cxx_flags="-nostdinc++ -isystem $sdk/usr/include/c++/v1"  # the SDK's libc++ headers, as the beds use them
cmake -S src -B build -DCMAKE_SYSTEM_NAME=Darwin -DCMAKE_SYSTEM_PROCESSOR=arm64 -DCMAKE_OSX_ARCHITECTURES=arm64 -DCMAKE_OSX_SYSROOT="$sdk" \
      -DCMAKE_CXX_FLAGS="$cxx_flags" \
      -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON -DMLX_BUILD_METAL=OFF -DMLX_BUILD_TESTS=OFF -DMLX_BUILD_EXAMPLES=OFF \
      -DMLX_BUILD_BENCHMARKS=OFF -DMLX_BUILD_PYTHON_STUBS=OFF -DMLX_USE_CCACHE=OFF > cmake.log
cmake --build build -j "$(sysctl -n hw.ncpu)" > build.log
clang++ -std=c++20 -O2 -arch arm64 -isysroot "$sdk" $cxx_flags -I src "$here/spiral.cpp" -L build -lmlx \
        -Wl,-rpath,@executable_path/build -o spiral
file spiral build/libmlx.dylib
"$py" "$root/tools/translate.py" spiral out-spiral > translate.txt || true  # exit 1 also means unsupported instructions
tail -1 translate.txt
grep -q "build error: none$" translate.txt || { echo "FAILED: the translation did not build"; exit 1; }
grep -q "unsupported instructions: 0 " translate.txt || echo "NOTE: the unsupported instructions sit in MLX's float16, float64" \
  "and complex kernels (half-precision arithmetic is a stop by design); each stops the program loudly" \
  "if it runs, and spiral.cpp runs float32 only."

t0=$("$py" -c 'import time; print(time.time())')
./out-spiral/translated > got.txt
echo "translated run: $("$py" -c 'import sys,time; print("%.1f s" % (time.time()-float(sys.argv[1])))' "$t0")"
cat got.txt
if cmp -s "$here/expected.txt" got.txt; then echo "PASS spiral training: identical to Apple silicon"; else
  echo "FAIL spiral training differs from Apple silicon:"; diff "$here/expected.txt" got.txt || true; exit 1; fi
