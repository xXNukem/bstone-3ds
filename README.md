# BStone 3DS

**A native dual-screen port of Blake Stone: Aliens of Gold for New Nintendo 3DS.**

BStone 3DS is an unofficial homebrew adaptation of the 1993 game, based on
[BStone](https://github.com/bibendovsky/bstone) by Boris I. Bendovsky and its
contributors. The Nintendo 3DS port is developed independently by **xXNukem**.

The project targets New Nintendo 3DS, New Nintendo 3DS XL and New Nintendo 2DS XL.
The stereoscopic effect requires a model with a 3D display. The working baseline
has been tested on real New Nintendo 3DS hardware. Every newly built cleanup
version still needs a hardware check before it is treated as validated.

## Architecture

- The original software raycaster renders the scene, sprites and player weapon.
- Citro3D/Citro2D present the full-screen 400×240 upper image, including separate
  left/right-eye buffers controlled by the 3D slider.
- The lower screen has a permanent automap, player marker, animated ECG, health,
  ammunition, weapons, score, lives, keys, radar and gameplay messages.
- SDL2 supplies audio devices, controller/keyboard event translation and the N3DS
  video event pump. Its N3DS backend owns `gfxInit`/`gfxExit` and HID/APT polling.
  The game does not create SDL windows, renderers or textures. The native
  presenter releases its GPU resources before SDL shuts down graphics.
- libctru HID supplies gameplay actions and the emergency exit chord. SDL
  GameController navigation and the existing native menu input remain in use.
- The system mixer, DOSBox DBOPL and Nuked OPL3 reproduce the original AdLib,
  PCM and PC Speaker audio. Memory, timing and concurrency utilities remain
  where the engine needs them.
- Original data, saves, configuration, scores and screenshots use the SD card
  through newlib/POSIX file access. Resource decompression and validation remain
  in the engine.

Only widescreen and UI stretching are exposed as presentation settings. Desktop
renderers, window-mode/resolution selection, Steam/GOG installation detection,
asset export commands, xBRZ, desktop tools and the unsupported Vita port have
been removed. SDL2 is an external static Nintendo 3DS dependency.

## Installation and game data

You need a homebrew-capable New Nintendo system, the Homebrew Launcher and your
own original game data. Game assets are not distributed with this project.

Create `/3ds/bstone/` on the microSD and put `bstone.3dsx` and the original full
Aliens of Gold `.BS6` files there:

```text
sdmc:/3ds/bstone/
    bstone.3dsx
    AUDIOHED.BS6
    AUDIOT.BS6
    EANIM.BS6
    GANIM.BS6
    IANIM.BS6
    MAPHEAD.BS6
    MAPTEMP.BS6
    SANIM.BS6
    VGADICT.BS6
    VGAGRAPH.BS6
    VGAHEAD.BS6
    VSWAP.BS6
```

Launch through the Homebrew Launcher. Existing installations, saves and user
configuration continue to use `sdmc:/3ds/bstone/`; no migration is required.
The content search retains the configured data directory, working directory
and profile directory. Old desktop configuration keys are reported as unknown
and skipped, while retained settings continue to load.

Aliens of Gold is the tested default launch mode. The original Planet Strike
loaders and game logic are preserved, but Planet Strike remains untested on
hardware. The current launch arguments in `3d_main.cpp` select `--aog`; selecting
Planet Strike is still a source-level launch setting.

## Controls

| Control | Action |
|---|---|
| Circle Pad / D-Pad Up and Down | Move forward / backward |
| Circle Pad / D-Pad Left and Right | Turn left / right |
| L / R | Strafe left / right |
| A | Fire / Confirm |
| B | Interact, open doors / Cancel |
| ZL / ZR | Next / previous weapon |
| START | Open the game menu |
| START + SELECT | Emergency exit |

The lower automap is always visible during gameplay. Existing menus, save/load,
introductions, movies and transitions use the current console controls.
Touchscreen interaction and C-Stick camera turning are not implemented; neither
is needed to play.

## Build from a clean directory

### Dependencies

- CMake 3.18 or newer and a host build tool such as Make or Ninja.
- devkitPro's devkitARM toolchain and Nintendo 3DS tools (`3dsxtool`, and `3dslink`
  for network testing).
- libctru, Citro2D and Citro3D installed under devkitPro's Nintendo 3DS roots.
- SDL2 cross-compiled as a static Nintendo 3DS library, exporting the CMake target
  `SDL2::SDL2-static`. A host SDL2 package cannot be linked into the console game.
- Optional Git for the version suffix. Source archives also build; releases can
  set `BSTONE_CI_GIT_HASH` explicitly.

DOSBox DBOPL, Nuked OPL3 and the stb PNG writer are retained in the repository,
with their licenses. Keep the installed SDL2 N3DS audio, video, events and
GameController/joystick support enabled, together with their threading and
synchronization dependencies.

The toolchain tested during cleanup is devkitARM GCC 16.1.0. The external SDL2
installation reports version 2.33.0; its clean source checkout is commit
`8acf7cfc0f88717e5c036f60f0314bfcafe39189` of
[libsdl-org/SDL](https://github.com/libsdl-org/SDL).

### Building SDL2 separately

If SDL2 is not already installed for Nintendo 3DS, obtain that SDL2 source
revision and build it outside this repository. Example paths below are local
paths you choose:

```sh
export DEVKITPRO=/opt/devkitpro

cmake -S /path/to/SDL2 -B /path/to/SDL2-build-3ds \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=/path/to/SDL2-install-3ds \
  -DSDL_STATIC=ON -DSDL_SHARED=OFF \
  -DSDL_TEST=OFF -DSDL_TESTS=OFF -DSDL2_DISABLE_SDL2MAIN=ON
cmake --build /path/to/SDL2-build-3ds --parallel 4
cmake --install /path/to/SDL2-build-3ds
```

The game loads assets from the SD card and does not link SDL2main or use ROMFS.
The external SDL2 license is preserved in [licenses/SDL2.txt](licenses/SDL2.txt).

### Building the game

From the root of this repository, choose a **new** build directory:

```sh
export DEVKITPRO=/opt/devkitpro

cmake -S . -B build/3ds-release \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
  -DSDL2_DIR=/path/to/SDL2-install-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-release --parallel 4
```

The default build generates both:

```text
build/3ds-release/src/bstone/bstone.elf
build/3ds-release/src/bstone/bstone.3dsx
```

Keep the ELF corresponding to each tested `.3dsx`: ARM11 crash addresses must be
resolved against the exact matching ELF. Missing return statements are build
errors (`-Werror=return-type`).

`SDL2_DIR` may be omitted when the cross-compiled SDL2 package is discoverable
in devkitPro's target prefix. With `DEVKITPRO` set, the project can also select
`cmake/3DS.cmake` automatically. Other target platforms are rejected.

Optional flags are `BSTONE_MORE_COMPILER_WARNINGS`, `BSTONE_USE_PCH` and
`BSTONE_NO_ASSERT`. Debug builds are selected with `-DCMAKE_BUILD_TYPE=Debug`.

### Testing on the console

Enable the Homebrew Launcher's network receiver and send the executable:

```sh
3dslink build/3ds-release/src/bstone/bstone.3dsx
```

Check startup, original data detection, both screens, both stereo eyes and the
slider, widescreen/UI settings, menus, intros/movies/transitions, movement,
actions, weapon selection, sound/music, existing saves, saving/overwriting,
configuration persistence and START+SELECT exit. Successful compilation and
transfer do not establish correct behavior on hardware.

Generic tests for retained engine primitives live under `src/tests`. Their
separate build also targets Nintendo 3DS:

```sh
cmake -S src/tests -B build/3ds-component-tests \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-component-tests --parallel 4
```

This produces `build/3ds-component-tests/src/tests/bstone_tests.3dsx`.
The component suite is cross-compiled during cleanup; it has not been validated
as a complete runtime suite on the console. In particular, generic file tests
can exercise operations not supported by the console filesystem.

## Repository layout

| Path | Purpose |
|---|---|
| `src/bstone/src/` | Game, resource loaders, software video and native 3DS presentation/input |
| `src/bstone/src/dosbox/` | DBOPL sound emulation and its license |
| `src/lib/nuked_opl3/` | Nuked OPL3; `opl3.cpp` includes the required `opl3.c` implementation |
| `src/lib/stb/` | PNG image writer for screenshots and upstream license |
| `src/tests/` | Component tests for retained primitives |
| `cmake/` | Source-revision helper |
| `licenses/` | External runtime dependency notices |

## Known limitations

- Occasional audio interruptions may occur.
- Stereoscopic rendering costs additional CPU/GPU time and is used during
  gameplay; menus and other non-gameplay screens remain in 2D.
- The lower HUD/automap and some original menus remain work in progress.
- Touchscreen controls and optional C-Stick camera turning remain future work.
- Planet Strike has not been tested on hardware.

Report issues at [BStone 3DS Issues](https://github.com/xXNukem/bstone-3ds/issues),
including the build revision and whether the issue occurs in 2D or 3D.

## Credits

Nintendo 3DS port by **xXNukem**.

This project would not be possible without the work of **Boris I. Bendovsky**, creator of the original [BStone source port](https://github.com/bibendovsky/bstone), and its contributors.

Special thanks to the original developers of *Blake Stone: Aliens of Gold*, **JAM Productions**, and its publisher, **Apogee Software**, for creating the game.

BStone 3DS is an independent, unofficial adaptation and is not affiliated with or endorsed by Nintendo or the original game's rights holders.

## License

BStone 3DS is based on the original BStone project and retains its applicable open-source licenses.

The original and modified game source code is licensed under the **GNU General Public License, version 2 or later (GPL-2.0-or-later)**. Independently developed BStone components may be covered by the **MIT License**, as specified in the upstream project.

See [LICENSE.txt](LICENSE.txt) and the included original source code license documentation for full details.

The original BStone documentation is preserved in [README_UPSTREAM.md](README_UPSTREAM.md).
