# Comfortable long-document stereo plane

This pass changes only the shared adapter's long-document stereo treatment.
It does not change layout, text, controls, CRT animation, loading presentation,
gameplay/HUD rendering, weapon rendering or projection calculations.

## Classification and planes

`show_document` / `document_mode` already identify the existing long-page
presenter. Briefings/debriefings, Instructions, History, credits, ordering
information and the CLASSIC guide therefore share a reading policy without
adding another renderer or per-section duplicated code. `show_list` resets that
mode for main/pause, options, save/load, new game and difficulty. Meters and
ordinary short information dialogs retain their prior treatment.

At full slider, per-eye shifts are:

| Screen/element | Left | Right |
| --- | --- | --- |
| All CRT backgrounds (unchanged) | -1.25 px | +1.25 px |
| Selectable-menu heading/body/help (unchanged) | -0.35 px | +0.35 px |
| Unselected menu rows (unchanged) | 0 | 0 |
| Selected menu row (unchanged) | +0.65 px | -0.65 px |
| Long-document main text and embedded art | +0.25 px | -0.25 px |
| Long-document heading/page counter/help | 0 | 0 |
| LOADING presentation (unchanged) | -0.35 px | +0.35 px |

The document body's total disparity is now a conservative forward 0.5 pixels,
instead of the previous backward 0.7 pixels. Its direction uses the same
convention as the approved selected-menu pop-out, with substantially less
separation. Every part of the document body shares one plane, including its
original illustrations. Header and button help do not compete with that plane.
The background stays behind. The physical slider still scales the shifts
linearly, and zero/below-threshold values give the same 2D view in both eyes.

Neither page index, scroll position nor text length changes this depth. Existing
menu-to-document transition/lifetime code remains in use. Loading explicitly
keeps its old plane even if entered from a document, and returning to a list
restores the approved interactive-menu plane. The same indexed UI texture and
Citro2D model translation are reused; no other stereo code is changed.

## Changed files and verification

- `bstone_3ds_menu.cpp`: document-specific body and chrome planes.
- `tests/3ds_terminal_adapter.py`: document tests at zero/half/full slider,
  neutral heading/help positions in both eyes, page/scroll depth consistency,
  loading exception and exact return to existing menu shifts.
- This report and a cross-reference in the earlier stereo report.

```sh
python3 tests/3ds_terminal_adapter.py
cmake -S . -B build/3ds-reading-depth \
  -DCMAKE_TOOLCHAIN_FILE=/opt/devkitpro/cmake/3DS.cmake \
  -DSDL2_DIR=/Users/kurlos/Desktop/Proyectos/BlakeStone3DS/dependencias/SDL2-instalado-3ds/lib/cmake/SDL2 \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build/3ds-reading-depth --parallel 4
```

Artifact: `build/3ds-reading-depth/src/bstone/bstone.3dsx`.

The actual adapter regression passes against mocked draw/platform boundaries;
this establishes offsets, state transitions and the unchanged menu treatment,
not subjective comfort on the console. Validate sustained reading, scrolling,
page changes and menu returns on a New Nintendo 3DS with the slider at low,
mid and full levels, in AOG and PS. No commits, merges or pushes were made.
