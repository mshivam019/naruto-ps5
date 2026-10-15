# Releasing

Keep the title ID **PPSA99680** for every release. Raise `contentVersion` in
`ps5/sce_sys/param.json` for each new release and commit it before tagging. Tag
releases `vMAJOR.MINOR.PATCH`.

A release has four assets, all built from the tagged commit:

| Asset | Built with |
| --- | --- |
| `PPSA99680.zip` | `python3 ps5/tools/package-title.py --output release/PPSA99680.zip` |
| `Naruto-PS5-vX.Y.Z-setup-linux.zip` | `python3 ps5/tools/package-setup.py --output release/Naruto-PS5-vX.Y.Z-setup-linux.zip` |
| `Naruto-PS5-vX.Y.Z-dependency-sources.tar.gz` (+ `.manifest.json`) | `python3 ps5/tools/package-sources.py --output release/Naruto-PS5-vX.Y.Z-dependency-sources.tar.gz` |
| `SHA256SUMS` | `(cd release && sha256sum -- * > SHA256SUMS)` |

Before publishing:

1. Build the app (`ps5/tools/build-title.py`), install it on a console with a
   game folder prepared by the release's own setup ZIP (extracted to a clean
   directory), and play the village, a battle, saving and loading.
2. The app ZIP contains `PPSA99680/` with `eboot.bin`, `sce_sys/param.json`,
   metadata, the default `narutorise.toml` and license texts only: no `game/`,
   no `user/`, no generated code. `package-title.py` checks this.

Never replace an asset of a published release; publish a new version instead.

## Homebrew catalog

Follow the [catalog submission guide](https://github.com/blackbearreloaded/ps5-homebrew-catalog/blob/main/docs/submitting.md).
The repository must be public with its MIT license detected. The record, in a
fork of the catalog as `apps/PPSA99680.json`:

```json
{
  "titleid": "PPSA99680",
  "name": "Naruto: Rise of a Ninja for PS5",
  "kind": "game",
  "description": "Native PS5 port of Naruto: Rise of a Ninja. Requires game files prepared from your own Xbox 360 disc.",
  "license": "MIT",
  "author": "mshivam019",
  "version": "vX.Y.Z",
  "source_repo": "https://github.com/mshivam019/naruto-ps5",
  "artifact_url": "https://github.com/mshivam019/naruto-ps5/releases/download/vX.Y.Z/PPSA99680.zip",
  "sha256": "<sha256 of PPSA99680.zip>",
  "icon_url": "https://raw.githubusercontent.com/mshivam019/naruto-ps5/vX.Y.Z/ps5/sce_sys/icon0.png"
}
```

Check it with `python3 -m catalog check` and `python3 -m catalog verify PPSA99680`
in the catalog checkout, then open the pull request from the repository owner's
account, changing only that record.
