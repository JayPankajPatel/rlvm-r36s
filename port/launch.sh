#!/bin/bash
# CLANNAD (Steam HD) via rlvm on ArkOS handhelds. Usage: launch.sh mouse
# Logs: clannad/run.log (summary), clannad/run-game.log (rlvm/SDL/gl4es output)
MODE=${1:-mouse}
if [ -d "/opt/system/Tools/PortMaster/" ]; then controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/PortMaster/" ]; then controlfolder="/PortMaster"
else controlfolder="/roms/ports/PortMaster"; fi
source $controlfolder/control.txt 2>/dev/null
GAMEDIR=/roms/ports/clannad
[ -d "$GAMEDIR" ] || GAMEDIR=/roms2/ports/clannad
cd "$GAMEDIR" || exit 1
[ -f run.log ] && mv -f run.log run-prev.log
exec > "$GAMEDIR/run.log" 2>&1
date; echo "mode: $MODE"; uname -a; head -2 /proc/meminfo
export LD_LIBRARY_PATH="$GAMEDIR/libs:$LD_LIBRARY_PATH"
export HOME="$GAMEDIR/home"; mkdir -p "$HOME"
export SDL_AUDIODRIVER=alsa SDL_VIDEODRIVER=KMSDRM
export SDL12COMPAT_DEBUG_LOGGING=1
export LIBGL_ES=2 LIBGL_GL=21 LIBGL_FB=4 LIBGL_DEBUG=1

# Real panel size: the game draws 1280x960 offscreen and scales to this.
OUT=""
for c in /sys/class/drm/card0-*; do
  if [ "$(cat "$c/status" 2>/dev/null)" = "connected" ]; then OUT=$(head -1 "$c/modes" 2>/dev/null); break; fi
done
[ -z "$OUT" ] && OUT=$(tr ',' 'x' < /sys/class/graphics/fb0/virtual_size 2>/dev/null)
OUT=${OUT%%[!0-9x]*}
echo "panel size: '$OUT'"
[ -n "$OUT" ] && export RLVM_OUTPUT_SIZE="$OUT"

GPTK="$GAMEDIR/clannad-$MODE.gptk"
if [ -n "$GPTOKEYB" ]; then
  $GPTOKEYB "rlvm" -c "$GPTK" &
elif [ -x /opt/inttools/gptokeyb ]; then
  /opt/inttools/gptokeyb -1 "rlvm" -c "$GPTK" &
fi
t0=$(date +%s)
./rlvm --font "$GAMEDIR/game/dat/migmix-1m-regular.ttf" "$GAMEDIR/game" > "$GAMEDIR/run-game.log" 2>&1
echo "rlvm ran $(( $(date +%s) - t0 ))s"
pkill -f gptokeyb 2>/dev/null
