# Naruto: Rise of a Ninja PS5 Vulkan handoff

Updated 2026-10-14. Workspace: `/home/skm/Downloads/ps5/Naruto-Rise-PS5`.

## Current status (supersedes historical entries below)

See [ps5/CURRENT_STATUS.md](ps5/CURRENT_STATUS.md) for the authoritative current
handoff, installed hash, permissions, measured performance and outstanding checks.
Latest installed build is the forward texture-invalidation fix, SHA-256
`f048ef3277322a835e8000f1f39e9618b36da6fe0e1acf13f20b280c08a480c9`.
Naruto remains running. No rollback occurred; user explicitly requested none.
Latest60.26sec averages28.58 guest frame requests/sec with no gaps>50ms; longer
85.31sec window still dips to23.35/sec and has a62.1ms gap. These are not displayed
FPS. Performance polish and latest user visual/smoothness confirmation remain open.
All entries below are historical and may describe superseded candidates.

## Objective and user decisions

Port Xbox 360 **Naruto: Rise of a Ninja** to a native PS5 title using the existing
ReXGlue Vulkan backend. The user chose Vulkan after the feasibility assessment,
said a Naruto ZIP exists in Downloads, then requested a new project folder with
all context and a plan another agent can pick up. The user subsequently requested starting implementation. Initial local preparation
is now underway; see the October 14 implementation entry below. No console deployment has happened.
Do not switch to the unrelated MUGEN game without discussing the mismatch.

Initial target: correct rendering, controls, audio and saves at a modest resolution,
then a playable scene. 4K, 60 FPS, DLC and polished release packaging come later.
There is no evidence yet that ReXGlue's Vulkan features work on PS5 Mesa.

## Input inspected and immediate blocker

`/home/skm/Downloads/Naruto Rise of a Ninja.zip` exists: 524,257,506 bytes,
2,083 entries, 813,299,735 uncompressed bytes. Its root is
`Naruto Rise of a Ninja/`. It contains `data/mugen.cfg`, character `.cns`, `.sff`,
`.air`, `.cmd` assets, Windows CRT files and `Naruto Rise of a Ninja.exe`.
The embedded configuration explicitly identifies M.U.G.E.N. There are no `.xex`
or `.iso` entries. This is a Naruto MUGEN fan-game package, not the required Xbox
360 dump. The original archive was left untouched and its EXE was not run.
`archive-inspection.json` records the evidence; full archive CRC is untested.

For the intended game, obtain the user's extracted Xbox 360 files including
`default.xex`, `ai2c.dll` and remaining disc content, or an Xbox 360 disc ISO.
Expected Xbox title ID: **555307E5**, unrelated to the eventual PS5 title ID.
Hash and inspect supplied files and confirm supported revision before codegen.
Runtime portability and Vulkan capability work can proceed without game data.

## Primary upstreams, inspected October 14

- Game: https://github.com/jorjbarbosa/naruto-rise-recomp
- Architecture: https://github.com/jorjbarbosa/naruto-rise-recomp/blob/main/docs/architecture.md
- Build guide: https://github.com/jorjbarbosa/naruto-rise-recomp/blob/main/docs/build-and-run.md
- Runtime: https://github.com/rexglue/rexglue-sdk
- Runtime configuration: https://github.com/rexglue/rexglue-sdk/blob/main/CMakeLists.txt
- Foundations: https://github.com/xenia-project/xenia and https://github.com/hedge-dev/XenonRecomp

Naruto translates PPC to C++ and uses ReXGlue for guest kernel, memory, Xenos
GPU, audio and input. Current verified Naruto distribution is Windows x64:
game defaults D3D12, launcher uses D3D11. Other presets are not proof of support.
ReXGlue main has Vulkan enabled by default on non-Windows builds, requires
Clang 18+ and C++23, and currently detects Windows/Linux/macOS only. A PS5 target
needs explicit platform integration; pretending PS5 is Linux is insufficient.
Inspect the game's pinned SDK revision before choosing newer runtime main.
No upstream clones or commits are pinned in this folder yet.

Game integration lives in `app/src/`, manifests/seeds in `app/`, SDK adaptations
in `patches/sdk/`, generated code in `app/generated/`. Patch application and
packaging are Windows-oriented. Preserve manifests; do not use `init --force`
which can overwrite title-specific configuration. Inspect actual manifests for
DLC requirements and make an explicit base-game-only configuration initially.
Address-based hooks/seeds need revision verification. Upstream architecture notes
a missing project root license: check current licensing before public redistribution.
Use dependency licenses and preserve notices; do not assume a license from public visibility.

## Existing working PS5 references

Read root `../AGENTS.md`; older October 5 prototype notes are historical.
Twilight Princess and Wind Waker now run on PS5. Morrowind used OpenMW and needed
native keyboard, UI/cursor and clean-exit work. That experience helps platform
integration but does not supply an Xbox 360 kernel/Xenos runtime.

Useful existing files, all relative to `/home/skm/Downloads/ps5`:

- `Wind-Waker-HD-PS5/ps5/main.cpp`: native startup, display/runtime and lifecycle.
- `Wind-Waker-HD-PS5/ps5/input.cpp`: native controller integration.
- `Wind-Waker-HD-PS5/ps5/native/` and `ps5/tools/`: platform/build/setup tooling.
- `Wind-Waker-HD-PS5/ps5/Platform.cmake`: pinned Vulkan headers, native RADV linking.
- `Wind-Waker-HD-PS5/ps5/dependencies.json`: tested SDL3, SDK and Vulkan pins.
- `Wind-Waker-HD-PS5/ps5/module/`: private native game-module loader precedent.
- `Wind-Waker-HD-PS5/ps5/RELEASING.md`, `CATALOG-SETUP.md`: ROM-free distribution.
- `Dusklight-PS5/ps5/BUILD.md`, `RENDERER.md`, `native/`, `tools/`: SDL/Vulkan work.
- `PS5_Vulkan/`, `PS5_Mesa/`, `PS5_VulkanTemplate/`: driver/template sources.
  Read their own AGENTS instructions before changes.
- `/home/skm/ps5sdk/`: installed compiler, dependencies and native-app tooling.

Wind Waker's recorded working dependencies include base SDK
`adc8dd795ada7bf696cd98748aae463a3970ffbb`, Mesa
`0b2d6d1a61d9bbf89cf8beb88a696144f67c61f8`, RADV SDK revision
`95c08f27386fc698f6bbe21dde3030140a41d10b`, and LLVM runtime 18.1.8.
These are reference pins, not a Naruto compatibility claim. Locate real SDK/library
paths from existing build scripts. Do not replace the working toolchain globally.

## Execution plan

1. Clone Naruto under `upstream/naruto-rise-recomp` and record commit, SDK gitlink,
   submodule pins and licenses. Keep PS5 support/scripts separate and adaptations
   reproducible. Audit game SDK patches against that pinned revision.
2. Audit ReXGlue platform services and build graph: guest address-space mapping,
   permissions/protection, threads/TLS, synchronization, timing, exceptions,
   filesystem, dynamic modules, input, audio and video decode dependencies.
   Create an explicit PS5 CMake/toolchain target. Build the host code generator
   separately from PS5 runtime libraries and disable optional profiling/FidelityFX
   initially. Keep real unsupported APIs visible as failures, not silent stubs.
3. Audit the **pinned** Vulkan backend's required API version, extensions,
   features, formats, descriptor model, subgroup requirements, memory semantics
   and shader translation dependencies. Compare against PS5 driver source and
   a real console feature report. Determine static RADV dispatch and presentation
   integration using native display/WSI code. Do not infer feature support from
   a successful Wind Waker render.
4. Cross-build the smallest runtime/graphics probe with actionable file logs.
   Verify memory mapping and a Vulkan clear/triangle/presentation on PS5 before
   integrating Xenos rendering. Separate host compile proof from console proof.
5. Once the correct game dump exists, validate revision/hashes, prepare base-game
   manifest and run host codegen. Cross-compile generated game plus adapted runtime.
   Avoid the Windows launcher initially; use a small native game entry point with
   explicit game/user/cache/log paths and a useful missing-game screen.
6. Integrate native controller and audio. Default Xbox mapping:
   Cross=A, Circle=B, Square=X, Triangle=Y; retain analog sticks/triggers and
   sensible Start/Back bindings. Validate every mapping in gameplay. Consider
   touchpad for settings after the game works. Save to writable sandbox/M.2 paths,
   without overwriting working sibling-title data.
7. Deploy an isolated title after selecting an unused PS5 title ID. Verify title
   screen, new game, movement, combat, camera, audio/cutscenes, save/reload and clean
   exit. Preserve progress; ask whether the user saved before closing active play.
   Capture errors/FPS/frametimes without permanent diagnostic popups. Start with
   the game's normal logic rate; do not equate display refresh with game FPS.
8. Once gameplay works, prepare build/validation scripts, concise README with
   controls/languages/game-file setup, attribution, and ROM-free release artifacts.
   Prefer prebuilt reusable runtime components and simple Windows/Linux/macOS
   setup like our other ports. Verify the actual supported hosts and disclose
   untested execution accurately. Consider a private generated module flow only
   after checking ReXGlue module/ABI requirements. No public repository or release
   creation is currently requested.

## Console and deployment context

Firmware 9.00. Last recorded LAN address `192.168.50.6`, laptop eno1
`192.168.50.1/24`, local-only; WLAN supplies laptop internet. Preserve this setup.
Last recorded endpoints: FTP anonymous 2121, PS5 Upload FTX2 9113/9114,
Payload Manager 8084, ELF loader 9021. Availability is unverified for Naruto;
recheck. Port 9021 was closed in the latest Wind Waker work. Do not load payloads
merely to test reachability or add a resident service without a demonstrated need.
Native games live `/mnt/ext1/etaHEN/games/<TITLE_ID>`; preserve M.2 preference.
Chosen stack is kstuff + ShadowMount, not etaHEN/Itemzflow. Preserve autoload.

Controller users: first DualSense User1, second DualSense User4, Redgear User3;
Chiaki uses renamed former User2. Keep assignments. Redgear direct receiver works
through its manually loaded Payload Manager bridge, including front USB-C OTG.

Useful control helpers: `Wind-Waker-HD-PS5/ps5/tools/console_control.py` and
`updates-20261004/console_ops.py`. Set PS5_HOST explicitly. FTP may expose decoded
ELF for SELF files; raw FTX2 reads are needed for byte-exact eboot verification.
Sandbox paths can require FTP instead of raw FTX2 reads and exist only while active.
Close a title before executable replacement and keep rollback copies/receipts.
Do not interrupt unrelated games or change established payload/network settings.

The preceding Wind Waker task is complete: stable ps5-v0.1.2, main `73007c5`,
catalog PR #269. Interpolation retained at user request, diagnostics off, game
closed. Remaining scene-dependent frame drops were not claimed fixed. There is
no outstanding Wind Waker work needed to start Naruto.

## Current evidence and next action

Completed: project folder, instructions, this plan and ZIP classification.
Not completed: cloning, PS5 runtime port, compilation, extraction/codegen,
Vulkan capability probe, console test or playable game. No console changes made.

Next agent should begin with steps 1–3 while requesting the correct Xbox 360
input if none is available. Record reproducible commands and blockers here rather
than restarting the feasibility discussion. Never treat the MUGEN ZIP as a dump.


## October 14 implementation started

The user requested starting the Naruto port. Work is isolated here; no console
connection, deployment, payload change or sibling-port modification occurred.

- New 6,704,818,097-byte 7z supplied in this folder contains a 7,835,492,352-byte
  Xbox 360 ISO. 7z extraction passed its integrity checks. The earlier MUGEN ZIP
  is unrelated and remains untouched.
- Built XboxDev/extract-xiso locally and extracted 103 files, 4,652,980,307 bytes,
  to `private/game_root`. Original archive/ISO preserved. Input hashes are in
  `evidence/input-sha256.txt`; extractor log in `evidence/disc-extraction.log`.
- `tools/validate-game.py` verifies XEX2 headers and execution title identity,
  hashes both modules and handles original `AI2C.dll` casing. Both modules report
  title 555307E5, media 12DF7A0D, version/base 00000002. Receipt:
  `evidence/game-identity.json`. Revision-sensitive seed compatibility remains
  unverified; checked-in seeds do not provide the source hashes they reference.
- Naruto source pinned at 226af94268de3660c3de24061f9a6bf7ae982d3c;
  SDK c94f5ebdcb3c9d1a460ca48e04f9758448f8d518. Full submodules initialized.
  Pins and six SDK patch digests: `evidence/source-pins.json` and
  `evidence/submodule-pins.txt`. No root Naruto license found.
- `tools/prepare-host.sh` checks the Naruto pin, initializes dependencies, applies
  upstream patches idempotently, configures Linux Clang with x86-64-v2 (as in
  upstream preset) and builds rexglue with four jobs. Tracy/FidelityFX/perf
  counters disabled. First attempt lacked the preset ISA flag and failed on
  SSSE3 intrinsics; corrected script rebuild is recorded in host-build.log.
- `python tools/prepare-codegen.py` validates inputs and creates the isolated
  `private/codegen/narutorise_manifest.toml`, retaining only the base AI2C module
  and its original seeds. Upstream DLC manifest remains untouched.
- Initial source-based platform review: `evidence/PS5-PORT-AUDIT.txt`. Primary
  memory concern is 0x120000000 virtual backing with aliased views, including a
  4 KiB-skewed physical mapping. This needs a PS5 page-granularity probe before
  broad runtime integration. Vulkan hard requirements identified; live PS5
  capability/format/presentation verification still pending.

Reproduce host preparation from this directory with `tools/prepare-host.sh`.
Then run `python tools/prepare-codegen.py` and
`upstream/naruto-rise-recomp/sdk/out/linux-amd64/rexglue codegen private/codegen/narutorise_manifest.toml`.
Generated code must stay private. No PS5 title ID or repository has been selected.


### Host/codegen milestone completed

`tools/prepare-host.sh` completed successfully; final log is
`evidence/host-build-final.log`. The host executable reports
ReXGlue v0.10.0.2-dev.gc94f5eb. Base-game codegen then exited 0, reporting
316 files written, including default.xex, AI2C.dll and module registry/CMake
outputs. Generated private tree is about 156 MiB. Do not publish it.

Codegen logged a cleanup diagnostic: `BaseHeap::Release failed because address
is not a region start`, address 82000000. Preserve and investigate this rather
than treating successful exit as a clean runtime check. Full log:
`evidence/codegen.log`; milestone receipt: `evidence/initial-preparation.json`.
This demonstrates successful host analysis/emission only. It does not establish
seed correctness in gameplay, PS5 compilation or Vulkan compatibility.

Next implementation milestone: explicit PS5 platform integration and an isolated
memory reservation/alias probe, then Vulkan capability/presentation integration.
No native Naruto executable or console deployment exists yet.

## October 14 texture-request optimization candidate

User reports descriptor-reuse build is better but still not smooth enough.
Added Vulkan texture-request fast path after the base guest-memory invalidation
check. Repeated identical masks skip Vulkan binding walks only while the binding/
usage epoch, submission and 3D-wrapper setting match. Binding resets/updates and
all image usage transitions invalidate it; submission changes retain usage tracking.
No simulation clock changes. Existing descriptor snapshot sanitizer test passes.
Native runtime build and executable import/segment validation pass. Candidate
SHA-256: `8f6ebd32dbb4f243a02044ffbc79af24d5237073e0a4d8be47e5a1636ce2812a`.
Build commands: `NARUTO_BUILD_JOBS=4 bash ps5/tools/build-runtime.sh`, then
`python3 ps5/tools/build-title.py --link-only`. Source pins unchanged.
Evidence: `evidence/texture-reuse-{runtime-build,link}.log`; deployment and
manual gameplay comparison pending at this entry. Saves/configuration untouched.

Texture-reuse deployment completed: raw SHA-256 matches candidate, rollback at
`eboot.bin.before-texture-reuse`. Boot monitor confirms PPSA99680 process164;
startup runtime log is narutorise_043.log. First steady intro intervals requested
150 swaps in 5 seconds with no >50ms gaps; this is not gameplay/FPS proof.
User same-area gameplay/visual comparison requested; performance not yet verified.
Local dist executable and its package manifest now match this candidate. Live
batching configuration remains experimental and differs from repository defaults.
Receipts: `evidence/texture-reuse-deployed.json`, `texture-reuse-result.json`,
`texture-reuse-startup-runtime.log`. Existing heap-release warnings remain unresolved.

## October 14 combined CPU draw optimization candidate

Texture-request-only test: user reports "Still choppy, graphics normal". Saved
narutorise_043.log shows 223 gaps >50 ms across 16 gameplay intervals (~80 sec),
20.4–30.0 swap requests/sec, no shader skips. These are request timings, not measured
display FPS; changing route/draw counts prevent a controlled improvement claim.
User requested a broader code/batching/flushing pass before the next test.

Combined candidate changes (runtime pins unchanged):
- Fetch-register packets now invalidate only changed texture/vertex fetch slots.
  Replayed identical packets retain uploaded fetch constants and texture keys.
  Single writes retain base-register behavior but skip redundant invalidation.
- Base texture preparation returns early only after processing guest-memory
  invalidation and confirming all used bindings remain synchronized.
- Memoize sampler parameter calculations by exact fetch data, shader filtering
  overrides, anisotropy settings and invalid-fetch policy.
- Reuse sampler handles within one submission with exact keys. New submissions
  require normal lookup/usage tracking; samplers used in the current submission
  cannot be evicted by the existing lifetime policy.
- Retain descriptor binding validity for unchanged reused sets. Apply the
  previously computed pipeline-layout compatible-prefix mask before reuse.
- Preserve adjacent primary-buffer batching in packaged defaults, matching the
  existing live setting. Required readback, presentation, upload flush and GPU
  barrier boundaries remain. Upload flushes already skip coherent memory and
  unwritten ranges; pipeline descriptions already reuse the previous match.
- Timing now distinguishes EndSubmission calls from actual vkQueueSubmit calls;
  earlier 'submissions' field counted calls, including no-work frame closure.

Checks: native runtime compilation/link/import and nonoverlapping ELF segments;
ASan/UBSan register-update regression (10000 randomized partial packets, endian
conversion, unchanged replay, boundary masks), descriptor snapshots, save-path
case handling. Patch applies to the fresh pinned baseline. No shared SDK/driver
or other title changed. Source patch includes all new runtime headers.
Candidate SHA-256: `7b84c96389b7bf0df8bcb6bde5c0b46dacd4a2dc4e87a84c9bbfe64dd127fcb2`.
Build: `NARUTO_BUILD_JOBS=4 bash ps5/tools/build-runtime.sh`, then
`python3 ps5/tools/build-title.py --link-only`. Local dist executable and manifest
match. Logs: evidence/batched-draw-checked-{runtime-build,link}.log.
Deployment initiated with evidence/deploy-batched-draw.py and boot-with-logs.py;
raw-hash receipt and gameplay confirmation must be checked before calling this
candidate verified. No claim of smooth/locked framerate or release readiness.

Combined candidate deployment confirmed: raw SHA matches
7b84c96389b7bf0df8bcb6bde5c0b46dacd4a2dc4e87a84c9bbfe64dd127fcb2;
rollback eboot.bin.before-batched-draw. Process165 launched and remains running.
Receipt evidence/batched-draw-deployed.json; startup runtime captured separately.
User same-area comparison requested. Save files and live configuration untouched.
Local dist config now matches repository batching defaults and package manifest.

Combined optimization gameplay log collected after user said done. Latest 12
intervals cover 60.25 seconds: zero swap-request gaps over 50 ms, longest 48.9 ms,
zero shader skips, interval rates 25.6–30.0 swap requests/sec. Prior texture-only
run had 223 >50ms gaps over ~80sec; route/draw counts differ, so this is promising
runtime evidence, not a controlled speedup or displayed-FPS measurement. One
actual queue submission per swap request in these intervals. User-perceived
smoothness and visual correctness remain pending. Evidence:
`evidence/batched-draw-gameplay-result.json` and matching runtime log.

## October 14 dynamic uniform descriptor candidate

User confirms combined batch build is "definitely better but not good enough".
The latest recorded minute has zero swap-request gaps >50ms, but still intervals
around 25.6 requests/sec; this does not establish displayed FPS or locked30.

Next change: five guest uniform bindings use UNIFORM_BUFFER_DYNAMIC, with a
separate correctly typed descriptor pool. Descriptors retain offset zero and the
original buffer/range; original aligned upload offsets are copied into deferred
bind commands in binding-number order. Buffer/range changes allocate descriptors;
offset-only changes reuse the set and force rebinding with new dynamic offsets.
Frame/submission invalidation and resource lifetime tracking are preserved.
Offsets >UINT32_MAX fail explicitly. External/single uniform descriptors remain
non-dynamic. Sampler/texture optimizations from previous candidate are retained.
Counter avoided_constant_updates counts actual invalidations rescued by reuse.

Source review: deferred binder copies offset arrays, allocator supports typed
pools, local RADV source limit is16 dynamic uniform buffers (five needed).
Vulkan reference: https://docs.vulkan.org/guide/latest/descriptor_dynamic_offset.html
and https://docs.vulkan.org/refpages/latest/refpages/source/vkCmdBindDescriptorSets.html

ASan/UBSan dynamic-uniform-descriptors test checks10000 offset-only updates,
exact effective addresses/ranges, buffer/range changes and invalidation. Native
runtime build/link/import/ELF segment checks pass. Fresh-baseline patch check
passes. SHA-256: `6eea437e5128773230a43b3ec9846943edcfd16b419964f4e2913d69ed6326a5` (89824733 bytes). Local dist/manifest match.
Evidence: dynamic-uniform-final-{runtime-build,link}.log; deployment initiated
with deploy-dynamic-uniform.py and boot-with-logs.py. Gameplay speed and graphics
remain unverified for this candidate; no release/push performed.

Dynamic-uniform deployment raw-SHA verified; rollback is
`eboot.bin.before-dynamic-uniform`. Process166 running, startup log
narutorise_045.log. Opening intervals show normal swap cadence with no >50ms
request gaps or shader skips; descriptor-heavy gameplay comparison pending.
User asked to test same area with graphics checks. Evidence:
evidence/dynamic-uniform-deployed.json and dynamic-uniform-startup-runtime.log.

## October 14 descriptor preparation fast path

User says dynamic-uniform build feels smoother but not polished; clarifies the
remaining issue is small stutters/uneven motion. Latest12 intervals in log045
cover60.21sec with zero >50ms request gaps, longest46.0ms. Dynamic uniforms avoid
hundreds of thousands of actual descriptor updates each5sec. This is still not
a controlled displayed-FPS comparison or proof of smooth pacing.

Candidate skips rebuilding texture descriptor image descriptions only if the
shader identity, binding/usage epoch, descriptor layout/shape and every sampler
handle match an already valid set. Binding changes, memory reload transitions,
submission boundaries and 3D-wrapper policy changes invalidate reuse. Existing
full descriptor comparison remains the fallback. Game timing unchanged.

Extended descriptor-snapshot ASan/UBSan regression verifies layout/count/sampler
matching, out-of-range sampler rejection and invalidation. Native build/link,
ELF segment/import validation and fresh-baseline patch checks pass. Candidate:
`8742b0012434d566b721e2c635b1223e081f7049c49e24b47c6a80de8ddbc831`,89825853bytes. Source patch and local dist/manifest updated.
Evidence descriptor-fast-{runtime-build,link}.log; deployment initiated using
existing close/deploy/boot tools, only PPSA99680. Same-area manual test pending.

Descriptor-fast deployment verified by raw SHA; rollback eboot.bin.before-descriptor-fast.
Process167 running; startup log saved as evidence/descriptor-fast-startup-runtime.log.
Receipt evidence/descriptor-fast-deployed.json. User same-area stutter/graphics
comparison requested. Save files and live config preserved. Not released/pushed.

## October 14 regression diagnosis and targeted texture invalidation

User reports descriptor-fast build feels worse, explicitly says DO NOT ROLLBACK,
then asks to check FPS, identify why and fix. No rollback took place. Latest
forward candidate retains all preceding optimizations and timing behavior.

Log046 busy intervals17:39:29–17:40:00: 89–136 swap requests per~5sec (~17.8–27.1/s),
up to97.5ms gaps, no shader skips. Pipeline waits~0.1–0.2ms per5sec; GPU fence waits
~0.8–1.1ms. CPU draw preparation sampled~3.6–4.7sec, texture stage~1.0–1.85sec.
Quieter intervals return~30 requests/sec. These are not displayed-FPS measurements;
scene differences prevent assigning all regression to the last descriptor change.

Found a concrete invalidation inefficiency: one watched texture change called
ResetTextureBindings for all valid bindings, defeating reuse of unrelated textures.
Forward fix checks the existing per-texture atomic outdated masks and resets only
changed unsigned/signed bindings, across all slots (not merely currently used ones).
Retains normal key rebuild/reload for changed textures and notification handling
for races. A cheap atomic load avoids exchange on the common clean path. New
5sec counters report notifications, reset bindings and retained bindings.

Native build/link/import/segment checks and fresh-baseline patch check pass.
Candidate SHA-256 `f048ef3277322a835e8000f1f39e9618b36da6fe0e1acf13f20b280c08a480c9`,89827741bytes. Source patch and dist/manifest match.
Evidence texture-invalidation-{runtime-build,link}.log. Existing working saves
and configuration untouched. Forward deployment initiated; actual performance
and visual correctness remain pending. Do not claim the regression solved until
measured. No release/push. Manual comparison needed in the same busy area.

Forward deployment verified by raw SHA. Process168 remains running; rollback
was not performed. Backup eboot.bin.before-texture-invalidation retains the latest
prior build. Startup evidence texture-invalidation-startup-runtime.log, deployment
receipt texture-invalidation-deployed.json. User comparison pending.

## October 14–15: 60 FPS work

User requested 60 FPS. Current details are in `ps5/CURRENT_STATUS.md` (sections
"60 FPS work" and "Installed candidate"); this entry records the sequence.

- Guest analysis: the open world locks 30 FPS through engine commands 45/68
  (`sub_820EACE0`) and a fixed 1/30 s timer step (`sub_825B6148`); battles
  request 60. `ps5/app/frame_rate.cpp` requests 60 and keeps the timer measured
  with a 1/30 s maximum step. First console build: menus 60/s, village 27–43/s,
  command processor saturated.
- Private Linux harness `.scratch/host-linux` builds the same patched SDK and
  generated code for the laptop RADV, runs headless in nested KWin with a scripted
  virtual pad, and captures guest output. Profiling there drove the SDK changes
  listed in CURRENT_STATUS; idle village view 32.1 → 45.2 requests/s.
- PS5 now passes `--clear_memory_page_state=false` (Xenia Canary default).
- Fixed the `cLoader` thread start race (`ps5/app/loader_thread.cpp`) found with
  new invalid-call diagnostics in `function_dispatcher.cpp`.
- SDK patch re-exported and checked against a fresh pinned SDK with the six
  upstream patches. Console gameplay check of the current build is pending.

## October 15: v6 (three follow-up fixes)

- Root cause of the stale geometry with `clear_memory_page_state=false`: PS5
  16 KB host pages hold four 4 KB guest pages, and the physical heap's write
  watches looked only at the first guest page. Fixed in `xmemory.cpp`; found with
  the new `shared_memory_verify_valid_pages` debug check. The page reset is off
  again by default (`ps5/config/narutorise.toml`).
- Non-swap Vulkan submissions are replayed and submitted on a worker thread
  (`vulkan_async_submission`).
- Console v6: graphics confirmed correct by the user, village 30–50 requests/s
  with no gaps over 50 ms; user: "this one feels better". Release preparation is
  next, pending user approval.

## October 15: steady 30 default

User compared modes: the game's own steady 30 in the open world felt better
than the unlocked 30–50. Console log `evidence/steady30-user-runtime.log`:
30.0 requests/s, max frame gap 35 ms outside loading. `naruto_fps60 = false` is
now the packaged default (battles and menus remain 60). Frame generation work
is on hold at the user's request.

## October 15: release structure

The PS5 executable contained recompiled game code, so it could not be
published. Following the Link's Awakening port, the app is now runtime-only and
loads `game/naruto.module`, which `ps5/tools/setup.py` builds from the player's
own ISO (Python XDVDFS extractor, hash check of default.xex and ai2c.dll, pinned
ReXGlue code generator, pinned llvm-mingw and payload SDK headers). Code
generation is deterministic (only dependency-file paths differ). Release
packaging: `package-title.py`, `package-setup.py`, `package-sources.py`; see
`ps5/RELEASING.md`. Project license: MIT.
