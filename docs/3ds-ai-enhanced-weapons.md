# AI ENHANCED WEAPON DESIGNS — local optional integration

The laboratory's final artwork was visually approved by the user. This implements
an optional presentation path, default OFF, on feature/3ds-menu-redesign. It does
not change original VSWAP files, combat/weapon state, saves, controls, audio,
raycasting, HUD, or stereo projection.

## Actual assets audited

Source: `~/Desktop/BlakeStone-AI-Enhanced-Weapons/index.html`, embedded manifest.
The current user-approved presentation is **Previous Designs + Uniform Scale**,
reviewed in `~/Desktop/BlakeStone-AI-Enhanced-Weapons/previous-uniform-scale/`.
Its 25 PNGs are byte-identical to the first generation's final artwork. The
discarded regenerated designs are NOT installed. Only the pack's scale headers
and corresponding CRCs change; frame payloads remain identical.
Only its `frames[].enhanced` paths under `animations/aog-sw-3.0/` are converted.
Not the extracted originals, discarded master concepts, working 256-pixel images,
or the older weapon-study experiments. All 25 chosen frames are 128×128 RGBA,
alpha 0/255, with RGB already exactly in the engine VGA palette. No resampling,
dithering, palette approximation, PNG decoding or color quantization happens in
the gameplay loop. The converter preserves every visible chosen color and hole.
SHA-256 of every source PNG is retained in the local pack report. The laboratory
is read-only and its originals remain untouched.

Art source: Aliens of Gold shareware 3.0 only. There are NO approved Planet Strike
frames. Mapping uses the game's semantic weapon slots and edition-initialized
`SPR_*READY` constants, not the shareware sprite numbers in the art manifest.
Thus the five AOG designs map to AOG shareware/full without relabelling them PS.
Full AOG's semantic mapping is exercised on the host; artwork source is still SW,
and in-game visual validation of each edition remains on hardware. PS always
falls back with this local pack; no PS directory is generated.

| Slot / pack file | AOG SW source IDs | AOG full runtime IDs | Frames | Scale / logical 200px |
|---|---|---|---|---|
| 0 autocharge.bsw | 484–488 | 668–672 | 0 READY, 1–4 original attack sequence | 139 |
| 1 protector.bsw | 489–493 | 673–677 | same | 142 |
| 2 pulse.bsw | 494–498 | 678–682 | same | 167 |
| 3 ion.bsw | 499–503 | 683–687 | same | 162 |
| 4 grenade.bsw | 504–508 | 688–692 | same | 180 |

These IDs are descriptive; the code uses the active constants. PS bases
722/727/732/737/742/982 are a separate namespace. Its BFG and all other PS weapons
stay ORIGINAL. Missing or invalid AOG weapon files also fall back for ALL FIVE
phases of that weapon, never phase-by-phase.

## Local preparation / installation

```sh
python3 tools/prepare_ai_weapons.py \
  --lab "$HOME/Desktop/BlakeStone-AI-Enhanced-Weapons" \
  --layout "$HOME/Desktop/BlakeStone-AI-Enhanced-Weapons/previous-uniform-scale/catalog/proposed-layout.json" \
  --output build/ai-weapons-local-pack
```

Python 3 and already-installed Pillow are preparation tools only; no new runtime
dependency. Converter reads the actual gallery, requires complete ordered phases,
validates dimensions/palette/alpha/margins, and emits CRC-protected `.bsw` files.
It stages the entire valid conversion before writing output and replaces each
file atomically. Do not update the SD pack while the application is running.
An optional `--layout file.json` supplies per-weapon `refHeight`, `anchor:[x,y]`,
`offset:[x,y]` overrides using the same format/bounds; no extra runtime preference
file or duplicated positioning logic. The explicit layout argument above is
required to reproduce the approved uniform presentation: omitting it restores
the older original-area-based calibration. Changed artwork must be re-audited.

Copy `build/ai-weapons-local-pack/ai-weapons/` to:

```
sdmc:/3ds/bstone/ai-weapons/aog/autocharge.bsw
sdmc:/3ds/bstone/ai-weapons/aog/protector.bsw
sdmc:/3ds/bstone/ai-weapons/aog/pulse.bsw
sdmc:/3ds/bstone/ai-weapons/aog/ion.bsw
sdmc:/3ds/bstone/ai-weapons/aog/grenade.bsw
```

This is relative to `data_dir_`, so a custom data directory works too. Pack is
optional and external, not ROMFS or embedded in `.3dsx`. Deleting it restores
normal original-only operation at startup. The locally generated pack is under
ignored `build/`; no artwork is added to tracked source or automatically included
in a release. Original resources/derived-art distribution permissions must be
resolved before publishing a pack. Source/extracted game data is not distributed
by the converter.

## Rendering, scale, bobbing, stereo

`DrawPlayerWeapon` still computes original base + `gamestate.weaponframe` and calls
`scale_player_weapon`; no second animation clock or attack table. The existing
`generic_scale_shape` raster loop accepts either its original 64-pixel Sprite
or a 128-pixel trimmed enhanced Frame. Cropped column-major indexed samples use
-1 transparency, preserving holes separately from color index 0. Sampling is
nearest-neighbor; it still writes the existing indexed VGA framebuffer.

Original OFF/fallback path keeps its previous height (AOG 128 / PS 88), position,
rounding and bob offsets. Enhanced scale is common to all phases of a weapon.
The approved layout normalizes first-generation idle opaque area against the
uniform-scale laboratory's class-specific presentation targets, rather than
against the small original game sprites. A single isotropic presentation factor
per weapon preserves its artwork and animation. Grenade is capped at the existing
loader maximum of 180 and still has the greatest visible presence. At 240 high,
full-canvas projected sizes are 166 / 170 / 200 / 194 / 216 pixels, respectively. Transparent
margins are part of the 128 canvas, not the visible body height. These settings
retain more visual presence while leaving the horizon unobstructed.

All approved frames are horizontally registered at x=64 and have opaque lower
bound y=113 exclusive. Anchor (64,110), offset (0,0): the final three native rows
are deliberately below the lower view boundary, preventing flat glove/receiver
ends from floating. X/Y anchor is fixed through firing phases so AI-created
motion remains intact. Each weapon's physical size differs by its own scale.
Current down-only vertical motion (roughly 0–4 screen pixels) and horizontal sway
reuse the existing oscillator and NO WEAPON BOBBING gate. No change to original
bobbing or introduction of a separate oscillator. No enlarged upward motion is
added until hardware review justifies it.

Exactly the existing visible-pixel write mirrors the weapon to the right-eye
buffer at zero parallax when stereo is active. Both eyes use the SAME selected
Frame, anchor and bob offsets; each transparent pixel preserves that eye's own
world background. Slider-off writes only the active left presentation. No world
projection/Citro3D/global-depth calculation changes.

## Loader / cache / failure policy

At startup after original sprite constants/preferences and on entry to VIDEO
SETTINGS, validate optional files independently, one at a time. A weapon file has
exactly five frames; header, edition, slot, dimension, calibration bounds, lengths,
colors, CRC-32 and EOF are checked. Invalid/incomplete file rejects the weapon.
No partial animation can enter the cache. Enabling retries validation from the
menu (pack may be installed between runs). No per-frame polling of the SD.

On first use / weapon change, read a complete weapon transactionally. Keep ONE
weapon resident; the old cached weapon survives while validating the replacement.
On failure, pin that slot to original until explicit rescan. A cached five-phase
animation stays immutable even if its file disappears, so removal cannot change
its design between shots. Switching away then attempting to reload it falls back
for the whole weapon. OFF frees the enhanced cache immediately. Originals remain
available through the unchanged engine cache for immediate restoration.

Pack total: 275,256 bytes (~269 KiB). Largest trimmed pixel payload resident:
63,348 bytes (~62 KiB), plus five Frame records/vector bookkeeping. Worst normal
switch peak is roughly old cache + new file + new decoded pixels, under ~190 KiB
for this pack, excluding allocator overhead. Format hard caps each file at
163,912 bytes; no unbounded allocation from external metadata. Decode is integer
unpacking only on load; no PNG/texture conversion in steady-state gameplay.
SD latency and actual heap consumption remain hardware measurements.

## Option and persistence

VIDEO SETTINGS adds only `AI ENHANCED WEAPON DESIGNS: OFF / ON`. No restart.
Archived existing-engine CVar `vid_ai_enhanced_weapons`, default false, is registered
with the normal CVar manager and written/read by the existing profile config.
No new config file, save-game field or gameplay setting. With no usable files for
the active game, getter forces OFF, label shows `[NO PACK]`, and selecting it
explains installation in the themed information dialog. A persisted ON is also
normalized OFF on startup without a pack. Partial packs may stay ON while a
missing weapon uses all original frames. Removing the last usable weapon forces
OFF when its failed load is observed. Installing during a live game is not
required; enable retry/menu rescan is provided, not continuous hot reloading.

## Validation

```sh
python3 tests/3ds_ai_weapons.py
python3 tests/3ds_weapon_bob_render.py
python3 tests/3ds_native_options.py
```

Host harness links the actual production loader/CRC and executes the extracted
production rasterizer with ASan/UBSan. Approved real pack covers 25 phases, AOG
SW/full ID mapping, 320/400 views, stereo on/off, seven oscillator positions,
NO WEAPON BOBBING, bottom coverage, per-eye transparency, unchanged world draw,
OFF restoration, missing/incorrect-edition/corrupt/truncated/bad-pixel/oversized
files, immutable cached phases and missing-weapon fallback. Preference getter and
setter bodies are exercised and archive registration/default checked. Original
bobbing/input/aspect regressions run separately. This is NOT a live save/load or
New 3DS hardware test.

Hardware checklist: cold OFF; toggle ON in pause/main menu; every firing phase
and held fire; all weapon switches; ON→OFF instant restoration; NO WEAPON BOBBING
both states while walking/stopping; slider 0/intermediate/full; 5:3 and 4:3;
save/load preserving original game phase, next level, exit/restart preference;
missing/corrupt pack startup and per-weapon fallback; PS original-only; visibility,
centering, lower cutoffs and SD-switch latency. No visual/functional approval is
claimed from compilation or host tests alone.

## Clean devkitARM build and local test artifacts

```sh
cmake -S . -B build/3ds-ai-weapons-clean \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-ai-weapons-clean --parallel 4
```

Clean build completed successfully. Executable:
`build/3ds-ai-weapons-clean/src/bstone/bstone.3dsx`.
Its 3DSX header/sections and ARM ELF were checked; hash and verification details
are in `build/3ds-ai-weapons-clean/verification.json`.
Local-only install ZIP: `build/ai-enhanced-weapons-test-kit.zip`, containing
`3ds/bstone/bstone.3dsx` and five `.bsw` files under `3ds/bstone/ai-weapons/aog/`,
without original game data. Merge with the existing SD directory; preserve its
data. Not published, committed or pushed.

The kit now includes the approved Previous Designs + Uniform Scale pack. Its
existing `.3dsx` is preserved byte-for-byte; no port rebuild or renderer change
was needed. A pack-only overlay is also available at
`build/previous-uniform-scale-stage/previous-designs-uniform-scale-pack.zip`.
Extract its `3ds/` directory into the SD root while the game is closed, keeping
existing game data, then enable AI ENHANCED WEAPON DESIGNS in VIDEO SETTINGS.
No physical SD was mounted during this update. Backup location, executable hash
and verification of all 25 decoded frames against the approved PNGs are recorded
in `build/previous-uniform-scale-stage/integration-verification.json`.

Files changed in THIS task, preserving all pre-existing changes:
- `src/bstone/CMakeLists.txt`: add optional resource-loader source.
- `3d_draw.cpp`: initialize edition-specific semantic mapping after preferences.
- `3d_scale.cpp`: alternate samples/dimension/anchor in the shared raster loop.
- `3d_menu.cpp`: one Video Settings option and unavailable-pack dialog.
- `id_vl.h/.cpp`: archived default-OFF preference and availability gate.
- `bstone_3ds_weapons.h/.cpp` (new): validated five-frame cache/loader.
- `tools/prepare_ai_weapons.py` (new): local offline conversion/layout overrides.
- `tests/3ds_ai_weapons.py` (new): production integration/malformed-pack tests.
- `tests/3ds_weapon_bob_render.py`: stub disabled optional resource provider in
  original-weapon regression; original assertions retained.
- This document. No files deleted, no original resources edited.
