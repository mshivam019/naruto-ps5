# Local candidate ready — October 14, 2026

Title: Naruto: Rise of a Ninja / PPSA99680. Repository remains private.
User requested local preparation only and will connect the PS5 when notified.
No deployment or console test has occurred.

## Prepared artifact

`dist/PPSA99680`: 108 files, 4757531820 bytes, source/destination SHA-256 verified.
Signed eboot SHA-256: `c3895b3c98d261ccca32910fb8232919e88359e7b018151354191f6b959c7355`.
This entire title is private: it includes generated game code and disc assets.

## Validation completed

- Host code generation for the base default.xex and AI2C.dll modules.
- Complete native compilation, linking, conversion and SELF signing/inspection.
- Native import audit and expected runtime/controller/module/GPU symbol checks.
- Host memory alias, protection, failure rollback and teardown checks with UBSan.
- PS5 runtime patch applies after the six pinned upstream Naruto patches.
- Full staged-file hash comparison; receipts and logs remain in `evidence/`.

The runtime uses native RADV/direct display, SDL3 controller/audio integration,
a static AI2C module and an isolated XMAFRAMES FFmpeg build. Original game files,
sibling ports, shared SDK and console configuration were preserved.

## Next console session

Once the user connects it, recheck helper availability and current title. Install
PPSA99680 under `/mnt/ext1/etaHEN/games/`, verify transfers, then test startup and
capture logs before assessing gameplay. Preserve other titles and all saves.
Required checks: guest memory/MMIO, Vulkan presentation and Xenos rendering,
new game/movement/combat/camera, audible audio/cutscenes, save/reload and exit.
Successful compilation is not evidence of a playable port or a performance claim.

Build log: `evidence/ps5-full-build.log`; package receipt:
`evidence/local-package.json`; dependency hashes: `evidence/native-dependencies.json`.
