# 3DS terminal interface

## Implemented coverage

The existing controllers (`CP_iteminfo`, `CP_itemtype`, `HandleMenu`), actions,
cache ownership and original resource licenses remain in place. The shared
`bstone_3ds_menu` adapter now provides the full presentation, not just headers:

- Main/pause, mission/episode selection and difficulty.
- Options, Video Settings, Sound Settings, Sound Volume, Control Settings,
  Switches and Switches 2.
- Load/save lists, overwrite/quit/end-game confirmations, progress and errors
  reached through the control panel; standalone `Confirm` owns its session.
- Instructions / CLASSIC controls, History, mission briefings/debriefings,
  ordering information and credits via the existing text presenter.
- High scores from the menu, attract loop and post-game name-entry controller.
- Level-loading mission-status text during the loading phase.

The original cinematic presentations, movies, loss-scene artwork, gameplay
messages, stats/automap overlays and lower gameplay HUD remain outside this
interface migration. They are game scenes or gameplay overlays; their artwork,
raycaster, audio and stereo calculations have not been redesigned. The legacy
PC binding editor and mouse panel are no longer reachable from Control Settings.

## Common presentation and transitions

The native canvas is 400 x 240, with the shared header at native y=12 and
right-aligned ONLINE/PAUSED status on the same baseline. All lists measure their actual original font,
center each label and derive selection bounds from that label. Blank controller
slots do not consume visual rows or draw stripes. The green line formerly
below LOGOFF was the explicit footer divider at y=218; it has been removed.
The help baseline is the final visible row plus 20 pixels, so it follows
both main and pause layouts rather than sitting at a fixed bottom coordinate.
Lists are centered vertically in the expanded native body: the main/pause and
save lists have 16-pixel row spacing; shorter lists have 20-pixel spacing.
For nine main rows LOGOFF is at y=182 and help at y=202; for ten pause rows
they are at y=190 and y=210. Proportional horizontal centering is unchanged.
Information panels use the same enlarged logical body y=28..179; wrapped modal
messages are centered near logical y=104. Native volume meters were moved up
and given 60-pixel spacing.
The terminal is now borderless: the 400 x 240 physical screen is the monitor.
There are no inset outlines, corner marks, header bands or divider lines.
The CRT background covers the entire display. Selection accents and functional
volume tracks remain, since they communicate state rather than frame content.

Headers are committed through `DrawMenuTitle` / `publish_destination` with the
new screen, using MAIN TERMINAL, NEW GAME, DIFFICULTY, LOAD GAME, SAVE GAME,
OPTIONS, VIDEO SETTINGS, SOUND SETTINGS, SOUND VOLUME, CONTROL SETTINGS,
INSTRUCTIONS, HISTORY, MISSION BRIEFING, HIGH SCORES and the equivalent section
name. The title of the game appears only on the lower screen during menus.

Decorative `BACKGROUND_SCREENPIC`, old bevel boxes, title/footer glyphs and
old menu icons no longer supply the visible menu composition. Native lists and
meters read the current controller state. The indexed UI remains the content
surface for longer text, text input and embedded narrative art. It becomes
transparent around content; a small ink classification mask colors glyphs
phosphor green while keeping original illustration pixels and palette colors.
Text erasure clears transparency instead of painting green over old input.
The existing cached GPU UI texture is reused, including invalidation on scroll,
ink changes and session entry/exit. No new texture or renderer is introduced.

The common animated background is drawn before that texture, so opaque old
background pixels cannot cover the CRT effect. The foreground supplies headers,
selection and contextual help. Destination publication restores the palette
left flat by legacy fades before exposing the destination; no black-frame wait
or legacy main-menu frame is introduced. The Citro2D object pool has capacity
for long save names in both UI views plus the bottom title; Citro3D setup,
shaders and stereoscopic calculations are unchanged.

The later [CRT, menu stereo and loading pass](3ds-crt-stereo-loading.md)
supersedes the refresh and stereo details below; the approved borderless layout
and reading viewport remain unchanged.

## Ambient CRT

The initial animation was being updated, but its old green range (roughly
18..24) and three-line sweep were too faint. The shared background now has a
21-line soft sweep peaking at green 40, moving through the full screen in about
9.2 seconds, dynamic scanline luminance, a smooth six-level phosphor drift and
six sparse low-contrast grain dots. Text colors remain stable. Work is bounded
and uses existing solid rectangles, without textures, threads or a random
number generator.

`CycleColors` requests a UI presentation after at least 33 ms even if no palette
range changed. The volume loop and acknowledgement waits also keep updating;
this fixes idle screens that otherwise only refreshed after input. The clock
is sampled once per presented frame for matching phases in both UI views.
Outside a terminal session these extra refresh requests are disabled.

## Narrative pages and CLASSIC instructions

History and briefings keep the original scripts, page order, text, original
fonts and embedded illustrations. The existing `TP_Presenter` renders each
original page into its logical 320 x 200 content surface. A terminal document
viewport maps source y=8..175 into logical destination y=28..179, approximately
native y=34..215. This gives 152 logical lines (about 182 native pixels), up
from 119 (about 143 pixels): roughly 28% more reading height without reducing
font size. Original x=8..311 maps to approximately native x=10..389, with no
inset contour crossing those glyphs. Up/down scroll the remaining 16 logical
lines, down from 49; left/right retain previous/next-page handling. The page
counter moved to the header at native (12,12); help is centered at y=225.
Neither is drawn inside the reading viewport. The same geometry applies to
Instructions/CLASSIC, History, briefings, debriefings, ordering and credits;
text, original line wrapping, page order and illustrations are unchanged.
The last original line is reachable; the source page is not truncated to the
height of the terminal viewport. A continues where the original briefing
requires it; B exits/cancels without being mistaken for its gameplay SPACE alias.

Only Instructions is replacement instructional content. It documents the
fixed HID mapping in `id_in.cpp`: D-pad movement/turning, L/R strafe, A fire,
B use, ZL/ZR next/previous weapon, X automap and START pause. It also explains
menu controls, document navigation, ALWAYS RUN and START+SELECT emergency exit.
It uses the same presenter and two script pages. No narrative resource is
rewritten. Control Settings opens this CLASSIC documentation; Modern is not
implemented or shown as selectable. Desktop Mouse and Customize Controls do
not configure this fixed HID path and are removed from the visible console UI.

## Options audit

The functional cleanup supersedes the earlier interim option audit. See
[Native options and debug-HUD cleanup](3ds-native-options.md) for the final
visible options, Classic scheme preparation, every switch, both-edition weapon
bobbing, removed settings, tests and the latest executable path. The borderless
layout and document viewport described here remain unchanged.

## Lower-screen lifetime

A terminal session owns the existing edition-aware intro title presentation.
Main-to-submenu transitions do not end that session. Narrative and standalone
confirmation/high-score sessions use scope guards; loading ends its session
before rebuilding the gameplay screen. During a session the bottom presenter
always chooses the title rather than a residual HUD, even for quick dialogs.
Session cleanup returns ownership to the existing HUD/automap/messages/EKG
branch without modifying their data or renderers. The intro title resource,
font cache, colors and bottom-screen bezel are shared; no title is duplicated
above. The border removal applies only to the top terminal interface; the
approved bottom-screen title and its presentation are unchanged.

## Verification

The borderless pass changes only the shared adapter, the native modal-message
vertical positioning, regression checks and this document. The bottom title,
session ownership, input, gameplay HUD, audio and stereo code are unchanged.
Final build started in the new, empty `build/3ds-terminal-borderless` directory:

```sh
cmake -S . -B build/3ds-terminal-borderless \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-terminal-borderless --parallel 4
python3 tests/3ds_terminal_adapter.py
```

Artifact: `build/3ds-terminal-borderless/src/bstone/bstone.3dsx`.

The host regression compiles the actual adapter against mocked platform/engine
boundaries. It checks proportional centering, row spacing, footer placement,
absence of an inset contour/corners/header dividers, animated frame changes and matching eye phases,
header mappings and handoffs, palette restoration, green ink versus original
art colors, the full 152-line viewport and transparent boundaries, access to
the final narrative line by scrolling, page changes,
volume display, both edition titles and session cleanup. These checks passed;
`git diff --check` passed. They do not execute the full game navigation.

Definitive visual, performance and functional validation remains on a real New
Nintendo 3DS for both editions: nested menu returns, all save slots and dialogs,
SFX carousel, volumes, every switch, full narrative page traversal, idle CRT,
credits/high-score entry, loading and repeated gameplay resume with automap,
messages and EKG. Console text entry for high scores remains the pre-existing
input path; a dedicated console naming UI is a separate functional improvement.

No commits, merges or pushes were made.
