# Naruto: Rise of a Ninja — native PS5 work in progress

Target: PPSA99680, Vulkan, firmware 9.00. This private development repository
contains port code and build tooling. There is no verified playable PS5 build yet.

The host ReXGlue tool builds, and both base-game executable modules have been
translated locally. The PS5 runtime and native app integration are in progress.

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
