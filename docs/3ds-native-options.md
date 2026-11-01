# Native options and debug-HUD cleanup

This pass keeps the approved borderless terminal presentation and the menu
controllers. It changes only the requested functional options and weapon bounce.
No commits, merges or pushes are made.

## Visible options

### Video Settings

- **VIEW ASPECT: 5:3 NATIVE / 4:3** (`vid_is_widescreen`, archived). Native
  mode renders the existing 400-wide view across the whole 400 x 240 panel.
  The original 4:3 configuration produces a 320 x 240 logical view. Previously
  that image was stretched back to 400 x 240; it is now presented at x=40 with
  width=320, preserving 4:3 and leaving equal side bars. The same rectangle is
  used for both eyes. Existing reconstruction calls are retained; no stereo
  projection, eye offset, raycasting or depth calculations are changed. Menus
  remain 400 x 240 regardless of the gameplay aspect.
- **MENU CRT: ON / OFF** (`vid_menu_crt`, archived, default ON). Controls the
  shared top-menu scanlines, grain and ambient sweep. OFF keeps the dark
  background, green text and selection, and stops animation-only refresh
  requests. It does not change the gameplay CRT or lower-screen title.

`vid_is_ui_stretched`, its getter/setter, menu case, registration and startup
argument are removed. The 3DS UI has an explicit native presentation and never
consumed that CVar. No PC resolution, window, GL/Vulkan or stereo setting added.

### Sound Settings

The four retained rows use their existing operations:

| Row | Configuration | Consumer |
| --- | --- | --- |
| SOUND EFFECTS | `snd_is_sfx_enabled` | `sd_enable_sound`, effects voice group |
| SFX TYPE: ADLIB / PC SPEAKER | `snd_sfx_type` | `sd_apply_sfx_type`, audio content manager's original synthesized assets |
| DIGITIZED SFX | `snd_is_sfx_digitized` | `apply_digitized_sfx`, content manager's digitized preference |
| BACKGROUND MUSIC | `snd_is_music_enabled` | `sd_enable_music`, existing music playback |

AdLib and PC Speaker here are real synthesis modes, not unavailable devices.
SOUND VOLUME retains its approved controls, presentation and SFX/music setters.
The desktop driver and OPL implementation selectors, callbacks and dead menu
state are deleted. The unused public OPL selection wrappers are removed, but
the internal OPL CVar/decoder selection is retained because it has a real
consumer; synthesis implementation and mixer code have not been changed.

`snd_driver` and its unused public selection API are deleted. The factory sends
both `auto_detect` and `system` to the exact same `SystemAudioMixer`; startup now
passes `system` directly, preserving that path. Mix size, rate, voice groups,
original assets and decoders are unchanged.

### Control Settings

- **CONTROL SCHEME: CLASSIC**, the only implemented/selectable scheme.
- **CLASSIC BUTTON GUIDE**, using the existing console instructions presenter.

`InControlScheme` and archived integer CVar `in_3ds_control_scheme` retain the
choice through the existing config writer/reader. The allowed range derives
from the implemented scheme count. A table describes each scheme's buttons,
scan codes, held/edge behavior and consumed weapon-key behavior; one input loop
applies the selected table. A static assertion requires a table for every
selectable scheme. Adding Modern later requires its actual enum entry, table,
menu entry and instructions, without copying the input loop. There is no Modern
mapping or selectable placeholder now.

CLASSIC preserves D-pad movement/turning, A fire, B use, L/R strafe, ZL/ZR next/
previous weapon, X stats map and START pause. Menu A/B navigation and the
START+SELECT emergency-exit condition are retained. Keyboard bindings remain
because the engine consumes the translated scan codes through them and old
configs may contain legitimate bindings; save-game structures are unchanged.

The desktop mouse panel, sensitivity screens, binding-editor presentation,
mouse enable/sensitivity CVars and gameplay mouse polling are removed. The
existing SDL N3DS source was inspected: Circle Pad/C-stick generate joystick
axes, which this port does not consume as mouse input, and touch-to-mouse
emulation requires an SDL window; the port creates none. Generic SDL event and
acknowledgement infrastructure remains for console/menu use. No physical button
assignment was changed.

## Every switch audited

All referenced configurations remain archived by the existing CVar system.
Edition-specific rows retain their original availability, except bobbing is now
implemented for both editions and PS map-in-stats is hidden in AOG.

| Original/current row | Configuration and effective consumer | Result |
| --- | --- | --- |
| LIGHTING | inverse `gp_no_shading`; wall/sprite/plane shading in `3d_draw.cpp` | Retained |
| REBA ATTACK INFO | inverse `gp_hide_attacker_info`; `TakeDamage` -> `DisplayInfoMsg` -> native message capture | Retained; attacker messages have an active native consumer |
| SHOW CEILINGS -> TEXTURED CEILING | inverse `gp_is_ceiling_solid`; `DrawPlanes`/world rendering | Retained; label now distinguishes textured from solid ceilings |
| SHOW FLOORS -> TEXTURED FLOOR | inverse `gp_is_flooring_solid`; `DrawPlanes`/world rendering | Retained; label now distinguishes textured from solid floors |
| NO WALL HIT SOUND | `gp_no_wall_hit_sfx`; collision/wall sound path in `3d_agent.cpp` | Retained |
| ALWAYS RUN | `gp_is_always_run`; `PollKeyboardMove`, running movement/strafe scale | Retained |
| QUIT ON ESCAPE | `gp_quit_on_escape`; `US_ControlPanel` return-to-game versus confirmation | Retained; console menu B uses the same exit branch |
| HEART BEAT SOUND | `gp_use_heart_beat_sfx`; `DrawHealthMonitor` -> original `H_BEATSND` | Retained for AOG; already absent from PS's switch list |
| ROTATED AUTOMAP -> STATS MAP: FULL/LOCAL | `am_rotatable`; `PopupAutoMap` changes `OV_WHOLE_MAP` | Retained for AOG with truthful label; does not rotate the permanent lower automap |
| SKIP INTRO/OUTRO -> SKIP TITLE / EXIT PAGES | `gp_no_intro_outro`; attract/title loop, shareware exit promo | Retained; redundant duplicate condition removed. Does not claim to skip every narrative movie |
| SKIP FADE -> SKIP GAME FADES | `gp_no_fade_in_or_out`; existing palette fade paths | Retained; native terminal transitions remain flash-free |
| NO WEAPON BOBBING | `gp_no_weapon_bobbing`; `DrawPlayerWeapon` and common weapon scaler | Fixed and selectable in both editions |
| VANILLA FIZZLE FX | `gp_vanilla_fizzle_fx`; `GenericFizzleFX`, `PsFizzleFX` initialization | Retained; classic versus alternate transition appearance |
| [PS] MAP IN STATS | `gp_ps_map_in_stats`; PS `PopupAutoMap` optional map | Retained in PS; hidden/nonselectable in AOG, compacted automatically |

No switch merely flips a disconnected local display value. The lower native
map and EKG visual design are unchanged; the table distinguishes their state
from legacy stats-map functions.

## Weapon bobbing diagnosis and fix

The engine still calculates `bounceOffset` in `HandleWeaponBounce` and the
scaler still applies it to the weapon before copying visible pixels into both
stereoscopic buffers. The effect was disabled for AOG by the explicit edition
condition in `DrawPlayerWeapon`, not by missing stereo offsets.

That condition now depends only on `!gp_no_weapon_bobbing()`. A subsequent
hardware correction replaces signed vertical travel with subtle down-only
dips and horizontal sway: see [Weapon bobbing edge fix](3ds-weapon-bob-fix.md).
Rest and disabled bobbing both yield zero movement offsets. Lateral walking
also keeps the bounce active. The
existing sprite dimensions, clipping and zero-parallax copy to both eyes are
unchanged, with no duplicate weapon render and no new eye-specific displacement.

## Debug HUD cleanup

The HUD-local FPS counter and the presenter's W/PIX/FL/UI/DRAW overlay were
independent. Both drawing paths, text buffers, sampling timestamps and
accumulators have been removed. The underlying HUD is redrawn normally, exposing
its existing ECG/status area and the automap tiles previously covered by the
profiler. No empty panel replaces them. `CalcTics`, `TimeCount`, frame begin/end,
vsync and the audio CPU-yield timing remain. The menu title screen is unchanged.

## Files and verification

Changed in this pass: `bstone_sw_video.cpp`, `bstone_3ds_hud.cpp`,
`bstone_3ds_menu.cpp`, `id_vl.cpp/.h`, `3d_menu.cpp`, `id_in.cpp/.h`,
`id_sd.cpp/.h`, `3d_main.cpp`, `3d_play.cpp`, `3d_agent.cpp`, `3d_draw.cpp`,
`3d_scale.cpp`, `3d_def.h`; regression tests and these documents. No entire source
file was deleted. Retired functions, declarations, data and CVars were removed
in-place. Old config keys are ignored/logged per line by the existing parser;
they do not abort the rest of the file and are not archived on the next write.

Subsystem builds passed after HUD, video, controls/sound and switch changes.
A new clean Release build uses devkitARM GCC 16.1.0:

```sh
cmake -S . -B build/3ds-native-options \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-native-options --parallel 4
python3 tests/3ds_native_options.py
python3 tests/3ds_terminal_adapter.py
```

Artifact: `build/3ds-native-options/src/bstone/bstone.3dsx`.

Host tests compile actual extracted production input, bounce and image-placement
code with mock platform/game state. They exercise 2048 Classic combinations,
held/edge/consumed buttons, START+SELECT, movement and rest, both edition gates,
bobbing ON/OFF, bounded positive/negative offsets, non-weapon exclusion, and the
same 5:3/4:3 destination for each eye. The adapter regression also checks CRT
ON/OFF, reading viewport, centering, transitions, palette restoration and title
lifetime. These are not an end-to-end console run.

Hardware checks required: both aspects at stereo slider zero and nonzero; all
weapons during forward/back/strafe walking and rest in AOG/PS; bobbing toggle
without a resting position jump or eye mismatch; each switch at its documented
trigger; real synthesis/digitized/music changes and unchanged volumes; restart
persistence and old-config migration; same Classic feel, A/B menu returns,
save/load compatibility; absence of all debug text and restoration of lower
HUD/map/messages/EKG. No new unrelated feature should be added before that test.
