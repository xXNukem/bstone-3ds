# BStone 3DS

**A native port of Blake Stone: Aliens of Gold for New Nintendo 3DS.**

BStone 3DS is an unofficial homebrew port of the classic 1993 first-person shooter *Blake Stone: Aliens of Gold* to the New Nintendo 3DS family of systems.

Based on [BStone](https://github.com/bibendovsky/bstone), the open-source engine developed by Boris I. Bendovsky, this project aims to bring the complete Blake Stone experience to Nintendo's handheld while taking advantage of its unique hardware, controls, and dual-screen design.

The project is developed independently as a personal, open-source effort.

## Current Status

**Early playable version**

The game is currently playable on New Nintendo 3DS hardware.

Current features include:

- *Blake Stone: Aliens of Gold* running on the upper screen at 400×240 resolution.
- Full-screen game rendering, including weapons, HUD, menus, and interface.
- Controls adapted to the New Nintendo 3DS.
- Working sound effects and music.
- Functional game menus and save/load system.
- Playable in-game performance.
- Native `.3dsx` homebrew executable.

The entire game is currently displayed on the upper screen. The lower screen is reserved for future features.

*Blake Stone: Aliens of Gold* is the game currently tested with this port. **Blake Stone: Planet Strike has not been tested yet**, so its compatibility is currently unknown.

## Installation

### Requirements

- A New Nintendo 3DS, New Nintendo 3DS XL, or New Nintendo 2DS XL capable of running homebrew.
- The Homebrew Launcher.
- The `bstone.3dsx` executable.
- Original game data files from the full version of *Blake Stone: Aliens of Gold*.

### Steps

1. Obtain `bstone.3dsx` from a published release or compile it from source.
2. Create the directory `/3ds/bstone/` on your SD card.
3. Copy `bstone.3dsx` into that directory.
4. Copy your original *Blake Stone: Aliens of Gold* game data files (`.BS6`) into the same directory.
5. Launch BStone 3DS through the Homebrew Launcher.

The final directory should look similar to this:

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

The game automatically uses this directory for its data and profile files.

**Important:** Game assets are not included in this repository or its releases. You must provide your own legally obtained copy of the game.

## Controls

| Button | Action |
|---|---|
| D-Pad Up / Down | Move forward / backward |
| D-Pad Left / Right | Turn left / right |
| L / R | Strafe left / right |
| A | Fire / Confirm |
| B | Interact, open doors / Cancel |
| X | Toggle automap |
| ZL | Next weapon |
| ZR | Previous weapon |
| START | Open game menu |
| START + SELECT | Exit the game |

The original game menus can be navigated using the console controls.

## Known Issues

This is an early version, and some issues are still being addressed.

- Occasional audio glitches or interruptions may occur during gameplay.
- The lower screen is not yet used.
- Some interface elements and menus still need further adaptation to the handheld format.
- Only *Aliens of Gold* has been tested; *Planet Strike* remains untested.

Bug reports, testing results, and suggestions are welcome through [GitHub Issues](https://github.com/xXNukem/bstone-3ds/issues).

## Roadmap

The long-term goal is to create a complete, polished Blake Stone experience specifically designed for the New Nintendo 3DS.

### Planned Improvements

- [ ] Fix remaining audio glitches and improve sound stability.
- [ ] Further optimize performance and gameplay stability.
- [ ] Refine controls and menu navigation, including optional C-Stick camera turning.
- [ ] Move the HUD to the lower screen, leaving the upper screen dedicated entirely to the first-person view.
- [ ] Implement a permanently visible, real-time automap on the lower screen.
- [ ] Add touchscreen interactions and controls to the lower screen.
- [ ] Improve the dual-screen interface and overall presentation.
- [ ] Implement native stereoscopic 3D rendering as a final major feature.

Features and priorities may change as development progresses.

## Credits

This project would not be possible without the work of **Boris I. Bendovsky**, creator of the original [BStone source port](https://github.com/bibendovsky/bstone), and its contributors.

Special thanks to the original developers of *Blake Stone: Aliens of Gold*, **JAM Productions**, and its publisher, **Apogee Software**, for creating the game.

BStone 3DS is an independent, unofficial adaptation and is not affiliated with or endorsed by Nintendo or the original game's rights holders.

## License

BStone 3DS is based on the original BStone project and retains its applicable open-source licenses.

The original and modified game source code is licensed under the **GNU General Public License, version 2 or later (GPL-2.0-or-later)**. Independently developed BStone components may be covered by the **MIT License**, as specified in the upstream project.

See [LICENSE.txt](LICENSE.txt) and the included original source code license documentation for full details.

The original BStone documentation is preserved in [README