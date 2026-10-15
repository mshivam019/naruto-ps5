# Naruto: Rise of a Ninja PS5 workspace

`main` is the stable release line. Development notes (`ps5/CURRENT_STATUS.md`,
`PLAN.md`) and the experimental 60 FPS work live on branch `experimental-60fps`;
read them there before performance work. Shared console instructions are in
`../AGENTS.md`. Release steps: `ps5/RELEASING.md`.

The port is playable on PPSA99680 (firmware 9.00). The app is runtime-only and
loads `game/naruto.module`, built by `ps5/tools/setup.py` from the player's disc. User
data, saves and caches live in `/app0/user` on the console; preserve them.

Keep source modifications, build outputs and receipts isolated in this folder.
Do not modify working sibling ports or update console payloads for this project.
Do not publish game dumps, derived game code, executables, credentials or
private artifacts. Repository https://github.com/mshivam019/naruto-ps5 stays
private; the user plans a release after the remaining performance work is
tested. Verify console availability and the running title before deployment;
close the target before replacing live files and keep an `eboot.bin.before-*`
backup. Report actual test results.

The user authorizes closing/restarting Naruto during testing and asks to be
told when to play for a test. This does not authorize closing unrelated titles.
The intended runtime is native PS5 with Vulkan, without WebGPU.

SDK edits live in the ignored `upstream/naruto-rise-recomp/sdk` tree; export
them to `ps5/patches/rexglue-ps5.patch` (diff against the pinned SDK plus the
six upstream patches) before committing.
