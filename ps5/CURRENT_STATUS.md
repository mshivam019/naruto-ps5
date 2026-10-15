# Naruto PS5 — current status

Updated October 15, 2026. `main` keeps the original frame rates (open world
30 FPS, battles and menus 60); the 60 FPS unlock lives on branch
`experimental-60fps` (work in progress). The app is now runtime-only; the game
code is a module that `ps5/tools/setup.py` builds from the player's disc.
Next step: publish v0.1.0 (private release first), see `ps5/RELEASING.md`.

## Current state and user instructions

- **Playable.** User confirmed picture, normal game speed, controls, audio,
  rumble in battles, save creation and reload, and a steady 30 FPS in the
  village (`evidence/steady30-user-runtime.log`: 30.0 presents/s, max gap 35 ms).
- Title **Naruto: Rise of a Ninja / PPSA99680**, firmware 9.00.
  Installed at `/mnt/ext1/etaHEN/games/PPSA99680`.
- Installed build: PlayStation prompts and save pictures (SHA-256
  `af9ef5e1f1fd7c25fd58e5c8090b140a9c5a2a92048cbefbce8c76bf40eeac6d`, backup
  `eboot.bin.before-savepic`). Closing/restarting Naruto for testing is
  authorized. Never close unrelated titles.
- Repository `mshivam019/naruto-ps5` remains private. No release published.
  Generated game code, executables, disc assets and raw evidence must not be published.

## Release structure (app + setup-built module)

- **App** (`eboot.bin`, 33 MB): runtime only. At startup `ps5/module/` maps
  `/app0/game/naruto.module` (ELF shared object), applies RELATIVE/64/GLOB_DAT/
  JUMP_SLOT relocations, binds 332 imports by name from a table generated from
  `ps5/module/host-imports.txt`, registers unwind info, runs `.ctors`, and
  checks `naruto_module_abi` (SHA-256 of the import list, compile flags, SDK
  patch and module preparation script). Settings ship as `/app0/narutorise.toml`.
- **Module** (59 MB): generated default.xex and ai2c.dll code plus the loader
  thread hook, compiled with the pinned llvm-mingw (clang 23) for
  `x86_64-sie-ps5` against payload SDK v0.42 headers, hidden visibility except
  four exports; direct calls between game functions.
- **Setup** (`setup.sh`): Linux/WSL2. Clean run on this machine with cached
  sources: 2.6 minutes. The setup-built module booted on console (main module
  and ai2c.dll registered, 30 FPS); the user played the app/module build and
  reported it feels fine.

## PlayStation prompts, rumble and save pictures

- **Prompts** (`ps5/app/button_prompts.cpp`): every Xbox prompt is its own
  small DXT5 texture. The texture cache hook `TextureCache::SetGuestTextureLoadHook`
  runs before a texture is uploaded; textures whose untiled contents hash to a
  known prompt are redrawn in guest memory (BC3) as Cross, Circle, Square,
  Triangle, L1 or L3. 17 variants are listed, found with
  `naruto_texture_dump_dir` on the harness and in a console session that
  visited the village, menus, map and a battle (583 distinct textures). RB,
  triggers, Start and Back prompts did not appear in that session.
- **Rumble** works (user felt it in battles); the game sends vibration only in
  battles, otherwise zero.
- **Save pictures** (`ps5/app/save_picture.cpp`): when saving, the game resolves
  a 320x180 RGB565 picture and reads it on the CPU into `Save.dat`; Load Game
  shows it. With resolve readback off, saves stored a blank picture (black
  box). `readback_resolve = "fast"` (the PC port's `d3d12_readback_resolve`)
  fixed it on the harness but dropped the console to about 2 FPS (0.9 s per
  frame). The SDK now has `CommandProcessor::SetResolveReadbackFilter`; only the
  320x180 RGB565 resolve is read back, synchronously, once per save. Harness:
  the new save shows its picture. Older saves stay black until saved again.

## 60 FPS work (branch `experimental-60fps`)

- **Unlock** (`ps5/app/frame_rate.cpp` on the branch, cvar `naruto_fps60`). The
  game picks its rate through engine command dispatcher `sub_820EACE0`:
  command 45 requests and 68 locks a rate; 60 selects PresentInterval 1 and sets
  the engine 60 FPS flag at `0x82C0F050`, anything else PresentInterval 2.
  Battles request 60, the open world 30. The hook turns 30 requests into 60.
- **Game speed.** The open world also calls timer `SetFixedStep`
  (`sub_825B6148`) with a 1/30 s step, which would double game speed at 60 FPS.
  The hook keeps the timer in its measured mode with a 1/30 s maximum step:
  correct speed between 30 and 60 FPS, original slowdown below 30. Host harness
  confirmed engine time per second matches wall time in the village.
- **Command-processor optimizations** (SDK, exported in
  `ps5/patches/rexglue-ps5.patch`): bulk type-0 register ranges split by
  register region; register-table lookup only in debug builds; run-length float
  constant uploads; memexport readback check before its cvar lookup; persistent
  request-range buffer; single-range request cache keyed by a shared-memory
  validity epoch (cvar `shared_memory_request_cache`); memoized render-target
  ownership for repeated draws (cvar `render_target_update_memo`); early exit
  when no render-target transfers are needed.
- **PS5 page protection** (`src/core/memory_ps5.cpp`): pages already at the
  requested access are skipped and consecutive backed pages change with one
  `sceKernelMprotect` call instead of one per 16 KB page. Memory host test passes
  (`ps5/tools/test-memory.sh`; rollback case now fails the single run call).
- **16 KB host-page write watches** (`src/system/xmemory.cpp`,
  `PhysicalHeap::IsSystemPageGuestWritable`). PS5 host pages are 16 KB while
  guest pages are 4 KB. `EnableAccessCallbacks` and `TriggerCallbacks` checked
  only the first guest page of each host page, so a host page whose first 4 KB
  was read-only or uncommitted was never write-protected and guest writes to
  the rest went unnoticed. They now consider every committed guest page in the
  host page. This caused the stale/flickering geometry seen with
  `clear_memory_page_state=false` (user recording
  `evidence/recordings/20261014_210338.webm`).
- **`clear_memory_page_state = false`** is the packaged default again
  (`ps5/config/narutorise.toml`, also written to the console). User confirmed
  correct graphics with v6. Setting it true restores the old every-frame
  re-upload if anything goes stale.
- **Missed-write verifier** (cvar `shared_memory_verify_valid_pages`, off by
  default; debug only, costs most of the frame rate). With the fix it still
  reported a few pages (`0x14630000`, `0x13174000`, `0x132B0000`) changing every
  frame, probably host-side writes through the physical view (CP/XMA), with no
  visible effect (`evidence/fps60-v6-startup-runtime.log`).
- **Async Vulkan submission** (cvar `vulkan_async_submission`, default on):
  non-swap submissions are replayed and submitted on a worker thread; swaps and
  blocking fence waits drain it first. Linux harness A/B: 35.0/35.4 vs
  27.8/33.7 requests/s with it off.
- **Stage timing:** PS5 logs now also report primitives, vertex buffers,
  viewport, system constants and command-processor stall time per 5 seconds.
  Sampled stage values look inflated on PS5 (viewport and system constants both
  about 600 ms per 5 s); compare them relatively, not as absolute costs.

First 60 FPS build console result (`evidence/fps60-gameplay-runtime.log`): menus
59.6–60.4 requests/s; village 26.6–42.8/s with the command processor saturated
(about 6,700 draws per frame). A later idle spot ran at about 18/s with 8,700
draws per frame (`evidence/fps60-session2-runtime.log`). v4 at the user's
recorded spot: 19–24/s, user reports "not very smooth"; largest sampled stages
were vertex buffers, textures and primitives, all shared-memory/watch work
(`evidence/fps60-v4-user-runtime.log`).

v5 (coalesced page protection) at the same area, user-reported "much better":
menus 60/s; village 24.4–49.4 requests/s over the last 10 intervals, with 0–2
gaps over 50 ms per 5 s (v4: 10–48 per 5 s). About 7,500 draws per frame; the
command processor is still saturated, so 60 is not reached in the village
(`evidence/fps60-v5-user-runtime.log`).

v6 (16 KB watch fix, no per-frame page reset, async submission), user: graphics
correct, "this one feels better": village 29.8–50.0 requests/s with **no gaps
over 50 ms** in 13 consecutive 5 s intervals (`evidence/fps60-v6-user-runtime.log`).
The command processor is still the limit; 60 is not reached in the village.

The Linux harness (`tools/host-harness`) reproduces the village on the laptop's
RADV: 32.1 → 45.2 requests/s on a fixed idle view after the first passes.

## Installed candidate

**v6: 60 FPS, loader-thread fix, 16 KB write-watch fix, coalesced page
protection, async submission**, 89,887,133 bytes:

```text
de0d64d02800a0214b3587ebd488717326073f46375625f95ce5dd1ee466a6be
```

Deployment raw readback matched (`evidence/fps60-v6-deployed.json`). Console
config `user/narutorise.toml` equals `ps5/config/narutorise.toml`; its previous
copy is `user/narutorise.toml.before-v6`. Each deployment kept the previous
executable as `eboot.bin.before-fps60-v6` (v5), `-v5`, `-v4`, `-v3`, `-v2e`,
`-v2d`, `-v2` and `eboot.bin.before-fps60` (targeted texture invalidation).
`dist/PPSA99680` was not repackaged. Saves were not touched.

**Startup crash fixed** (`ps5/app/loader_thread.cpp`). About 18 seconds after
boot, when `ai2c.dll` loads, the game crashed with `Call to invalid or
unregistered function` (`0xE464C0C8`; older builds `0xD79CE9B4`). Diagnostics
traced it to the engine's `cLoader` thread: `sub_8265BDE0` starts the thread
before storing its entry function at +236, and the body `sub_8265BB68` calls
+236 at once. The body now waits for its creator to return. Before the fix 4 of
5 launches without input crashed; after it, 3 of 3 reached `ai2c.dll` and kept
running (`evidence/fps60-v3-{1,2,3}-runtime.log`).

## Latest measured performance

Source: console `narutorise_047.log`, copied to
`evidence/texture-invalidation-gameplay-runtime.log`. Parsed results and process
check: `evidence/texture-invalidation-gameplay-result.json`.

| Measurement | Latest 12 intervals | Longer 17-interval window |
| --- | ---: | ---: |
| Duration | 60.26 seconds | 85.31 seconds |
| Mean frame-request rate | 28.58/sec | 27.85/sec |
| Range of approximately 5-second rates | 25.65–30.02/sec | 23.35–30.02/sec |
| Gaps above50ms | 0 | 1 |
| Longest request gap | 48.1ms | 62.1ms |
| Shader-skipped frames | 0 | 0 |

The longer window ends17:45:54 and begins with the interval ending17:44:33.
An earlier interval ending17:44:28 included a703.6ms gap; the table does not
include startup/earlier intervals and must not be represented as the entire run.

**These are guest swap-request timings, not measured display FPS.** Routes and
camera positions were not controlled. No locked30/60FPS or complete stutter fix
is established. User has not yet explicitly confirmed the latest candidate's
smoothness or visual correctness. Prior dynamic-uniform build was reported
smoother but not polished; descriptor-fast alone was reported worse.

Latest minute's invalidation counters:37,240 notifications,17,443 bindings reset,
**219,170 unchanged bindings retained**. This confirms avoided reset work; it does
not independently establish a frame-rate speedup. CPU draw preparation remains
expensive. Sampled CPU stage times overlap and must not be added as independent
frame-time components. Current logging separates EndSubmission calls from actual
queue submissions; observed actual submissions track swap requests one-to-one.

## Regression finding and current fix

The preceding descriptor-fast run dropped to roughly18–27 requests/sec in busy
intervals, with gaps up to97.5ms. Pipeline creation waits and GPU fence waits were
small, while CPU draw preparation and texture handling were expensive. This
supports a CPU-side bottleneck; it does not prove that all regression was caused
by one code change or rule out every presentation/GPU issue.

A watched texture change previously called ResetTextureBindings for **all** bound
textures. The current code checks existing per-texture atomic dirty masks and
resets only changed unsigned/signed bindings. Unchanged textures keep their
keys/views, making reuse useful. Concurrent writes still publish notifications
for subsequent draws. Clean draws avoid an unnecessary atomic exchange.

Retained optimizations:

- Changed-value fetch-register updates and base texture preparation fast path.
- Sampler parameter memoization and same-submission sampler handle reuse.
- Texture descriptor reuse and compatible pipeline-layout binding retention.
- Dynamic uniform descriptors: original buffer/range plus aligned bind-time
  offsets; offset-only changes avoid descriptor allocation/writes.
- Descriptor preparation fast path guarded by shader, binding/usage epoch,
  layout/count and exact sampler handles. Full comparison remains the fallback.
- Primary-buffer batching; required barriers, upload flushes, readbacks and
  presentation synchronization retained. Game clock/simulation speed unchanged.
- Persistent shader/pipeline storage append/flush fixes and ordered fence checks.

## Confirmed functionality and remaining checks

- Startup fixes cover nonoverlapping ELF segments, sandbox-safe paths, thread
  resume,1216-byte native fiber context writes into2048-byte storage, correct
  nanosecond clock frequency and FFmpeg FFT tables outside execute-only text.
  Initial console shutdown cause was not proven; later builds remained running.
- PlayerSave1 reload is user-confirmed after case-insensitive content folder and
  header lookup. Existing Autosave/KonohaAutosave mount/read successfully in logs;
  a newly created autosave followed by reload still needs explicit verification.
- Xbox prompts remain: Cross=A, Circle=B, Square=X, Triangle=Y, R2=RT,
  Options=Start, touchpad=Back. Full physical mapping/prompt polish is incomplete.
- Native RADV/direct display, SDL3 controller/audio, static AI2C and custom XMA
  FFmpeg are in use. Guest presentation logs report1280x720; output swapchain
  has reported3840x2160. Do not equate either with measured rendering performance.
- Combat/camera coverage, clean exit, long sessions and final visual regression
  checks remain incomplete. Existing BaseHeap::Release warnings are unresolved;
  no evidence establishes them as the performance cause.

## Source, build and verification

- Project: the 60 FPS work and the earlier performance changes are committed
  on `main` and pushed to the private GitHub repository.
- Game pin:`226af94268de3660c3de24061f9a6bf7ae982d3c`.
- SDK pin:`c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`.
- Ignored nested SDK edits are exported into `ps5/patches/rexglue-ps5.patch`
  (checked against a fresh pinned SDK plus the six upstream patches); preserve
  both. Do not stage the unrelated `archive-inspection.json` or private generated
  artifacts.

```sh
NARUTO_BUILD_JOBS=4 bash ps5/tools/build-runtime.sh
python3 ps5/tools/build-title.py --link-only
```

Latest compilation/link/import and nonoverlapping ELF segment validation passed;
patch applicability against the fresh pinned baseline passed. Existing sanitizer
checks passed for fetch-register updates, descriptor snapshots, dynamic uniform
addresses/ranges and save-path case handling. The latest selective invalidation
change has native-build and console runtime evidence; its visual correctness and
race behavior are not exhaustively tested.

Configuration, matching current live batching choice:

```toml
store_shaders = true
log_level = "info"
log_noisy = false
log_flush_interval = 1
vulkan_submit_on_primary_buffer_end = false
```

Useful tools (private evidence scripts): `fetch-runtime.py TAG`,
`close-naruto.py`, `deploy-texture-invalidation.py`, `boot-with-logs.py TAG`.
Do not rerun a deployment script with an existing backup suffix. Console control
is FTX2 at192.168.50.6:9114, not HTTP; FTP is2121. Recheck service/title state
before changes. Preserve networking, payload stack, shared SDK/driver and siblings.

## Next work

1. Get user confirmation of current smoothness and graphics on the busy route.
2. Measure the remaining CPU spikes with comparable scene/camera conditions;
   distinguish guest request pacing from actual presentation cadence.
3. Prioritize proven hot paths; retain normal clock speed and required resource
   synchronization. Do not promise locked FPS or deploy speculative changes as fixes.
4. Preserve saves and honor the explicit no-rollback instruction. Finish gameplay,
   autosave/reload, exit and visual checks before considering a release.

Detailed historical notes remain in PLAN.md. The previous append-only status file
is preserved privately at `evidence/current-status-history-before-20261014-1746.md`.
