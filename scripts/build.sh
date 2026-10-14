#!/bin/sh
# Runs inside the container. $1 = rlvm source dir, $2 = output dir.
set -e
SRC=$1; OUT=$2; HERE=$(dirname "$0")
# Shims for the handheld: SDL 1.2 API over the card's SDL2 (KMS/DRM), and desktop GL over GLES.
git clone -q --depth 1 --branch release-1.2.68 https://github.com/libsdl-org/sdl12-compat.git /tmp/sdl12
cmake -S /tmp/sdl12 -B /tmp/sdl12/b -DCMAKE_BUILD_TYPE=Release -DSDL12TESTS=OFF -DSDL2COMPAT_STATIC=OFF >/dev/null
make -C /tmp/sdl12/b -j"$(nproc)" install >/dev/null
ldconfig
git clone -q --depth 1 https://github.com/ptitSeb/gl4es.git /tmp/gl4es
cmake -S /tmp/gl4es -B /tmp/gl4es/b -DCMAKE_BUILD_TYPE=Release -DNOX11=ON -DGBM=ON -DDEFAULT_ES=2 >/dev/null
make -C /tmp/gl4es/b -j"$(nproc)" GL >/dev/null
mkdir -p "$OUT/libs"
cp -L /tmp/gl4es/lib/libGL.so.1 "$OUT/libs/libGL.so.1"
cp -L /usr/local/lib/libSDL-1.2.so.0 "$OUT/libs/libSDL-1.2.so.0"
cd "$SRC"
scons -j"$(nproc)" --release build/release/rlvm || true   # msgfmt/translations may fail; binary is what we need
test -x build/release/rlvm
cp build/release/rlvm "$OUT/"
# Bundle only the shared libs the card does not already have.
ldd build/release/rlvm | awk '/=>/ {print $3}' | while read -r lib; do
  [ -f "$lib" ] || continue
  b=$(basename "$lib")
  case "$b" in libGL.so.1|libSDL-1.2.so.0) continue;; esac
  grep -qx "$b" "$HERE/card-libs.txt" || cp -L "$lib" "$OUT/libs/"
done
ldd build/release/rlvm > "$OUT/ldd.txt"
ls -la "$OUT" "$OUT/libs"
