# One switch action per physical press; weapon audio audit

## Reproduced cause

`ReadAnyControl` deliberately maps native A/B using `hidKeysHeld`. `HandleMenu`
returns the selected row as soon as A is down. Switch screens call it again
immediately after toggling. `IN_ClearKeysDown` only clears the emulated keyboard;
it cannot release a physical key. Thus one ordinary sustained press could toggle
settings every pass. B could also propagate through multiple parent screens.

Enhanced appeared especially hard to turn OFF: OFF frees the cache quickly;
turning ON rescans files and is slower. Repeated held-button actions could return
it to ON before the user released A. There was no intentional ON lock.

## Fix

Native `HandleMenu` now calls the existing `WaitKeyUp` at entry, before accepting
another action. It consumes the previous held A/B, including across submenu
boundaries. Switch changes still happen on the first accepted press, not after
an arbitrary timeout. `WaitKeyUp` pumps input and the CRT's paced idle while held;
there is no busy spin, timer debounce or frozen background. D-pad hold/repeat
and gameplay bindings are unchanged. The Enhanced branch also records the old
value once: ON goes directly OFF; OFF rescans and tries ON once.

No separate fixes per switch, no new menu/input system, no forced audio preference.

## Audio findings and limits

Enhanced does not replace audio data and does not participate in `GunAttack`,
`T_Attack` or `sd_play_player_weapon_sound`. Production calls still select
Autocharge 23, Protector 24, Pulse 26, ION 11 and Grenade 87 from the original game
sound resources. UI sounds use their own original route. The PCM/AdLib/PC speaker
selection and mixer have not been modified by this fix.

A held-button bug also affected SOUND EFFECTS and DIGITIZED SFX, so sound settings
could have been unintentionally changed and saved. This is a possible explanation,
not an observed reading of the console's configuration. After installing the fix,
check SOUND SETTINGS → SOUND EFFECTS ON and SOUND VOLUME → effects above zero,
and compare Enhanced OFF/ON with the same weapon. Do not force unmuting on users
who intentionally disabled effects. If shots alone remain silent with those
settings correct, hardware logs/configuration and whether UI/doors/enemies/music
play are still needed to identify the audio cause. No hardware audio fix is
claimed from host mocks or from compilation.

## Tests and build

- `tests/3ds_menu_switch_input.py` executes production HandleMenu, WaitKeyUp,
  TicDelay and cp_video against traces held for 1/5/60/300 polls. Exactly one
  switch action per press, B isolation across parents, CRT idle pumping,
  D-pad navigation and actual Enhanced OFF→ON→OFF branch pass under ASan/UBSan.
- `tests/3ds_weapon_audio_route.py` executes production weapon sound selection,
  player weapon wrapper and non-positional playback code into a recorded mixer.
  Sound IDs, sample pointers, non-positional/non-looping flags, gain setup,
  AdLib/PC/PCM requests and disabled/null/empty guards pass. This checks routing,
  not the console's actual audio device or saved preferences.
- Enhanced loader/render and original Classic/bob/aspect regressions also run.

Clean devkitARM build: `build/3ds-menu-switch-fix`.
Executable: `build/3ds-menu-switch-fix/src/bstone/bstone.3dsx`.
The artwork pack is unchanged; no recopy is needed if it is already installed.
No commits, merges or push.
