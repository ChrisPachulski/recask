#!/bin/bash
# LLVM's own clang 23.1.4, from its official macOS release (published for Apple silicon only), translated on an Intel Mac,
# then used to build SQLite 3.54.1 for x86-64. The sqlite3 it builds must give the same output as one built from the same
# preprocessed source by Apple's native clang. Run on an Apple-silicon Mac, the script uses clang natively and prints the
# hashes to compare. Usage: examples/clang/run.sh [work directory]; exits 1 if any output differs.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
sqlite="$root/examples/sqlite"
work="${1:-$here/work}"
py="$root/sbtenv/bin/python"
llvm=LLVM-23.1.4-macOS-ARM64
llvm_url=https://github.com/llvm/llvm-project/releases/download/llvmorg-23.1.4/$llvm.tar.xz
llvm_sha256=5d3179db39adeca5b17a34d82708fe976bc60e055b05fe557878fb3fb64d39ef   # GitHub release asset digest
clang_sha256=2f6a5494a1199c5518dadeb58713d4994bcead1769f10d9270ed61d1e2fdc0ec  # bin/clang-23 inside it
zip_url=https://sqlite.org/2026/sqlite-amalgamation-3540100.zip
zip_sha3=92281c86e2957f9b9305890f9ac04204caa7b677cc868dc326d942689df28e30  # sqlite.org download page
defines=(-DSQLITE_ENABLE_FTS5 -DSQLITE_ENABLE_MATH_FUNCTIONS -DSQLITE_ENABLE_RTREE)
target=(-target x86_64-apple-macos13)
sdk="$(xcrun --show-sdk-path)"
fail=0

check() {  # check <name> <file> <hash> <algorithm>
  local got
  got="$("$py" -c 'import hashlib,sys; print(hashlib.new(sys.argv[2], open(sys.argv[1], "rb").read()).hexdigest())' "$2" "$4")"
  [ "$got" = "$3" ] || { echo "FAILED: $1 $4 is $got, expected $3"; exit 1; }
}
seconds() { "$py" -c 'import subprocess,sys,time; t=time.time(); subprocess.run(sys.argv[1:], check=True); print("%.1f" % (time.time()-t))' "$@"; }
same() {  # same <name> <file> <file>
  if cmp -s "$2" "$3"; then echo "PASS $1"; else echo "FAIL $1: $2 and $3 differ"; fail=1; fi
}

mkdir -p "$work" && cd "$work"
[ -f llvm.tar.xz ] || curl -fsSL -o llvm.tar.xz "$llvm_url"
[ -f sqlite.zip ] || curl -fsSL -o sqlite.zip "$zip_url"
check "$llvm.tar.xz" llvm.tar.xz "$llvm_sha256" sha256
check sqlite.zip sqlite.zip "$zip_sha3" sha3_256
[ -f "$llvm/bin/clang-23" ] || tar -xf llvm.tar.xz "$llvm/bin/clang-23" "$llvm/lib/clang"
check clang-23 "$llvm/bin/clang-23" "$clang_sha256" sha256
rm -rf src && mkdir src && (cd src && unzip -q ../sqlite.zip) && src="$work/src/sqlite-amalgamation-3540100"
file "$llvm/bin/clang-23"

if [ "$(uname -m)" = arm64 ]; then
  clang=("$llvm/bin/clang-23")  # Apple silicon: the reference run, no translation
else
  "$py" "$root/tools/translate.py" "$llvm/bin/clang-23" out-clang | tail -1
  clang=(./out-clang/translated)
fi
cc=("${clang[@]}" -resource-dir "$work/$llvm/lib/clang/23" "${target[@]}")
sys=(-isysroot "$sdk")  # not for -c: it stamps the SDK version into the object

for u in sqlite3 shell; do
  "${cc[@]}" "${sys[@]}" -E -P "${defines[@]}" "$src/$u.c" -o "$u.i"
done
echo "sqlite3.i compile seconds, clang 23: $(seconds "${cc[@]}" -w -O2 -c sqlite3.i -o sqlite3.o)"
"${cc[@]}" -w -O2 -c shell.i -o shell.o
shasum -a 256 sqlite3.i shell.i sqlite3.o shell.o
[ "$(uname -m)" = arm64 ] && { echo "Apple silicon: compare these hashes with an Intel Mac's run; equal .i files give equal .o files."; exit 0; }

"${cc[@]}" "${sys[@]}" sqlite3.o shell.o -o sqlite3.clang23
echo "sqlite3.i compile seconds, Apple clang (native, a different version): $(seconds /usr/bin/clang "${target[@]}" -isysroot "$sdk" -w -O2 -c sqlite3.i -o sqlite3.apple.o)"
/usr/bin/clang "${target[@]}" -isysroot "$sdk" -w -O2 sqlite3.i shell.i -o sqlite3.apple
file sqlite3.clang23
for bin in sqlite3.clang23 sqlite3.apple; do
  "./$bin" :memory: < "$sqlite/suite.sql" > "suite.memory.$bin.txt"
  rm -f "file.$bin.db" && "./$bin" "file.$bin.db" < "$sqlite/suite.sql" > "suite.file.$bin.txt"
  "./$bin" < "$sqlite/mandelbrot.sql" > "mandelbrot.$bin.txt"
done
same "suite.sql, in-memory database" suite.memory.sqlite3.apple.txt suite.memory.sqlite3.clang23.txt
same "suite.sql, on-disk database" suite.file.sqlite3.apple.txt suite.file.sqlite3.clang23.txt
same "mandelbrot.sql" mandelbrot.sqlite3.apple.txt mandelbrot.sqlite3.clang23.txt
exit $fail
