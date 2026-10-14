# Naruto: Rise of a Ninja PS5 Vulkan handoff

Updated 2026-10-14. Workspace: `/home/skm/Downloads/ps5/Naruto-Rise-PS5`.

## Current status (supersedes historical entries below)

Console testing is authorized for PPSA99680, including closing/restarting Naruto.
Native startup reaches Vulkan initialization, XEX loading, controller detection
and guest threads, and the user now confirms a visible game picture; gameplay is pending. The initial build
powered off the console; corrected nonoverlapping ELF segments have avoided that
in subsequent tests. Further startup fixes cover sandbox paths and PS5 thread
resume semantics. The native exit was traced to getcontext overwriting a 928-byte SDK context
with 1216 bytes (mc_len=1152). A locally padded context and PS5 fiber stack
initialization fix is deployed. Three fiber round trips passed and the user
confirmed that the picture is visible. The follow-up timing/audio build fixes
an 81x clock scaling error and execute-only FFT table access; the user confirms
normal speed and working Start. Gameplay/save/exit checks are pending. Preserve
the running test session. Disk shader
storage is temporarily disabled while its separate startup stall is investigated.
Evidence is private under `evidence/`; console rollback executables are retained.
No release is ready. See ps5/CURRENT_STATUS.md for previous build details; its
local-only and layout-not-deployed statements are superseded by this entry.

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
