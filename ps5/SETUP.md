# Installing Naruto: Rise of a Ninja on PS5

The app (`PPSA99680.zip`) contains no game. You prepare the game on a computer
from your own disc image with the setup package from the same release, then
copy the result into the installed app.

## Requirements

- A homebrew-capable PS5 with native folder-title support (tested on firmware 9.00).
- Your own **Naruto: Rise of a Ninja** Xbox 360 disc image (`.iso`), the
  USA/Europe release (title 555307E5). Setup checks the game files and refuses
  other versions.
- Linux x86-64 with Python 3.11+, Git, CMake, Ninja and Clang, plus about 15 GB
  of free space. On Windows, use WSL2 with Ubuntu.

On Ubuntu 24.04 (also in WSL2):

```sh
sudo apt install python3 git cmake ninja-build clang
```

## Install

1. Install `PPSA99680.zip` with your native-title launcher or PS5 Upload.
2. On your computer, extract `Naruto-PS5-<version>-setup-linux.zip` and run, from
   its folder:

   ```sh
   sh setup.sh --iso "/path/to/Naruto - Rise of a Ninja.iso"
   ```

   Setup downloads pinned public sources and tools (the ReXGlue SDK, a portable
   LLVM, the PS5 payload SDK headers), extracts your disc, generates the game
   code and compiles it. It takes a few minutes and prints the output folder
   (default `../Naruto-PS5-prepared/PPSA99680`; choose another with `--output`).
3. Close the app on the PS5. Copy the output's `game` folder into
   `/mnt/ext1/etaHEN/games/PPSA99680/`, next to `eboot.bin`.
4. Launch **Naruto: Rise of a Ninja** from the home screen.

The installed layout:

```text
PPSA99680/
  eboot.bin
  narutorise.toml     settings (replaced by app updates)
  sce_sys/  sce_module/  licenses/
  game/               from setup: your disc files and naruto.module
  user/               created by the app: saves and shader caches
```

The `game` folder contains your game data and code generated from it. Do not
share it. Keep the app and the game folder from the same release: after an app
update, run the matching setup again and replace `game/naruto.module` (the app
refuses a module built for a different version). Keep `user/` when updating.

If the app closes right after launch, `game/naruto.module` is missing or from a
different release.

## Controls

| DualSense | Xbox 360 |
| --- | --- |
| Cross / Circle / Square / Triangle | A / B / X / Y |
| L1 / R1, L2 / R2 | LB / RB, LT / RT |
| Options | Start |
| Touchpad press | Back |

The game shows PlayStation button prompts. Rumble works in battles.
