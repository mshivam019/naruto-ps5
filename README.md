# Naruto: Rise of a Ninja — native PS5 port

Native PS5 build of *Naruto: Rise of a Ninja* (Xbox 360, title `555307E5`) using
the [naruto-rise-recomp](https://github.com/jorjbarbosa/naruto-rise-recomp)
static recompilation and the ReXGlue SDK, rendered with Vulkan through RADV.
Title ID `PPSA99680`, tested on firmware 9.00.

**Status:** playable. Picture, audio, controls, saves and reloads have been
confirmed on console. Like the Xbox 360 original, the open world runs at a
steady 30 FPS and battles and menus at 60. An experimental 60 FPS open world
(work in progress, about 30–50 FPS in the village) lives on the
[`experimental-60fps`](https://github.com/mshivam019/naruto-ps5/tree/experimental-60fps)
branch. See [`ps5/CURRENT_STATUS.md`](ps5/CURRENT_STATUS.md) for measurements
and open work.

This repository contains only port code, build tooling and patches. Game files,
generated C++, executables and packaged titles are excluded: the PS5 executable
embeds code recompiled from your own disc and must not be redistributed. The
upstream Naruto project currently has no root license.

## What the port changes

- **Loader thread race fix** (`ps5/app/loader_thread.cpp`): the engine starts
  its `cLoader` thread before storing the thread's entry function; the thread
  now waits for its creator.
- **PlayStation button prompts** (`ps5/app/button_prompts.cpp`): the game's
  Xbox prompt textures are recognised by content hash when they load and
  redrawn as Cross, Circle, Square, Triangle, L1 and L3. The glyphs are drawn
  by the port; no console artwork is included. Cvar
  `naruto_playstation_prompts` (default true).
- **Save-file pictures** (`ps5/app/save_picture.cpp`): the 320×180 picture
  the game captures when saving is read back from the GPU, so Load Game shows
  it. Only that copy is read back; reading every copy back drops the console
  to about 2 FPS.
- **Rumble** works through the DualSense in compatibility mode; the game only
  vibrates in battles.
- **PS5 runtime** (`ps5/patches/rexglue-ps5.patch`): sandbox paths, fibers,
  clocks, aliased guest memory on PS5 direct memory, native display, and
  GPU command-processor optimizations.
- **Controls:** Cross=A, Circle=B, Square=X, Triangle=Y, Options=Start,
  touchpad=Back. User data and caches live in `/app0/user`.

## Building

The build reuses the PS5 Vulkan template, payload SDK, SDL3 and RADV prepared by
the sibling ports in this workspace (`../Links-Awakening-PS5`,
`../Dusklight-PS5`, `../PS5_Vulkan`); it is not yet a standalone setup.

1. `python3 tools/fetch-sources.py` — fetch the pinned upstream sources.
2. `tools/prepare-host.sh` — build the host code generator.
3. Extract your Xbox 360 disc into `private/game_root/`, then
   `python3 tools/prepare-codegen.py` to check the title and write the manifest.
4. `upstream/naruto-rise-recomp/sdk/out/linux-amd64/rexglue codegen private/codegen/narutorise_manifest.toml`
5. Build and package:

```sh
python3 tools/apply-ps5-patch.py
bash ps5/tools/build-ffmpeg.sh
python3 ps5/tools/build-title.py --jobs 4
bash ps5/tools/test-memory.sh
python3 ps5/tools/package-local.py
```

Packaging writes to a new output directory and verifies each copied file with
SHA-256.

## Profiling on Linux

`tools/host-harness/` builds the same patched SDK and generated code for a Linux
desktop with Vulkan, runs the game headless inside a nested KWin with a scripted
virtual gamepad, logs guest frame pacing and the engine timer, and captures the
guest output. See [`tools/host-harness/README.md`](tools/host-harness/README.md).

## Documentation

- [`ps5/CURRENT_STATUS.md`](ps5/CURRENT_STATUS.md): installed build, measured
  performance, known issues.
- [`PLAN.md`](PLAN.md): chronological development log.
- [`ps5/THIRD_PARTY_NOTICES.txt`](ps5/THIRD_PARTY_NOTICES.txt): third-party
  notices.
