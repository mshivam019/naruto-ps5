#!/usr/bin/env bash
# perf profile of one harness thread during the village benchmark view.
# Usage: profile.sh TAG THREAD_COMM_PREFIX [SECONDS]   e.g. profile.sh p1 "GPU Commands" 30
set -u
here=$(cd -- "$(dirname -- "$0")" && pwd)
root=$(cd -- "$here/../.." && pwd)
build=${NARUTO_HARNESS_BUILD:-$root/build/host-harness}
bin="$build/${HARNESS_OUT:-out}"
tag=$1; comm=$2; length=${3:-30}
export NARUTO_HARNESS_MASH=60 NARUTO_HARNESS_WALK=0
"$here/run.sh" "$tag" $((110 + length)) > /dev/null &
runner=$!
sleep 100
pid=$(pgrep -n -f "$bin/narutorise")
tid=$(grep -l "^$comm" /proc/$pid/task/*/comm | head -1 | cut -d/ -f5)
perf record -F 4999 -g -t "$tid" -o "$build/run/$tag/perf.data" -- sleep "$length" > /dev/null 2>&1
wait $runner
echo "perf data: $build/run/$tag/perf.data"
