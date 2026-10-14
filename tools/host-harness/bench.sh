#!/usr/bin/env bash
# Fixed benchmark: 60 s of menu taps into the Leaf Village, then standing still.
# Prints the mean guest present rate of the last minute and the busiest threads.
# Usage: bench.sh TAG [extra runtime args]
set -u
here=$(cd -- "$(dirname -- "$0")" && pwd)
root=$(cd -- "$here/../.." && pwd)
build=${NARUTO_HARNESS_BUILD:-$root/build/host-harness}
bin="$build/${HARNESS_OUT:-out}"
tag=$1; shift
export NARUTO_HARNESS_MASH=60 NARUTO_HARNESS_WALK=0 NARUTO_HARNESS_SHOT_INTERVAL=60
"$here/run.sh" "$tag" 170 "$@" > /dev/null &
runner=$!
sleep 100
pid=$(pgrep -n -f "$bin/narutorise")
cpu() { for t in /proc/$pid/task/*; do echo "$(basename $t) $(tr ' ' _ < $t/comm) $(sed 's/.*) //' $t/stat | awk '{print $12+$13}')"; done; }
before=$(mktemp); after=$(mktemp)
cpu > "$before"; sleep 60; cpu > "$after"
wait $runner
log="$build/run/$tag/runtime.log"
grep "harness\] presents" "$log" | tail -12 | sed -E 's/.*wall_fps=([0-9.]+).*/\1/' |
  awk '{s+=$1; n++; if(min==""||$1<min)min=$1; if($1>max)max=$1} END {printf "fps mean=%.2f min=%.2f max=%.2f\n", s/n, min, max}'
join <(sort "$before") <(sort "$after") | awk '{printf "%s %.0f%%\n", $2, ($5-$3)/60}' | sort -k2 -n -r | head -5
rm -f "$before" "$after"
