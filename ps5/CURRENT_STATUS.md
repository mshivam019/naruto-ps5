# Naruto PS5 console status — October 14, 2026

Title: Naruto: Rise of a Ninja / PPSA99680. Repository remains private.
Installed at `/mnt/ext1/etaHEN/games/PPSA99680` on firmware 9.00.
User authorizes closing/restarting Naruto during debugging. The current session
is running for manual testing; preserve it while the user plays.

## Current result

User confirms a visible picture, normal speed and working Start after fixes.
Gameplay/movement, audible audio, combat/camera, save/reload and exit are pending.
Do not call this a finished playable release or claim a frame rate.

Current installed and staged executable SHA-256:
`5263190965368395193c6ecadefc68a0547d10d199c242dedcb6e62f70a276e9`.
Raw console readback matched. Private staging: `dist/PPSA99680`; file hashes in
`evidence/local-package.json`. It includes generated game code and owned disc
assets: never publish the executable or complete title. Source-only fixes may
be pushed to the private repository. No release has been published.

The runtime uses native RADV/direct display, SDL3 controller/audio integration,
a static AI2C module and a custom XMAFRAMES FFmpeg build. Guest presentation
logs report 1280x720. Controller mapping: Cross=A, Circle=B, Options=Start,
touchpad=Back. Gameplay mappings still need physical verification.

## Confirmed startup fixes

- The initial launch powered off/rebooted the console. Its converted ELF had
  overlapping LOAD ranges. Project-local layout explicitly places large-code
  and libc++ override sections; intermediate/converted ELF checks now reject
  overlaps. Subsequent runs stayed up. The initial shutdown cause is not proven.
- Sandbox-safe executable/user paths and absolute optional controller mapping
  path avoid unsupported chdir/passwd assumptions. Missing optional input files
  use nonthrowing filesystem checks. Unhandled faults no longer retry forever.
- PS5 thread Resume now follows existing Linux/macOS behavior when already
  running, preventing a guest loop on NtResumeThread failure.
- Native getcontext writes 1216 bytes (mc_len=1152); the SDK ucontext_t is only
  928 bytes. This overwrote neighboring allocations during fiber conversion.
  Each PS5 fiber now reserves 2048 bytes and seeds new stacks through the
  verified register prefix, avoiding truncated uc_stack fields. Three console
  fiber-switch round trips pass. Private probe: evidence/context-probe.log.
- Clock counts are nanoseconds, but the old frequency used 1e9/clock_resolution.
  Console resolution is 81 ns, causing an 81x guest-time error. PS5 frequency is
  now 1e9; a 100 ms wait measured 0.100110 s. User confirms normal speed.
- Start triggered SYSTEM_XO_VIOLATION in FFmpeg ff_imdct_half_avx: its FFT
  dispatch tables were in execute-only text. All four SSE/AVX tables now reside
  in read-only data with preserved text-relative function offsets. Final ELF
  checks pass; SIMD is retained. User confirms Start works without crashing.

## Remaining constraints

Shader disk storage initialization stalled during earlier testing. The packaged
PS5 defaults keep `store_shaders=false`; runtime shader compilation may pause.
Current console logging is info/noisy=false. Prior config and executable versions
are backed up on the console. Do not replace live files or overwrite saves.
Physical-memory free warnings remain in runtime logs; investigate if memory
usage grows or gameplay fails. No broad compatibility or performance claim.

## Evidence and reproduction

- Current confirmation: evidence/confirmed-startup.json, timing-audio-*,
  fft-table-validation.json, start-button-crash-*, context-overwrite.json.
- Older visible build retained as evidence/visible-fiber-fix-eboot.bin and
  matching symbol ELF. Current build backup: normal-speed-start-working-*.
- Per-candidate *-deployed.json receipts record raw hashes and rollback paths.
- Original 108-file deployment checked all stored sizes and raw hashes of the
  executable/libc/metadata. Current staging adds PS5 default configuration.
- Native compilation/signing/import checks pass. Host guest-memory alias,
  protection, rollback and teardown checks passed with UBSan.
- ReXGlue patch applies over the six pinned upstream patches. FFmpeg patch is
  applied idempotently by ps5/tools/build-ffmpeg.sh.
- Build: `NARUTO_BUILD_JOBS=4 bash ps5/tools/build-ffmpeg.sh`, then
  `python3 ps5/tools/build-title.py --jobs 4`. Source pins are in PLAN.md.

Sibling ports, the shared SDK, payload stack and original assets are preserved.
