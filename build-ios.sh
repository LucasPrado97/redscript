#!/bin/bash
# Builds the scc C library (redscript 0.5.32) for the iPhone and, with the very same code path, for the Mac:
#   dist/ios/libscc_lib.a      static, to link into the CyPort shim (add -framework CoreFoundation, -dead_strip)
#   dist/ios/libscc_lib.dylib  dynamic variant (install name @rpath/libscc_lib.dylib; sign it with the app)
#   dist/macos/...             same crate + same `ios` feature for aarch64-apple-darwin, used by the tests
#   dist/include/scc_ios.h     prototypes
#   dist/compile_test_{mac,ios} the test harness (the iOS one only proves that it links)
# Usage: ./build-ios.sh [--test]     --test also compiles the current CyPort mod set on the Mac
set -euo pipefail
cd "$(dirname "$0")"
export PATH="$HOME/.cargo/bin:$PATH"            # the rustup toolchain: Homebrew's rustc has no iOS std
rustup target add aarch64-apple-ios >/dev/null

for t in aarch64-apple-ios aarch64-apple-darwin; do
  IPHONEOS_DEPLOYMENT_TARGET=18.0 cargo build --release --locked -p scc-lib --features ios --target $t
done

rm -rf dist; mkdir -p dist/ios dist/macos dist/include
cp scc/lib/include/scc_ios.h dist/include/
for p in ios:aarch64-apple-ios macos:aarch64-apple-darwin; do
  d=dist/${p%%:*}; t=target/${p##*:}/release
  cp $t/libscc_lib.a $t/libscc_lib.dylib $d/
  install_name_tool -id @rpath/libscc_lib.dylib $d/libscc_lib.dylib 2>/dev/null
  codesign -f -s - $d/libscc_lib.dylib 2>/dev/null
done

cc -O2 -Wall -Wl,-dead_strip -o dist/compile_test_mac test/compile_test.c dist/macos/libscc_lib.a -framework CoreFoundation
xcrun --sdk iphoneos clang -target arm64-apple-ios18.0 -isysroot "$(xcrun --sdk iphoneos --show-sdk-path)" -O2 -Wall -Wl,-dead_strip -o dist/compile_test_ios \
  test/compile_test.c dist/ios/libscc_lib.a -framework CoreFoundation
echo "iOS link: ok ($(nm -u dist/compile_test_ios | wc -l | tr -d ' ') imports, all libSystem/CoreFoundation)"
ls -l dist/ios dist/macos

if [ "${1:-}" = "--test" ]; then
  W="$(mktemp -d)/Documents/Cyberpunk 2077"
  test/make_workdir.sh "$W"
  /usr/bin/time -l dist/compile_test_mac "$W/r6" 2>&1 | grep -E "scc:|check|output|RESULT| real|peak memory"
  [ -f "$HOME/Downloads/CyPort-scripts/out/final.redscripts" ] &&
    python3 -I test/compare_blobs.py "$W/r6/cache/final.redscripts" "$HOME/Downloads/CyPort-scripts/out/final.redscripts"
  rm -rf "${W%/Documents/*}"
fi
