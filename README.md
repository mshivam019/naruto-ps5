# Naruto: Rise of a Ninja — native PS5 work in progress

Target: PPSA99680, Vulkan, firmware 9.00. This private development repository
contains port code and build tooling. There is no verified playable PS5 build yet.

The complete native candidate builds, links and signs locally. Both translated
game modules, the PS5 runtime, Xenos Vulkan renderer, controller bridge and XMA
decoder are included. The private title is staged at `dist/PPSA99680`. Console
boot, rendering, controls, audio, saves and exit remain untested.

Game files, generated C++, local executables and screenshots are excluded from
Git. The first local test executable will contain generated game code and must
not be uploaded. No release or redistribution of the upstream game-port source
is implied; the upstream Naruto project currently has no root license.

## Local preparation

1. Run `python tools/fetch-sources.py` to fetch the exact source pins.
2. Run `tools/prepare-host.sh` to build the host code generator.
3. Extract your base-game Xbox 360 disc into `private/game_root/`.
4. Run `python tools/prepare-codegen.py` to validate identity and prepare a
   base-game manifest (without DLC).
5. Run `upstream/naruto-rise-recomp/sdk/out/linux-amd64/rexglue codegen private/codegen/narutorise_manifest.toml`.

The supplied disc identified as title 555307E5, media 12DF7A0D. Title identity
and successful code generation do not establish gameplay compatibility.

Native integration reuses the working PS5 Vulkan template/platform and controller
bridge from the local Wind Waker and Link's Awakening ports. The PS5 build still
requires those prepared native dependencies; a standalone setup is not ready.

See `PLAN.md` for current build evidence and outstanding work.

## Native candidate build

After host code generation, run these commands from the repository root:

```sh
python3 tools/apply-ps5-patch.py
bash ps5/tools/build-ffmpeg.sh
python3 ps5/tools/build-title.py --jobs 4
bash ps5/tools/test-memory.sh
python3 ps5/tools/package-local.py
```

Packaging requires a new output directory and verifies every copied file with
SHA-256. The signed executable and packaged title contain private generated game
code and disc assets: do not publish either. Build tooling currently depends on
this workspace's prepared SDK, SDL3 and RADV; this is not a standalone installer.
Run host preparation before applying the PS5 patch on a fresh checkout.

Controller defaults: Cross=A, Circle=B, Square=X, Triangle=Y, Options=Start,
touchpad=Back. User data and caches are separate under `/app0/user`.

See `ps5/CURRENT_STATUS.md` for the candidate and pending console checks.
