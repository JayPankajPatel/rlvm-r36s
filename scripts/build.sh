#!/bin/sh
# Runs inside the container. $1 = rlvm source dir, $2 = output dir.
set -e
SRC=$1; OUT=$2; HERE=$(dirname "$0")
cd "$SRC"
scons -j"$(nproc)" --release build/release/rlvm || true   # msgfmt/translations may fail; binary is what we need
test -x build/release/rlvm
mkdir -p "$OUT/libs"
cp build/release/rlvm "$OUT/"
# Bundle only the shared libs the card does not already have.
ldd build/release/rlvm | awk '/=>/ {print $3}' | while read -r lib; do
  [ -f "$lib" ] || continue
  b=$(basename "$lib")
  grep -qx "$b" "$HERE/card-libs.txt" || cp -L "$lib" "$OUT/libs/"
done
ldd build/release/rlvm > "$OUT/ldd.txt"
ls -la "$OUT" "$OUT/libs"
