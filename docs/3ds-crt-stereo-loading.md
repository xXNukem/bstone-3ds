# CRT, stereo interface and real loading presentation

## Scope

The approved borderless layout, fonts, text, controls, sound and save format
remain. This pass changes shared menu presentation, cooperative refresh waits,
and loading-display integration. It does not change the raycaster, weapon
scaler, weapon assets, world projection or gameplay stereo calculations. No
new texture, graphics library, worker thread, percentage or minimum loading
screen duration is introduced.

## Continuous CRT and cooperative waits

The old CRT used integer line positions and a 33-ms refresh request. More
importantly, `TicDelay`, button-release waits and volume arrow waits could run
without any presentation, freezing the effect while a direction/button was
held. Acknowledgement waits also needed pacing rather than busy polling.

`n3ds_menu::idle` now services a 16-ms visual deadline, sleeping in slices of at
most 4 ms before rechecking it. `TicDelay`, `WaitKeyUp`, volume debounce,
`CycleColors` and acknowledgement waits use it. It does not change their input
conditions, repeat counters, actions or tick calculations. The short sleep
bridges the existing engine tick timing to the UI deadline and yields CPU.
When the terminal is inactive it is a no-op. When MENU CRT is disabled there
are no animation-only refreshes, except the real loading indicator.

`begin_frame` snapshots `osGetTime` once for both eyes. The background uses:

- A fractional-position, 25-band soft sweep at 24 pixels/second; a smooth
  squared falloff fades it into/out of the display without a wrap flash.
- Thin alpha-blended scanlines, with a subpixel ambient drift.
- Very small combined sine brightness drift and smoothly interpolated,
  deterministic intensity noise (240-ms samples, smoothstep interpolation).
- Four low-alpha grain dots occasionally, rather than a full noise field.

The text and highlight colors do not flicker. Work is bounded and uses only the
existing solid-rectangle path. This is a requested deadline, not a claimed
hardware framerate; long synchronous operations can still prevent a frame.

The later [long-document reading-depth adjustment](3ds-reading-depth.md)
supersedes only the document body/header/help planes below. The approved
selectable-menu and loading stereo values are unchanged.

## Stereo menu layers

The existing left/right top targets and the same `C2D_SceneBegin` eye loop are
reused. The physical slider is sampled once per presented UI frame, independently
of the gameplay's `n3ds_stereo_active` state. Its value is clamped to 0..1;
values <=0.01 produce identical 2D views.

Offsets below are per eye at full slider. The right offset is the negative of
the left; intermediate slider values scale them linearly:

| Element | Left X | Right X | Intended plane |
| --- | --- | --- | --- |
| CRT background | -1.25 px | +1.25 px | Behind the screen |
| Headers, document/info texture and help | -0.35 px | +0.35 px | Moderate reading plane |
| Unselected options | 0 | 0 | Screen plane |
| Selected option/indicator and selected meter | +0.65 px | -0.65 px | Slightly in front |
| LOADING text | -0.35 px | +0.35 px | Comfortable reading plane |

The shared drawing routines apply these offsets with Citro2D's existing model
translation. The indexed document texture uses the same moderate-plane X
translation at its existing image draw. Every drawing group restores zero
translation before another layer or the bottom screen. Only the background
uses a 3-pixel overscan margin to avoid exposing screen edges. No inner border
is added, no eye-specific text/animation state exists, and no extra rendering
of the world or weapon is requested.

While any terminal/loading session is active the presenter skips gameplay
buffer conversion/drawing. The world and weapon algorithms remain untouched;
their existing branches resume when the session ends. The bottom title/HUD is
always 2D and retains its original drawing coordinates.

## Loading display and lifecycle

`begin_loading` / `end_loading` are nested main-thread scopes in the existing
adapter. The outermost begin retains an existing menu session, or owns a new
one when entered from gameplay. It submits a full frame immediately before
blocking load work. Nested calls do not restart the presentation. It never
clears/reformats save data or substitutes a different load algorithm.

Top screen: the shared dark CRT and centered, double-scale original-font
`LOADING...`, without headers, old UI pixels or debug output. Its horizontal
plane follows the slider as above.

Bottom screen: a 24-segment circular indicator centered at (160,114), radii
51..72 pixels. A smooth phosphor intensity peak revolves every 1.8 seconds,
using the same captured clock. The indicator is indeterminate and remains
animated even if MENU CRT is OFF. No static progress bar or invented percentage
is displayed. On leaving the loading scope the usual menu title or gameplay
HUD/automap/messages/EKG takes ownership again.

Actual integration points:

| Path | First frame / ownership | Cooperative checkpoints |
| --- | --- | --- |
| New game and normal level entry | GameLoop's real not-already-loaded branch, nested `LoadLevel` / `SetupGameLevel` | Map planes, outer map rows, object reconstruction and setup boundaries |
| Load a saved game / quick load | `LoadTheGame`, then nested `LoadLevel` / `SetupGameLevel` | HEAD/LVXX load/deserialization boundaries, 4-KiB temp-data copy and reconstruction loops |
| Level/mission changes | `LoadLevel` / `SetupGameLevel`, with GameLoop's loading phase | Same existing read/setup steps |

An already restored saved game does not receive a second artificial loading
screen just to rebuild the view. The old pre-load progress page in
`DrawLSAction(0)` is suppressed; the real reader owns the first loading frame.
Saving's presentation/logic remains unchanged. Ordinary menu transitions do
not instantiate a loading scope.

`loading_step` only presents if the visual deadline is due; it does not sleep
or poll/consume controls. All graphics remain on the main thread. A single
filesystem read, decompression or other atomic function cannot animate while
it blocks that thread; the last submitted loading frame remains visible and
the clock catches up at the next checkpoint. No worker or pretend progress
hides that limitation.

The last loading frame stays on screen until the existing destination code
publishes completed content. Exit invalidates the cached UI representation,
so transparent loading content cannot remain cached over the destination.
`Message` suspends the loading overlay before an error/acknowledgement screen;
its scope still unwinds normally. This prevents an unreadable load-error wait
behind LOADING. The outer scopes use existing scope guards across returns and
exceptions; GameLoop ends its loading phase before rebuilding the play screen.

## Files and verification

Changed this pass:

- `bstone_3ds_menu.cpp/.h`: CRT, idle scheduling, UI layers and loading lifecycle.
- `bstone_sw_video.cpp`: slider and per-eye UI integration; active terminal avoids
  reading partially reconstructed gameplay buffers.
- `3d_menu.cpp`, `id_in.cpp`, `3d_main.cpp`: existing wait loops service UI;
  load-scope/checkpoint integration and error handoff.
- `3d_game.cpp`, `id_ca.cpp`: real setup/map loading checkpoints and ownership.
- `tests/3ds_terminal_adapter.py`: timing, stereo and loading regression cases.
- This document; previous reports remain historical context.

Built after CRT, stereo and load integration, then in a new clean directory:

```sh
cmake -S . -B build/3ds-crt-stereo-loading \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-crt-stereo-loading --parallel 4
python3 tests/3ds_terminal_adapter.py
python3 tests/3ds_native_options.py
python3 tests/3ds_weapon_bob_render.py
```

Artifact: `build/3ds-crt-stereo-loading/src/bstone/bstone.3dsx`.

The actual adapter is exercised against mocked platform/draw boundaries:
clock-only refresh with bounded sleeps; existing layout and document bounds;
matching animation phases; exact stereo offsets at zero/half/full slider;
restored model translation; safe background overscan; first-frame submission;
nested versus owned loading lifetimes; changing 24-segment ring; CRT-OFF loading;
error suspension and return to menu/2D title. The unchanged controls/aspect and
weapon-scaler regressions also pass. These are not a whole-game load test.

Hardware validation is still required for smoothness while idle and holding
navigation, depth direction/comfort across the slider in all page types, actual
cold/quick/failed loads and level changes in both editions, audio continuity,
no transition flashes, indicator update gaps during long I/O, and exact HUD/
automap restoration. Compilation alone does not validate those visual results.
No commits, merges, pushes or unrelated features were made.
