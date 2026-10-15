# Building from source

Players only need the release files and [setup](SETUP.md). This page builds the
app itself.

## App (`eboot.bin`)

The app build reuses native dependencies prepared by the sibling PS5 ports in
the same workspace: `../PS5_Vulkan` (RADV, link recipe, `libc.prx`),
`../Dusklight-PS5` (SDL3 and Mesa sources), `../Links-Awakening-PS5` (prepared
payload SDK) and `~/ps5sdk/native-app-boilerplate` (Clang 18 wrappers).

```sh
python3 tools/fetch-sources.py        # pinned Naruto PC port + ReXGlue SDK
for p in upstream/naruto-rise-recomp/patches/sdk/*.patch; do
  git -C upstream/naruto-rise-recomp/sdk apply "$PWD/$p"
done
python3 tools/apply-ps5-patch.py      # ps5/patches/rexglue-ps5.patch
bash ps5/tools/build-ffmpeg.sh
python3 ps5/tools/build-title.py --jobs 8
python3 ps5/tools/validate-elf-layout.py build/native-title/eboot.elf
bash ps5/tools/test-memory.sh
```

## Game module

`ps5/tools/setup.py` is the reference: it builds the code generator, extracts
the disc, generates code and runs `ps5/tools/build-game-module.py` with the
pinned toolchain (`ps5/module/toolchain.json`). For development:

```sh
python3 ps5/tools/setup.py --iso /path/to/naruto.iso --output /tmp/prepared/PPSA99680
```

The app and module share a contract: the runtime symbols the module imports
(`ps5/module/host-imports.txt`) and an ABI identity hashed from that list, the
compile flags (`ps5/module/compile-flags.txt`), the SDK patch and the module
preparation script. When any of these change, regenerate the list from a module
build and rebuild the app:

```sh
python3 ps5/tools/build-game-module.py --write-imports \
  --generated build/setup/codegen/generated \
  --toolchain build/setup/tools/llvm-mingw-20260922-ucrt-ubuntu-22.04-x86_64/bin \
  --sysroot build/setup/tools/ps5-payload-sdk \
  --sdk upstream/naruto-rise-recomp/sdk \
  --work build/game-module --output build/game-module/naruto.module --jobs 8
```

A module built for a different ABI is refused at startup.

## SDK patch

Edits to the SDK under `upstream/` are not tracked; export them into
`ps5/patches/rexglue-ps5.patch` as the difference from the pinned SDK plus the
six upstream patches, and check that it applies to a fresh checkout.

## Linux profiling harness

`tools/host-harness/` builds the same SDK and generated code for a Linux
desktop; see its README.
