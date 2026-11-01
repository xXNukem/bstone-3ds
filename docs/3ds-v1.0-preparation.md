# Native 1.0 candidate: implementation and validation

This is preparation for local hardware testing, not a published or certified
release. Existing uncommitted port work is retained. No Git commits, merges,
pushes, game-data migration or public publication are performed.

Follow-up hardware feedback added the startup framebuffer-format correction and
optional Modern/C-Stick controls. See [the follow-up notes](3ds-selector-controls.md)
and the current README for those additions; the initial audit below describes
the preceding candidate.

## Initial architecture and reuse

The previous `main` synthesized `--aog`, `--data_dir`, `--profile_dir` and audio
arguments, preventing a real pre-resource choice. BStone already has complete
edition-specific resource lists, SHA-1 signatures, `AssetsInfo`, weapon/sound
indices and save/score names. These mechanisms remain authoritative.

`bstone_game_catalog.{h,cpp}` moves the original asset types/tables out of
`id_ca` without changing their signatures. Both selector and original content
finder use that catalog. The selected verified version is passed internally to
`InitDestPath`, `find_contents` and `get_profile_dir`; no synthetic argv is built.
Engine cvars, assets, fonts, sprites and sounds initialize only after selection.

SDL2's 3DS backend owns initial gfx/HID/APT setup and graphics shutdown. The new
`bstone_3ds_display` owns only the existing Citro3D/Citro2D context and three
targets, shared by selector and `SwVideo`. It never calls gfxInit/gfxExit or
hidInit/hidExit. GPU textures and targets are freed before SDL shutdown. Partial
initialization and repeated shutdown are covered by host lifecycle tests.

`bstone_3ds_launcher` uses a resource-free, original 5x7 font, existing native
font drawing and common CRT background. Both screens update in the same GPU
frame. Selector eye offsets are small and slider-controlled; world/weapon eye
calculations are unchanged. The approved engine menus retain their layout.

## Discovery, integrity and selection

The platform-independent `bstone_3ds_installation` scans these candidates:

| Candidate | Directory | Profile |
|---|---|---|
| AOG Full, new | `games/aog/` | `profiles/aog_full/` |
| AOG Shareware, new | `games/aog/` | `profiles/aog_sw/` |
| AOG Full, legacy | installation root | `profiles/aog_full/` |
| AOG Shareware, legacy | installation root | `profiles/aog_sw/` |
| Planet Strike | `games/ps/` | `profiles/ps/` |

All paths are below `sdmc:/3ds/bstone/`. Complete Full is preferred to complete
Shareware; within an edition the new location is preferred. Left/right cycles
present AOG candidates, allowing Shareware selection when Full is installed.
Returning from PS remembers that AOG choice. Directories are never combined.

Presence of every nonempty required file is displayed as unverified. One bounded
32 KiB read/hash step runs between frames. The user can navigate, rescan or exit
while hashes run; no worker thread performs graphics. All hashes must match the
same known release before READY/start is allowed. The selected file lengths are
rechecked immediately before engine initialization. Unknown/mixed data remains
INVALID. No fabricated progress percentages or persistent verification cache.

Recognizable AOG SW/Full versions: 1.0, 2.0, 2.1 and 3.0. The original PS signature
set represents the 1.0/1.1 family and cannot safely distinguish those numbers.
Required counts are SW 10, Full 12 and PS 10; the README lists every filename.
Uppercase/all-lowercase lookup matches the original loader; FAT handles mixed
case. Selection remains fixed until application exit.

## User data and legacy safety

Edition-specific original save/score names and binary save formats remain
unchanged. Config, screenshots, save and score writes use only the selected new
profile. AOG reads may fall back to the same old-root filename only when the new
profile file is absent. A new slot shadows its old copy without modifying it.
PS never imports AOG's shared old-root config. No files are moved or deleted.
The common root log is not a game config. Manual migration is in the README.

## Planet Strike review

Original `AssetsInfo` initialization already selects PS sprite/sound tables,
six weapon definitions, projectiles, levels, save/score names and title assets.
The native lower menu title already branches on the selected `AssetsInfo`, as
does the intro. It does not need a second title implementation.

Two native HUD assumptions were corrected: PS displays its three actual access
cards (red/yellow/blue), not AOG's five, and the AOG lives cell displays PS plasma
detonator inventory. PS B interaction still uses the original engine detonator
logic. No raycaster, stereo calibration, input mapping or combat logic changes.
Retail PS progression, messages, briefings, music and endings must still be
tested with legitimate PS resources on hardware; this audit is not that test.

## Stable build and metadata

`BSTONE_ENABLE_AI_WEAPONS` defaults OFF. Stable builds omit the experimental
loader and video-menu row; original sprite/bob/animation paths remain. The
experimental source and preparation tool are preserved, and can be built with
the explicit ON option for future work. No experiment pack belongs in the test
installation or eventual 1.0 package.

Audio defaults retain the prior 22050 Hz / 80 ms values previously forced in
argv. The actual mixer/decoders are unchanged, and saved audio preferences are
respected. An earlier weapon-audio complaint remains a hardware verification
item, not a claimed fix from host routing tests.

CMake generates SMDH from `assets/bstone-3ds-icon-48.png` with devkitPro smdhtool
and embeds it using ctr_create_3dsx. The icon is an original hand-authored pixel
BS emblem; Python generation is optional authoring, not a build dependency.
Homebrew author: **Carlos de la Barrera Perez**. Original engine/game/library
credits remain in the README, upstream documents and license files.

`tools/verify_3ds_metadata.py <build>/src/bstone --stable` checks the actual
16-language metadata records, both icon sizes, exact embedded SMDH bytes and
absence of the experimental loader/menu from the stable executable.

## What was actually tested

- Real AOG Shareware 3.0 files: full production SHA-1 verification, uppercase and
  lowercase layouts, legacy fallback, fresh profile isolation.
- All nine known engine signature vectors: identification and rejection of
  partial/mixed vectors. These are not substitute retail game installations.
- Original AssetsInfo, sprite and music initialization for all nine versions:
  distinct SW/Full/PS weapon indices, PS BFG frames and episode/level counts.
- Synthetic missing, empty and invalid Full/PS/SW installations, both-game
  presence, no-game installation, Full preference and explicit SW choice.
- Actual selector input loop with host HID/GPU boundaries mocked: D-Pad,
  Circle Pad, held A/B, exit/back, invalid-data refusal and frame pumping.
- Production shared display owner: reuse, idempotent cleanup and every partial
  target-creation failure. No device rendering is claimed by this mock test.
- Existing production-path tests for Classic input, menu press repetition,
  bobbing/stereo coverage, CRT/documents/loading and weapon sound routing.

Real previous saves, physical 3D comfort, both-screen restoration, audible
audio, all retail game progression and Homebrew return remain hardware checks.
Do not declare the release stable until those checks are complete.

## Files in this change

- New catalog, installation scanner, launcher and shared display source/header
  pairs; native terminal exposes its existing CRT background for startup reuse.
- `3d_main`, `jm_free`, `id_ca`: startup selection, catalog integration, scoped
  profiles and safe shutdown. `3d_def` declares the legacy-safe read path.
- `3d_menu`: legacy save reads and stable video rows. `3d_draw`, `3d_scale`,
  `id_vl`: compile-time isolation of experimental weapon activation/loading.
- `bstone_sw_video`: shared Citro lifecycle. `bstone_3ds_hud`: PS cards/detonators.
- `id_sd`: preserve prior console audio defaults without fake command arguments.
- CMake, original icon assets/generator, metadata verifier, launcher/lifecycle
  tests, experimental regression-test build defines, README and this document.

Existing unrelated dirty files and experiment source are not reverted or
removed. Local build outputs contain no redistributed original game data.
