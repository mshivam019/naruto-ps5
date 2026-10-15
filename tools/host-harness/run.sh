#!/usr/bin/env bash
# Runs the Linux harness inside a headless nested KWin (nothing on the desktop).
# Usage: run.sh TAG SECONDS [extra runtime args]
# Env: NARUTO_HARNESS_BUILD (default build/host-harness), HARNESS_OUT (out),
#      NARUTO_HARNESS_MASH/_WALK/_SHOT_INTERVAL (see harness_probe.cpp).
set -u
root=$(cd -- "$(dirname -- "$0")/../.." && pwd)
build=${NARUTO_HARNESS_BUILD:-$root/build/host-harness}
bin="$build/${HARNESS_OUT:-out}"
tag=$1; seconds=$2; shift 2
out="$build/run/$tag"; mkdir -p "$out" "$build/run/user"
game=$(realpath "$root/private/game_root")
inner="$out/inner.sh"
{
  echo '#!/usr/bin/env bash'
  echo "export LD_LIBRARY_PATH='$bin' NARUTO_HARNESS_SCRIPT=1 NARUTO_HARNESS_SHOTS='$out' SDL_VIDEODRIVER=wayland"
  echo "export NARUTO_HARNESS_MASH=${NARUTO_HARNESS_MASH:-150} NARUTO_HARNESS_WALK=${NARUTO_HARNESS_WALK:-600}"
  echo "export NARUTO_HARNESS_SHOT_INTERVAL=${NARUTO_HARNESS_SHOT_INTERVAL:-30}"
  printf 'exec %q --game_data_root=%q --user_data_root=%q --log_file=%q' \
    "$bin/narutorise" "$game" "$build/run/user" "$out/runtime.log"
  printf ' %q' "$@"
  echo
} > "$inner"
chmod +x "$inner"
marker=$(mktemp)
timeout --kill-after=10 "$seconds" kwin_wayland --virtual --no-lockscreen \
  --socket "naruto-harness-$tag" --width 1280 --height 720 -- "$inner" > "$out/stdout.log" 2>&1
status=$?
# A killed runtime leaves its guest memory file behind; it fills /dev/shm and
# later runs fail with SIGBUS. Remove the files this run created.
find /dev/shm -maxdepth 1 -name 'xenia_memory_*' -user "$(id -u)" -newer "$marker" -delete
rm -f "$marker"
echo "exit=$status"
