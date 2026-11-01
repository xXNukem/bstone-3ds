# Blake Stone 3DS

**Blake Stone: Aliens of Gold and Planet Strike, now on New Nintendo 3DS. With two screens, stereoscopic 3D, and a few extras that weren't in the DOS version.**

Blake Stone 3DS is an unofficial homebrew port of both classic Blake Stone games, based on [BStone](https://github.com/bibendovsky/bstone) by Boris I. Bendovsky.

The port includes a redesigned dual-screen interface, a permanent automap, adapted controls, new menus and native stereoscopic rendering.

## What's included?

- **Two games in one executable:** Aliens of Gold and Planet Strike.
- **Shareware and full-version support** for Aliens of Gold.
- **Native stereoscopic 3D**, controlled by the console's physical 3D slider.
- **Full-screen first-person gameplay** on the upper display.
- **A redesigned lower-screen HUD**, including health, ammunition, weapons, score, access cards and other useful information.
- **A permanent, real-time automap**, so you don't have to interrupt the action just to figure out where the hell you are.
- **Animated ECG health monitor**, with its appearance changing according to your health.
- **Green phosphor-style menus**, complete with CRT effects and stereoscopic depth.
- **Redesigned loading screens** using both displays.
- **Three control layouts:** Classic, Modern and Modern + C-Stick.
- **Weapon bobbing**, customizable through the game's settings.
- **Original music and sound effects**, using the audio systems retained from BStone.

## Console compatibility

**Blake Stone 3DS has been tested on a New Nintendo 3DS XL, where it is fully functional.**

Other models in the Nintendo 3DS family have not been tested yet, so their compatibility still needs to be confirmed.

Stereoscopic 3D requires a console with a 3D display.

You'll need a homebrew-capable console and the Homebrew Launcher.

## Installation

You'll need `bstone.3dsx` from the [latest release](https://github.com/xXNukem/bstone-3ds/releases), plus the original game data from your own copies of Blake Stone.

**No commercial game data is included in this project.**

Create the following folders on your SD card:

```text
sdmc:/3ds/bstone/
    bstone.3dsx
    games/
        aog/
        ps/
    profiles/
```

Put your Aliens of Gold data files inside `games/aog/` and your Planet Strike files inside `games/ps/`.

### Aliens of Gold

The port recognizes the shareware (`.BS1`) and full (`.BS6`) editions.

For the full version, you'll need:

```text
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

For the shareware version, you'll need:

```text
AUDIOHED.BS1
AUDIOT.BS1
IANIM.BS1
MAPHEAD.BS1
MAPTEMP.BS1
SANIM.BS1
VGADICT.BS1
VGAGRAPH.BS1
VGAHEAD.BS1
VSWAP.BS1
```

### Planet Strike

Place these files in `games/ps/`:

```text
AUDIOHED.VSI
AUDIOT.VSI
EANIM.VSI
IANIM.VSI
MAPHEAD.VSI
MAPTEMP.VSI
VGADICT.VSI
VGAGRAPH.VSI
VGAHEAD.VSI
VSWAP.VSI
```

Keep each game's files together, and don't mix files from different releases.

### Starting the game

Launch Blake Stone 3DS through the Homebrew Launcher.

You'll be greeted by a game selector where you can choose between Aliens of Gold and Planet Strike.

The lower screen shows information about the selected installation, including its edition, detected version, missing files and integrity status.

The port checks the original data against known SHA-1 signatures before starting. If something's missing or the files don't match a supported release, you'll know before the engine tries loading them.

**Selector controls:**

- D-Pad / Circle Pad: choose a game.
- Left / Right: switch between available Aliens of Gold editions or installations.
- A: verify and launch.
- X: rescan the installed game files.
- B: exit confirmation.

Once you've selected a game, that title remains active until you close the application.

### Upgrading from an older version

If you already used an earlier Blake Stone 3DS build, the launcher can still detect Aliens of Gold data placed directly in `sdmc:/3ds/bstone/`.

Old saves and configuration files can also be read through the legacy compatibility mechanism.

New saves and settings are stored in separate profile folders:

```text
profiles/aog_sw/
profiles/aog_full/
profiles/ps/
```

**Back up your existing saves before upgrading.** The port won't automatically move or delete them.

## Controls

Three layouts are available through **Control Settings**.

### Classic

The original Blake Stone 3DS control layout.

| Button | Action |
|---|---|
| Circle Pad / D-Pad Up and Down | Move forward / backward |
| Circle Pad / D-Pad Left and Right | Turn |
| L / R | Strafe |
| A | Fire |
| B | Interact / Open doors |
| ZL / ZR | Next / previous weapon |
| X | Toggle automap |
| START | Game menu |
| START + SELECT | Exit to Homebrew Launcher |

### Modern

A more contemporary control scheme, with analog movement and strafing.

| Button | Action |
|---|---|
| Circle Pad | Move / Strafe |
| Y / A | Turn left / right |
| R | Fire |
| B | Interact |
| D-Pad Left / Right | Previous / next weapon |
| X | Toggle automap |
| START | Game menu |

### Modern + C-Stick

The Modern layout, with the C-Stick available for horizontal camera turning.

There's no vertical freelook, since Blake Stone uses a classic Wolfenstein-style engine.

All control layouts retain the same basic menu navigation: **A to confirm, B to go back**.

Your selected layout is saved in the game's preferences.

## The two-screen setup

The upper screen is dedicated to the actual first-person game, without the original DOS status bar taking up valuable space.

All that information has moved downstairs.

The lower screen displays health, ammo, weapons, score, access cards, radar information and a live automap. There's also an animated heart monitor that changes depending on your health.

During menus, introductions and other non-gameplay screens, the lower display switches to the appropriate Blake Stone title presentation.

The menu system has also been redesigned around a green phosphor terminal aesthetic, with animated CRT effects and stereoscopic depth.

## Stereoscopic 3D

The port uses separate perspectives for the left and right eyes rather than simply duplicating the same image with an offset.

Walls, enemies, objects and weapons are presented stereoscopically, with depth controlled by the physical 3D slider.

The interface also uses subtle depth effects, with more conservative settings for text-heavy screens such as mission briefings and instructions.

Stereoscopic rendering requires additional processing power, so performance can vary depending on the scene.

## Video and sound

The video settings have been adapted to the console instead of exposing desktop options that aren't relevant to the hardware.

Available presentation settings include native widescreen or centered 4:3 gameplay and the CRT menu effect.

Weapon bobbing can also be controlled through the game's switches.

Sound uses the original BStone audio systems, including the retained OPL emulation and sound effects.

## Building from source

You'll need:

- devkitPro and devkitARM.
- libctru, Citro2D and Citro3D.
- CMake 3.18 or newer.
- Nintendo 3DS build tools, including `3dsxtool` and `smdhtool`.
- A static Nintendo 3DS build of SDL2 providing `SDL2::SDL2-static`.

SDL2 is built separately from this repository. A normal desktop installation of SDL2 won't work here.

A typical release build looks like this:

```sh
export DEVKITPRO=/opt/devkitpro

cmake -S . -B build/3ds-release \
  -DCMAKE_TOOLCHAIN_FILE="$DEVKITPRO/cmake/3DS.cmake" \
  -DSDL2_DIR=/path/to/SDL2-install-3ds/lib/cmake/SDL2 \
  -DBSTONE_ENABLE_AI_WEAPONS=OFF \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build/3ds-release --parallel 4
```

The compiled executable is generated at:

```text
build/3ds-release/src/bstone/bstone.3dsx
```

The build also generates an ELF file, which is worth keeping around if you ever need to investigate an ARM11 crash.

For testing over the network, you can use `3dslink` with the executable.

The `docs/`, `tests/` and `tools/` directories contain additional development material.

## What about AI Enhanced graphics?

You may notice some experimental code and tools related to alternative weapon graphics.

I've been experimenting with AI-assisted reinterpretations of the original artwork, including redesigned weapons and potentially other game assets.

**Those experimental graphics are not included in version 1.0.**

The first stable release is focused on getting both original games running properly, with the 3DS-specific features in place.

Alternative graphic packs may arrive in later versions as optional additions. The original artwork will remain available.

## Known limitations

- There's no touchscreen control scheme.
- The C-Stick provides horizontal turning only.
- Unsupported or modified game-data sets may fail integrity verification.
- Not every historical release of both games has been exhaustively tested on real hardware.
- Audio behavior and compatibility with older save files may need further testing across different installations.

If you find something broken, feel free to [open an issue](https://github.com/xXNukem/bstone-3ds/issues).

Please include which game and edition you're using, what happened, and whether stereoscopic 3D was enabled.

## A note about AI-assisted development

Since AI-assisted programming has become a fairly hot topic, I'd rather be straightforward about it.

**Generative AI was used throughout development to help inspect and understand the original BStone codebase, investigate bugs, work on code changes, refactor and optimize parts of the engine, and speed up the overall porting process.**

I directed the project, made the design decisions, reviewed the changes and tested the results on real Nintendo 3DS hardware.

AI also helped with some of the less exciting parts of development, including writing painfully long documentation like this README.

Because apparently getting a 1993 FPS running in stereoscopic 3D on a handheld wasn't enough paperwork.

The underlying BStone engine, the original Blake Stone games and their respective authors deserve full credit for the work this port is built upon.

## Credits

**Nintendo 3DS port**

- **Carlos de la Barrera Perez (xXNukem)** — development and adaptation for New Nintendo 3DS.

**Original games and engine**

- **JAM Productions** — creators of Blake Stone: Aliens of Gold and Planet Strike.
- **Apogee Software** — original publisher.
- **id Software** — Wolfenstein 3D engine heritage.
- **Boris I. Bendovsky and the BStone contributors** — original BStone source port.

**Tools and libraries**

- devkitPro and the Nintendo 3DS homebrew community.
- libctru, Citro2D and Citro3D contributors.
- SDL contributors.
- DOSBox/DBOPL, Nuked OPL3 and stb contributors.

Blake Stone 3DS is an independent, unofficial homebrew project and is not affiliated with or endorsed by Nintendo or the original games' rights holders.

## License

Blake Stone 3DS is based on BStone and retains the applicable open-source licenses and original attributions.

The relevant source code is licensed under **GPL-2.0-or-later**, with certain independently developed components and bundled libraries retaining their own licenses.

For the full details, see:

- [LICENSE.txt](LICENSE.txt)
- [Original BStone README](README_UPSTREAM.md)
- [SDL2 license](licenses/SDL2.txt)
- Original source and library license notices included in the repository.

The original commercial game assets are not included and remain subject to their own rights.