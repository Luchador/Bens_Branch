# Ben's Mods to the Perfect Dark PC Port

This is mostly to help me learn the Perfect Dark code and maybe make some cool changes. This is meant for NTSC 1.1/Final only.

Changes include:
* Press G to go into Gangsta Mode whenever you want (hold some guns sideways including the CMP150 and ZZT 9mm)
* Far more corpses are allowed before they start being removed
* Male guards will select from all 42 available heads instead of just 8 like the original game
* Shotgun pellets do more damage and single blast has smaller spread, but magazine size reduced to 8
* Sniper Rifle has new secondary mode, Armor Piercing. Bullets do more damage and penetrate up to 3 objects at the cost of a small accuracy penalty
* All Guns cheat gives the Slayer on all stages, not just Attack Ship and Ruins
* Added a cheat called "Dinner Party"
* Far more bullet holes supported
* Laser weapon leaves scorch marks on props and background
* Twice the smoke puffs supported
* Tinted glass does not become fully opaque
* Laser obstacles stay fully bright even if a room's lights are destroyed
* Stars added on Mr. Blonde's Revenge
* Animated water at the bottom of the chasm in Air Base
* Moved the "Objective Completed/Failed" message down so it doesn't block the center of the screen

Bug fixes:
* Fixed Jonathan being unable to shoot the player in The Duel
* Fixed the bug where wine bottles don't play their shatter sound half the time when destroyed
* Fixed the function indicator not turning yellow during RC-P120 secondary mode
* Fixed a very minor bug where using a light switch would change a room's lighting faster than intended

Stage changes:

Note: These changes require alterations to the game's original files. You can find these files in the build/mods folder.

* Investigation: Added tables with microscopes in some lab rooms
* Investigation: Fixed Night Vision goggles item using the IR Specs model
* Villa: Brightened up the dock clipping tiles, which were oddly dark
* Villa: Fixed the broken texture underneath the stove hood
* Villa: Made some vertex coloring tweaks to the bedroom and bathroom

## Running

These instructions taken from the [PC Port](https://github.com/fgsfdsfgs/perfect_dark) project by fgsfdsfgs.

You must have a * `ntsc-final`/`US V1.1`/`US Rev 1` (md5 `e03b088b6ac9e0080440efed07c1e40f`) ROM. Name it pd.ntsc-final.z64

This assumes that you're using an x86_64 build. If you aren't, replace `x86_64` below with your arch (e.g. `i686`).

1. Create a directory named `data` next to `pd.x86_64` if it's not there.
2. Put your Perfect Dark NTSC ROM named `pd.ntsc-final.z64` into it.
3. Run the `pd.x86_64` executable.

Optionally, you can also put your Perfect Dark for GameBoy Color ROM named `pd.gbc` in the `data` directory if you want to emulate having the Nintendo 64's Transfer Pak and unlock some cheats automatically.

Additional information can be found in the [wiki](https://github.com/fgsfdsfgs/perfect_dark/wiki).

A GPU supporting OpenGL 3.0/ES3.0 or above is required to run the port.

## Download

*Coming soon

## Controls

1964GEPD-style and Xbox-style bindings are implemented.

N64 pad buttons X and Y (or `X_BUTTON`, `Y_BUTTON` in the code) refer to the reserved buttons `0x40` and `0x80`, which are also leveraged by 1964GEPD.

Support for one controller, two-stick configurations are enabled for 1.2.

Note that the mouse only controls player 1.

Controls can be rebound in `pd.ini`. Default control scheme is as follows:

| Action           | Keyboard and mouse     | Xbox pad                 | N64 pad                   |
| -                | -                      | -                        | -                         |
| Fire / Accept    | LMB/Space              | RT                       | Z Trigger                 |
| Aim mode         | RMB/Z                  | LT                       | R Trigger                 |
| Use / Cancel     | E                      | N/A                      | B                         |
| Use / Accept     | N/A                    | A                        | A                         |
| Crouch cycle     | N/A                    | L3                       | `0x80000000` (Extra)      |
| Half-Crouch      | Shift                  | N/A                      | `0x40000000` (Extra)      |
| Full-Crouch      | Control                | N/A                      | `0x20000000` (Extra)      |
| Reload           | R                      | X                        | X `(0x40)`                |
| Previous weapon  | Mousewheel forward     | B                        | D-Left                    |
| Next weapon      | Mousewheel back        | Y                        | Y `(0x80)`                |
| Radial menu      | Q                      | LB                       | D-Down                    |
| Alt fire mode    | F                      | RB                       | L Trigger                 |
| Alt-fire oneshot | `F + LMB` or `E + LMB` | `A + RT` or  `RB + RT`   | `A + Z`     or `L + Z`    |
| Quick-detonate   | `E + Q`   or `E + R`   | `A + B`  or  `A + X`     | `A + D-Left`or `A + X`    |

## Building

### Windows

1. Install [MSYS2](https://www.msys2.org).
2. Open the `MINGW64` prompt if building for x86_64, or the `MINGW32` prompt if building for i686. (**NOTE:** _do not_ use the `MSYS` prompt)
3. Install dependencies:  
   `pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-SDL2 mingw-w64-x86_64-zlib mingw-w64-x86_64-cmake mingw-w64-x86_64-python3 mingw-w64-i686-toolchain mingw-w64-i686-SDL2 mingw-w64-i686-zlib mingw-w64-i686-cmake mingw-w64-i686-python3 make git`
4. Get the source code:  
   `git clone --recursive https://github.com/fgsfdsfgs/perfect_dark.git && cd perfect_dark`
5. Run `cmake -G"Unix Makefiles" -Bbuild .`.
   * Add ` -DROMID=pal-final` or ` -DROMID=jpn-final` at the end of the command if you want to build a PAL or JPN executable respectively.\
6. Run `cmake --build build -j4 -- -O`.
7. The resulting executable will be at `build/pd.x86_64.exe` (or at `build/pd.i686.exe` if building for i686).
8. If you don't know where you downloaded the source to, you can run `explorer .` to open the current directory.

## Credits

* fgsfdsfgs for the incredible PC port

Original project [here](https://github.com/fgsfdsfgs/perfect_dark)
