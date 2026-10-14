# Linux profiling harness

Builds the PS5 port's patched ReXGlue SDK, the private generated game code and
the PS5 app's guest hooks (`frame_rate.cpp`, `loader_thread.cpp`) for a Linux
desktop with Vulkan. It reproduces the PS5 command-processor workload on a
desktop RADV GPU so changes can be profiled and checked before a console test.
Builds contain recompiled game code: never publish them.

Requirements: generated code in `private/codegen/generated`, the game in
`private/game_root`, clang, lld, ninja, `kwin_wayland` (headless nested
compositor) and optionally `perf`.

```sh
cmake -S tools/host-harness -B build/host-harness -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_LINKER_TYPE=LLD
cmake --build build/host-harness
tools/host-harness/bench.sh base          # fps of the fixed village view
tools/host-harness/profile.sh p1 "GPU Commands" 30
```

`run.sh TAG SECONDS` starts the game in a nested headless KWin, so nothing
appears on the desktop. `harness_probe.cpp` attaches a virtual SDL gamepad that
taps A (and Start early on) through the menus, then stands still or walks, logs
guest presents per second together with the engine timer's delta every five
seconds, and writes the guest output (not the desktop) to
`build/host-harness/run/TAG/shot-N.ppm`. The PS5 stage timings are also logged
because the build defines `REX_GPU_STAGE_STATS_HOST`.

Mind the platform difference: PS5 uses 16 KB pages and direct-memory aliases,
so results for shared-memory invalidation must still be confirmed on console.
