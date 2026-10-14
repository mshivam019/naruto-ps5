# Naruto PS5 — current status

Updated October 15, 2026, after the v5 console test (user: "much better now").
This document supersedes older candidate/status statements in PLAN.md.

## Current state and user instructions

- **Playable, still under performance development.** User confirmed picture,
  normal game speed, Start, movement, audio, save creation and PlayerSave1 reload.
- Title **Naruto: Rise of a Ninja / PPSA99680**, firmware9.00.
  Installed at `/mnt/ext1/etaHEN/games/PPSA99680`.
- Latest live check: Naruto not running after the second-build startup check.
  Recheck IDs before acting.
- User explicitly said **do not roll back** after the descriptor-fast regression.
  No rollback occurred. The latest build is a forward fix retaining the previous
  optimizations. Closing/restarting Naruto for testing is authorized, but preserve
  the active session during read-only/status work. Never close unrelated titles.
- Repository `mshivam019/naruto-ps5` remains private. No release published.
  Generated game code, executables, disc assets and raw evidence must not be published.

## 60 FPS work

- **Unlock** (`ps5/app/frame_rate.cpp`, cvar `naruto_fps60`, default on). The
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
- **`clear_memory_page_state` stays true (SDK default).** Setting it false (the
  Xenia Canary default) produced missing/flickering geometry and sky streaks on
  PS5 (user recording `evidence/recordings/20261014_210338.webm`), although the
  Linux harness looked correct. Likely some guest writes reach shared memory
  without a write-watch fault on PS5. User confirmed the issues gone after
  restoring true. Do not disable it without fixing that first.
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
(`evidence/fps60-v5-user-runtime.log`). Next candidates: move deferred
command-buffer replay and queue submission (about 0.9 s per 5 s) to a worker
thread, and reduce shared-memory/texture watch work.

Private Linux harness (`.scratch/host-linux`, never published) reproduces the
village on the laptop's RADV: 32.1 → 45.2 requests/s on a fixed idle view after
these changes.

## Installed candidate

**v5: 60 FPS, loader-thread fix, page-state clearing restored, coalesced page
protection**, 89,855,053 bytes:

```text
858b78d01360828bfbf593bd5397e7850debd10a0fe1e41508826afb183d7f1d
```

Deployment raw readback matched (`evidence/fps60-v5-deployed.json`). Each
deployment kept the previous executable as `eboot.bin.before-fps60-v5` (v4),
`-v4` (v3 loader fix), `-v3`, `-v2e`, `-v2d` (diagnostic builds), `-v2` (first
60 FPS build) and `eboot.bin.before-fps60` (targeted texture invalidation).
`dist/PPSA99680` was not repackaged. No saves/configuration were replaced.

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

- Project HEAD:`9ef3cd1ba86b9247775aad3cf7b1d0fdd764aeb0`.
- Game pin:`226af94268de3660c3de24061f9a6bf7ae982d3c`.
- SDK pin:`c94f5ebdcb3c9d1a460ca48e04f9758448f8d518`.
- Performance changes are local/uncommitted. Ignored nested SDK edits are exported
  into `ps5/patches/rexglue-ps5.patch`; preserve both. Do not stage the unrelated
  `archive-inspection.json` or private generated artifacts.

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
