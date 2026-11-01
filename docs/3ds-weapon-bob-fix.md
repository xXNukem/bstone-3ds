# Weapon bobbing: keep the lower sprite edge outside the view

Only the weapon offset and its projected lower-bound rounding change in this
pass (`3d_scale.cpp`). Original sprites, oscillator, movement controls, switches,
menus, audio, raycasting and stereo projection/copy logic are unchanged.

## Cause and coordinate conventions

The original scaler centers the 64 x 64 weapon canvas horizontally and anchors
it vertically at `vga_3d_view_bottom_y - height`. Positive screen Y points down.
`HandleWeaponBounce` supplies the existing inertial oscillator (`bounceOffset`,
neutral 6, targets 2/10, hard limits 0/12) and recovers toward 6 when movement
stops. Our previous repair enabled this for AOG as well as PS, and replaced the
original down-biased `12 - bounceOffset` placement with signed
`6 - bounceOffset`. That signed offset lifts the weapon on half of the cycle,
exposing the unfinished lower edge of source artwork intended to meet the
bottom of the screen. Horizontal placement previously had no sway.

Transparency still works as intended: negative source pixels are skipped. It
cannot supply missing artwork when an offset reveals an image's lower limit.

## Corrected motion

Let `s = clamp(bounceOffset - 6, -6, 6)` when bobbing is enabled, otherwise zero:

- X offset: `round(vga_width / 320 * s * 0.25)`.
- Y offset: `round(vga_height / 200 * abs(s) * 0.5)`.

At 240 pixels high this means down-only travel of roughly 2 pixels over the
usual target range, bounded at 4 pixels including oscillator overshoot. X sway
is bounded at 2 pixels in either 320- or 400-wide view modes. The vertical
absolute value yields two downward footfalls per left/right sway cycle. The
existing movement oscillator retains cadence, acceleration and stop recovery;
no independent timer or eye-specific state is introduced. Neither offset
applies to world sprites. At rest and with NO WEAPON BOBBING enabled the offsets
are exactly zero.

The weapon never moves above its original anchor, so motion cannot expose any
additional lower rows compared with rest. We also avoid a separate integer
rounding seam: if `top + sprite_height == 64`, its projected bottom is computed
as `offset_y + height`, instead of adding two independently truncated scaled
parts. This eliminates a possible one-pixel gap without adding source pixels.
The existing lower-view clipping removes the overhanging part as the weapon dips.

## Stereo and resource limits

The same X/Y anchor is computed before rasterization. The existing optimized
scaler then writes each visible weapon pixel to the second eye with the same
coordinates, retaining its original zero-parallax weapon presentation. Negative
sprite samples still leave each eye's own background untouched. No world-eye
projection difference, weapon size or stereo-depth setting is changed.

No resource was rebuilt or generated. If an individual frame already ends
above its intended canvas bottom at rest, down-only motion cannot reconstruct
that missing art; its lower edge will never be raised farther by this fix.
An asset-specific downward anchor adjustment could be evaluated after inspecting
that frame, before considering reconstruction. There is currently no evidence
from asset inspection that reconstruction is necessary. Check all real weapons
and firing frames on hardware; synthetic geometry tests cannot establish their
artwork completeness.

## Verification

`tests/3ds_weapon_bob_render.py` compiles and runs the actual production scaler
against two synthetic framebuffers, with AddressSanitizer and UBSan. It covers
both 88/128 weapon scales, 320/400 view widths, opaque and transparent patterns,
97 oscillator positions, bottom-reaching and short-bottom resources, identical
weapon coverage in both eyes with different preserved backgrounds, horizontal
bounds, exact disabled/rest stability and unchanged world-sprite placement.
All checks pass. The native-options regression also runs the actual original
oscillator and edition gate, with its Y expectation updated to nonnegative.

```sh
python3 tests/3ds_weapon_bob_render.py
python3 tests/3ds_native_options.py
cmake -S . -B build/3ds-weapon-bob-fix \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-weapon-bob-fix --parallel 4
```

Output: `build/3ds-weapon-bob-fix/src/bstone/bstone.3dsx`.
Visual approval remains on real hardware: forward/back/strafe, start/stop,
all firing frames, both editions, both view aspects, slider off/on, and the
NO WEAPON BOBBING switch. No commits were created.
