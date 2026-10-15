# Naruto: Rise of a Ninja for PS5

Native PS5 port of the Xbox 360 game **Naruto: Rise of a Ninja** (2007), based
on the [Naruto: Rise of a Ninja PC recompilation](https://github.com/jorjbarbosa/naruto-rise-recomp)
and the [ReXGlue SDK](https://github.com/rexglue/rexglue-sdk), rendered with
Vulkan through RADV. Title ID: **PPSA99680**.

The game's PowerPC code is statically recompiled to native code. You must supply
your own **Naruto: Rise of a Ninja** Xbox 360 disc image (USA/Europe release).
Game files, generated game code and saves are not distributed with this project.

[Releases](https://github.com/mshivam019/naruto-ps5/releases) ·
[Install](ps5/SETUP.md) · [Build from source](ps5/BUILD.md)

## Features

- Native homebrew title launched from the PS5 home screen (tested on firmware 9.00).
- The original frame rates: a steady 30 FPS in the open world, 60 FPS in battles
  and menus.
- DualSense controls with PlayStation button prompts and rumble.
- Save-file pictures in the Load Game menu.
- Native audio and Bink movie playback; persistent shader cache.
- Saves and caches stored with the title on the PS5.

An experimental 60 FPS open world (about 30–50 FPS in the village) is on the
[`experimental-60fps`](https://github.com/mshivam019/naruto-ps5/tree/experimental-60fps)
branch.

## Install

1. Install **PPSA99680.zip** with your native-title launcher or PS5 Upload.
2. On a Linux computer (Windows: WSL2), extract the matching setup ZIP and run
   `sh setup.sh --iso "/path/to/Naruto - Rise of a Ninja.iso"`.
3. Copy the `game` folder setup creates into `/mnt/ext1/etaHEN/games/PPSA99680/`.
4. Launch **Naruto: Rise of a Ninja** from the home screen.

[`ps5/SETUP.md`](ps5/SETUP.md) has the requirements, layout and controls.

## How it works

The app (`eboot.bin`) contains the PS5 runtime only. Setup builds the game code
from your disc into a private module, `game/naruto.module`, which the app loads
at startup and binds to its runtime (`ps5/module/`). Port-specific changes:

- **PS5 runtime** (`ps5/patches/rexglue-ps5.patch` on the pinned SDK): sandbox
  paths, fibers, clocks, guest memory on PS5 direct memory with 16 KB-page write
  watches, native display, asynchronous Vulkan submission and GPU
  command-processor optimizations.
- **Loader thread race fix** (`ps5/app/loader_thread.cpp`, compiled into the
  module): the engine starts its `cLoader` thread before storing the thread's
  entry function; the thread now waits for its creator.
- **PlayStation button prompts** (`ps5/app/button_prompts.cpp`): the game's
  Xbox prompt textures are recognised by content hash when they load and redrawn
  as Cross, Circle, Square, Triangle, L1 and L3. The glyphs are drawn by the
  port; no console artwork is included.
- **Save-file pictures** (`ps5/app/save_picture.cpp`): the 320×180 picture the
  game captures when saving is read back from the GPU.

Development notes and measurements are kept on the `experimental-60fps` branch.

## Repository layout

```text
ps5/app/       Native app: startup, input, prompts, save pictures
ps5/module/    Game-module loader and the app/module contract
ps5/tools/     Setup, module build, app build and release packaging
ps5/patches/   ReXGlue SDK changes for PS5
tools/         Source fetching and the Linux profiling harness
```

## Credits and licensing

The Naruto PC recompilation by jorjbarbosa supplies the code-generation
configuration; ReXGlue by Tom Clay (derived from Xenia) supplies the recompiler
and runtime. Mihawk's PS5 Vulkan template supplies the native graphics and
application foundation.

Project code is MIT ([LICENSE](LICENSE)). The app also contains BSD, MIT, zlib,
Apache-2.0, LGPL and GPL components; see
[`ps5/THIRD_PARTY_NOTICES.txt`](ps5/THIRD_PARTY_NOTICES.txt). Each release
includes their license texts and a source archive. The upstream Naruto project
declares no license; setup downloads its configuration at a pinned revision and
this repository does not include it.

This is an unofficial fan project, not affiliated with or endorsed by Ubisoft,
Bandai Namco, Masashi Kishimoto/Shueisha, Microsoft or Sony.
