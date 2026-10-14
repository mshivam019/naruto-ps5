# Local candidate ready — October 14, 2026

Title: Naruto: Rise of a Ninja / PPSA99680. Repository remains private.
User requested local preparation only and will connect the PS5 when notified.
No deployment or console test has occurred.

## Prepared artifact

`dist/PPSA99680`: 108 files, 4736922828 bytes, source/destination SHA-256 verified.
Current local eboot SHA-256: `e84ff472776b2ecdcaf14e7018f82a08abf94d06fce246a35695f25721e20b21`.
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

## First console attempt

User connected console and authorized testing. Console idle, FTP/control reachable.
All 108 files transferred to M.2; stored sizes checked and executable/libc/param
verified through raw SHA-256. Registration returned PPSA99680 successfully.
Launch control connection closed without a response; afterward FTP, control and
Payload Manager were unreachable. No boot/gameplay result was captured. Cause
unknown; awaiting user observation. Do not retry launch blindly. Receipts:
`evidence/console-deployment.json`, `evidence/first-boot-result.json`.

## Layout correction prepared after shutdown

User confirmed power-off/reboot during the first launch. Local inspection found
overlapping executable/readonly/RELRO LOAD ranges in the initial converted ELF.
Large-code sections and the libc++ `__lcxx_override` executable section were
not explicitly placed by the inherited linker script. A project-local layout
now places them in the proper segments. Intermediate and converted ELF checks
pass; the new validator rejects the original binary and runs before signing.
This is a confirmed layout defect, not a confirmed shutdown root cause.
Corrected SELF remains local and is reflected in `dist/PPSA99680` and its receipt.
Console still holds the first candidate. Wait for recovery before deployment;
back up the installed executable, upload only the replacement, raw-hash verify
and collect klog concurrently BEFORE sending a new launch request.
