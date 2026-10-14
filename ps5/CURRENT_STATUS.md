# Naruto PS5 console status — October 14, 2026

Title: Naruto: Rise of a Ninja / PPSA99680. Repository remains private.
Installed at `/mnt/ext1/etaHEN/games/PPSA99680` on firmware 9.00.
User authorizes closing/restarting Naruto during debugging. The current session
is running for manual testing; preserve it while the user plays.

## Current result

User confirms a visible picture, normal speed, working Start, gameplay, movement
and audible audio after fixes. This is a user-confirmed playable console build.
Specific combat/camera checks and clean exit remain unverified. Save creation and PlayerSave1 reload are now user-confirmed after the
case-insensitive content lookup fix. Existing Autosave and KonohaAutosave both
mount and read successfully in logs. A fresh autosave/reload cycle is unverified.
Do not call this a fully validated release or claim a frame rate.

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

User subsequently reports smooth rendering but sluggish movement. Cause is not
yet established; the earlier normal-speed confirmation covered startup playback.
Source inspection shows full-range stick conversion; live stick magnitude and
gameplay frame pacing have not been measured. Sprint is R2 and story-unlocked.
No timing or live configuration changes were made in response to this report.

Shader disk storage previously stalled. After preparing cache/shaders/shareable
before launch, store_shaders=true starts and writes cache files. A subsequent
restart preloaded 45/45 cached graphics pipelines. This proves persistence, not
elimination of all stutters. New local startup prepares these directories.
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

## Current diagnostic session

User clarified speed is fine; intermittent choppiness remains. Save validation
takes priority before another gameplay comparison. Original manual save and
autosaves are backed up privately in evidence/save-backup-before-performance.
Manual Save.dat is 262144 bytes with nonempty UBISG data, SHA256
8462415f0be836a59342a76c4e0c31f4b41578d062606a816baafb3ff0d2e22f.
No save bytes changed by the agent.

Installed diagnostic executable (supersedes hash above):
ce709c600c525834411591d4cff3395c94681a9adcc922473b5f620679dff6b6.
Raw readback verified, rollback eboot.bin.before-save-case. It logs Save.dat
read offsets, lengths and FNV-1a byte hashes at info level; full trace disabled.
Console config has store_shaders=true. Latest diagnostic proved lowercase requested content folders do not match the
stored mixed-case names. XamContentCreate returned path-not-found (3), before
any save bytes were read. Installed fix resolves the existing content folder
and header by ASCII case-insensitive leaf matching. Save data is untouched.
Host mixed-case open/header/missing-path tests pass under ASan/UBSan. Runtime
patch exported and reverse-apply verified. User confirms PlayerSave1 restores progress. Full reads return success for all
three saved packages. Choppiness comparison remains pending.

## Frame timing and cache append follow-up

User still reports intermittent choppiness; do not mark performance fixed.
Timing candidate measured roughly 24–30 swap requests/sec in sampled gameplay
windows, some shader-skipped frames and a maximum observed gap of 1049 ms.
These are swap-request statistics, not measured screen refresh/FPS.
The pipeline description cache stayed at 2982 bytes / 45 pipelines despite new
compilation. Shader bytecode storage grew. New candidate explicitly seeks both
update streams to end before writing and avoids sleeping with flush work pending.

Currently installed SHA256 (supersedes earlier hashes):
05b7c0d7e36e7244dcfa4e6a1c14c16263bb138abc7d13c7d2fd6bd1dc4ef69b.
Raw hash verified, rollback eboot.bin.before-cache-append. Saves untouched.
Cache growth verified: pipeline cache grew from 2982 to 23574 bytes during
gameplay. User repeated-route comparison remains pending; no all-stutters-fixed
claim. New timing and cache append code is exported in the runtime patch. Evidence:
evidence/frame-timing-*, timing-before-cache-fix-*, cache-append-*.
