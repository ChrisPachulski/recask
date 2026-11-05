#!/bin/bash
# Hex 3.3.0, a dictation app whose official macOS release is built for Apple silicon only, translated and run on an
# Intel Mac. The translation goes into a copy of the app bundle in place of the arm64 executable and runs with the UI
# guard off, so it opens its windows and draws with the GPU. framegrab.dylib writes the first frame each of its Metal
# layers presents to frames/. Usage: examples/hex/run.sh [work directory]; HEX_SECONDS (default 30) is how long the
# app runs before the script quits it. Exits 1 if the app stops before then or its first window draws nothing.
set -euo pipefail
here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/../.." && pwd)"
work="${1:-$here/work}"
py="$root/sbtenv/bin/python"
seconds="${HEX_SECONDS:-30}"
zip_url=https://github.com/anomalyco/hex/releases/download/v3.3.0/hex-3.3.0-macos-arm64.zip
zip_sha256=1c28d6a1e790323ef199b481bc9231e6ad8e70a1785be84913e0a7bf0a6ebb39   # GitHub release asset digest

[ "$(uname -m)" = x86_64 ] || { echo "Run this on an Intel Mac: on Apple silicon, open the release itself."; exit 1; }
mkdir -p "$work" && cd "$work"
[ -f hex.zip ] || curl -fsSL -o hex.zip "$zip_url"
got="$(shasum -a 256 hex.zip | cut -d' ' -f1)"
[ "$got" = "$zip_sha256" ] || { echo "FAILED: hex.zip sha256 is $got, expected $zip_sha256"; exit 1; }
rm -rf release && mkdir release && (cd release && ditto -x -k ../hex.zip .)
file release/Hex.app/Contents/MacOS/hex

rm -rf out
"$py" "$root/tools/translate.py" --ui-guard-off release/Hex.app/Contents/MacOS/hex out | tail -1 || true   # counts its unsupported instructions
[ -x out/translated ] || { echo "FAILED: no translated executable"; exit 1; }
rm -rf Hex-x86_64.app && cp -R release/Hex.app Hex-x86_64.app && cp out/translated Hex-x86_64.app/Contents/MacOS/hex
clang -fobjc-arc -dynamiclib -arch x86_64 -o framegrab.dylib "$here/framegrab.m" \
      -framework Foundation -framework Metal -framework QuartzCore -framework ImageIO -framework CoreGraphics

rm -rf frames && mkdir frames
# Started directly, not through a system binary: macOS strips DYLD_ variables from those.
FRAMEGRAB_DIR="$work/frames" DYLD_INSERT_LIBRARIES="$work/framegrab.dylib" Hex-x86_64.app/Contents/MacOS/hex 2> hex.log &
pid=$!
sleep "$seconds"
if ! kill -0 "$pid" 2>/dev/null; then
  wait "$pid" && status=0 || status=$?
  echo "FAIL Hex stopped after less than $seconds s, exit status $status; hex.log ends:"; tail -5 hex.log; exit 1
fi
kill "$pid"; wait "$pid" 2>/dev/null || true
grep -a "^framegrab:" hex.log || true
best="$(grep -a "^framegrab: layer " hex.log | sed -E 's/^framegrab: layer ([0-9]+) .*, ([0-9]+) of .*/\2 \1/' | sort -n | tail -1)"
[ -n "$best" ] && [ "${best% *}" -gt 0 ] || { echo "FAIL no Hex window drew anything"; exit 1; }
echo "PASS Hex ran for $seconds s; its fullest frame: $work/frames/layer_${best#* }.png"
