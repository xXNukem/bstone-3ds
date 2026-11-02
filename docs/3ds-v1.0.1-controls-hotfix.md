# v1.0.1 controls and button guide hotfix candidate

## Base and scope

Work began with a clean `master` at `e6caa9a8`. The local `v1.0.0` tag is
upstream BStone's 2015 commit `08808be659e90a3721e053ab7040ca596e96ae37`,
not the 3DS release. Branch `hotfix/v1.0.1-controls` starts at
`b32b818b8f9a69f0d26703307a46880b0bb33045` (Prepare Blake Stone 3DS v1.0.0).
The two later master commits only change README.md and are excluded.
No commits, merges, tags, pushes or changes to published artifacts were made.

No gameplay renderer, stereo calculations, bobbing, audio, automap, HUD,
launcher, SHA-1 validation, save format, loading screen, icon or metadata changes.
Experimental weapons remain OFF. Existing engine version/metadata are retained.

## Input fault and correction

`PlayLoop` calls `PollControls` before setting `vid_is_hud=true` for rendering;
it sets that flag false again before `CheckKeys`. Both `in_handle_events` and
`in_get_3ds_motion` incorrectly used that rendering flag as a gameplay flag.
Consequently the former selected Classic and the latter suppressed analog
movement during ordinary gameplay, despite the archived scheme being Modern.

Both paths now use `ingame` and the native terminal's active state. Menus keep
Classic navigation without changing the stored preference. Switching effective
layouts clears all scan codes owned by the old layouts. Scheme selection also
clears keyboard state. START/SELECT emergency exit stays in the event poller.

SDL's controller button handler independently converted A/START into Enter,
B into Escape, and D-Pad into arrows. These events bypassed the scheme and
could leak legacy menu/turn actions into gameplay. Native controller button,
axis and joystick events are now consumed without conversion. SDL still pumps
events and handles keyboard, quit and window events. HID is the single owner
of console button mapping; native menu-only Enter/Escape and arrow aliases
preserve legacy text-entry/presenter consumers. `ReadAnyControl` continues to
read A/B/directions directly for terminal menus.

SDL initializes HID. The installed libctru archive's `hid.o` references
`irrstInit`, `irrstScanInput`, `irrstKeysHeld` and `irrstExit`; the existing
`hidScanInput`/`hidCstickRead` route is retained without a second service owner.

Classic's binding table is unchanged. Modern uses Circle Pad Y for forward,
X for strafe, Y/A for left/right turn, R for attack, B for use, digital
D-Pad right/left for next/previous weapon, X for automap and START for menu.
C-Stick adds only horizontal turning in Modern + C-Stick. The existing axis
processing uses Circle deadzone 20, C-Stick deadzone 15, proportional rescaling
to 1000, normalized Circle diagonals, engine walk/run/tic scaling and a bounded
combined A/Y/C-Stick turn rate. Circle digital directions never select weapons
in either Modern layout. No vertical freelook is introduced.

## Configuration and editions

`in_3ds_control_scheme` remains an archived CVar, default 0 (Classic).
CVar string deserialization does not clamp integers to the declared range.
The getter now validates before any layout indexing and returns Classic for
invalid integers. The setter also validates. `ReadConfig` normalizes the
archived value and restores the native default action bindings after parsing;
old `in_bind` commands cannot reinterpret HID's generated scan codes or retain
mouse/secondary bindings that conflict with fixed console layouts.

Existing config IO still uses the selected edition's profile. AOG full,
AOG shareware and PS keep their existing separate directories and original
legacy-read fallback policy. No profile path or migration change was made.
Both editions share input code. B reaches the original `bt_use` action;
Planet Strike's interaction/detonator logic in `3d_agent.cpp` is unchanged.

## Presenter faults and correction

CONTROL SETTINGS calls `CP_Control`, whose BUTTON GUIDE branch calls
`HelpPresenter(nullptr, false, HELPTEXT, false, "CONTROL GUIDE")`.
The generated, writable vector remains alive throughout `TP_Presenter` and
does not use the cached-script flag or free a nonexistent HELPTEXT resource.

Two concrete faults were reproduced against the release source:

1. The presenter assumes `grsegs[STARTFONT + fontnumber]` is populated, then
   dereferences `font->width` when processing the guide's initial `^CE` title.
   Native terminal drawing retains separate font copies; it does not guarantee
   that original resource slot stays cached. For example, `Confirm` frees its
   font on exit and native menu title drawing can return without caching it.
   An empty slot produces a null font. Presenter startup now explicitly caches
   its selected original font before measuring or rendering the first command.
2. `TP_InitScript`, `TP_CachePage` and `TP_HandleCodes` read command letters via
   `uint16_t*` casts. Commands occupy arbitrary byte offsets (the first command
   after `^` already has an odd address for an aligned allocation). These are
   unaligned C++ accesses, detected by UBSan and unsafe to assume on ARM.
   All four reads now decode the same little-endian code from unsigned bytes.

The host regression test reproduces the old misaligned parser access and the
old presenter startup's null-font boundary separately. It then exercises the
fixed parser, font acquisition and actual end-of-page navigation. Resource
IO, rendering and HID are mocked boundaries. Without the console exception
dump we cannot establish which fault caused that particular ARM11 exception,
or claim that every hardware failure is resolved solely from these tests.

The existing terminal presenter is retained: green phosphor, no additional
frame, existing comfortable document depth and lower screen. HELPTEXT is
generated from the selected scheme for both BUTTON GUIDE and INSTRUCTIONS.
The first page describes gameplay, including C-Stick only for that scheme;
the second describes terminal/automap controls. Classic text explicitly covers
Circle Pad and D-Pad. Font size is unchanged. Original history/briefing/order
scripts retain their content and resource ownership and receive the shared
font/alignment fixes.

## Tests

Passing host suites:

- `tests/3ds_modern_controls.py`: actual mapping, actual default bindings and
  action dispatch for AOG/PS, real CVar archive string roundtrip and invalid
  values, movement/tic/run scaling, 130321 analog positions, drift/diagonal
  bounds, held weapon consumption, menu aliases and scheme changes. HUD starts
  false as it does in the gameplay loop.
- `tests/3ds_button_guide.py`: production guide strings and page parser at both
  alignments, presenter font acquisition and actual end-page next/previous/B
  logic across 300 reopen cycles. Rendering the next page is a mock boundary.
- `tests/3ds_input_pipeline.py`: production SDL poll filtering and emergency
  chord, keyboard/quit event retention, actual ReadConfig finalization with
  two simulated profile IO fixtures.
- Existing `3ds_native_options.py`, `3ds_menu_switch_input.py`,
  `3ds_terminal_adapter.py`, `3ds_display_lifecycle.py`,
  `3ds_edition_initialization.py`, `3ds_weapon_audio_route.py` and
  `3ds_weapon_bob_render.py`.

The new tests use ASan/UBSan. Both `3ds_modern_controls.py --baseline` and
`3ds_button_guide.py --baseline` verify failures using `git show b32b818b`.
They do not modify the worktree. The standalone launcher test requires an
external shareware directory/ZIP and was not run successfully in this task;
its no-argument invocation stops at argument validation. No retail AOG/PS
game-data or real console test is claimed. Experimental asset tests are outside
this hotfix.

## Clean build and diagnostics

Configure with the real 3DS SDL2 installation, then compile from scratch:

```sh
cmake -S . -B build/3ds-v1.0.1-hotfix \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release -DBSTONE_ENABLE_AI_WEAPONS=OFF \
  -DCMAKE_CXX_FLAGS_RELEASE='-O3 -DNDEBUG -g' \
  -DCMAKE_C_FLAGS_RELEASE='-O3 -DNDEBUG -g'
cmake --build build/3ds-v1.0.1-hotfix --clean-first --parallel 4
python3 tools/verify_3ds_metadata.py build/3ds-v1.0.1-hotfix/src/bstone --stable
```

Artifacts relative to this repository:

- `build/3ds-v1.0.1-hotfix/src/bstone/bstone.3dsx`
- `build/3ds-v1.0.1-hotfix/src/bstone/bstone.elf`

The ELF is the matching unstripped Release executable with DWARF symbols,
not a separate differently optimized Debug build. Keep it with the tested
3DSX. Final clean build and stable metadata verification passed. DWARF
`.debug_info`, `.debug_line` and the symbol table are present; addr2line was
checked against TP_Presenter, TP_InitScript and in_handle_events.
The complete host test output is `build/3ds-v1.0.1-hotfix/host-tests.log`.

Final artifact SHA-256:

```text
bstone.3dsx  4c790b88342fed912b41f89eb746de6c0c6127ee52baae009da510622e6dd82f
bstone.elf   a1aab96fe623b7f94c59eb20bb05f0df6cdb46c8955ddf4e69e472cd0fc31f5b
```

Resolve exception PC/LR/stack code addresses with:

```sh
/opt/devkitpro/devkitARM/bin/arm-none-eabi-addr2line -a -f -C -i \
  -e build/3ds-v1.0.1-hotfix/src/bstone/bstone.elf 0xADDRESS
```

## Required New 3DS XL validation before publication

For both AOG and PS: test all specified buttons in all three layouts, diagonal
walk/run, simultaneous fire/use/move/turn, Circle weapon isolation, C-Stick
deadzone and combined A/Y turning. Check PS interaction/detonator behavior.
Switch layouts with buttons held, return to gameplay, save preferences and
restart each edition independently. Try an invalid scheme in a backup config.

Open/close BUTTON GUIDE and INSTRUCTIONS repeatedly from the title and paused
game with all three layouts; visit every page, scroll, return with B and check
that held input does not activate the parent. Repeat other help documents.
Check actual font wrapping/line height, legibility, both eyes at slider minimum
and maximum, lower screen, A/B confirmations and save-name text entry.
Verify START and START+SELECT. If an exception remains, preserve the dump and
the matching ELF/3DSX; resolve the actual addresses before further changes.
