# ccleste

[![CMake](https://github.com/mupfdev/ccleste/actions/workflows/cmake.yml/badge.svg)](https://github.com/mupfdev/ccleste/actions/workflows/cmake.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/9ef529e6a22d409089bc35f1566fa269)](https://www.codacy.com/gh/mupfdev/ccleste/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mupfdev/ccleste&amp;utm_campaign=Badge_Grade)

![screenshot](https://raw.githubusercontent.com/lemon-sherbet/ccleste/master/screenshot.png)

This is a C source port of the [original celeste (Celeste
classic)](https://www.lexaloffle.com/bbs/?tid=2145) for the PICO-8,
designed to be portable.  PC and the Nokia N-Gage are the main supported
platforms, though other people are [maintaining ports to other
platforms](https://github.com/lemon32767/ccleste/network/members).

Go to [the releases
tab](https://github.com/mupfdev/ccleste/releases) for the latest
pre-built binaries.

This fork tries to make lemon32767's code even more robust and portable.
For this purpose, the entire code was translated to C89 and various C
standard functions were replaced by function calls of the SDL API.  This
is an ongoing process.

This port should compile for all platforms that are officially supported
by SDL2.  Including the Nintendo 3DS.

## Compiling

### Windows

The easiest way to get ccleste up and running is Visual Studio 2022 with
[C++ CMake tools for
Windows](https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio)
installed.  Simply open the cloned repository via `File -> Open ->
Folder`.  Everything else is set up automatically and all required
dependencies are fetched at compile time.

### Nokia N-Gage

The procedure for the Nokia N-Gage is the same as for Windows.  You only
need to install and set up the [N-Gage
SDK.](https://github.com/ngagesdk/ngage-toolchain) in advance and select
the respective build configuration in Visual Studio.

Please note: In order for the SDL executable to be executed properly by
the launcher, the path in the file
[ngage_appui.cpp](src/ngage_appui.cpp#L35) file must be adjusted
accordingly.  Unfortunately, I have not yet found a way to configure
this dynamically.

In addition to the project configuration, the application launcher also
needs to be customised.  The associated files are located in the
sub-folder [res](res/).

### Linux

ccleste can also be compiled on Linux with the included CMake
configuration.

```bash
mkdir build
cd build
cmake ..
make
````

## Controls

|PC                |N-Gage             |Action              |
|:----------------:|:-----------------:|-------------------:|
|LEFT              |LEFT               | Move left          |
|RIGHT             |RIGHT              | Move right         |
|DOWN              |DOWN               | Look down          |
|UP                |UP                 | Look up            |
|Z/C               |7                  | Jump               |
|X/V               |5                  | Dash               |
|ESCAPE            |Softkey (right)    | Pause              |
|E                 |3                  | Toggle screenshake |
|SHIFT+D           |1                  | Load state         |
|SHIFT+S           |2                  | Save state         |
|Hold F9           |Hold C             | Reset              |
|F11               |                   | Fullscreen         |

Controller input is also supported on PC (SDL2 ver) and web version. The
controller must be plugged in when opening the game.  The default
mappings are: jump with A and dash with B (xbox360 controller layout),
move with d-pad or the left stick, pause with start, save/load state
with left/right shoulder, exit with guide (logo button).  You can change
these mappings by modifying the `ccleste-input-cfg.txt` file that will
be created when you first run the game.

You can make the game start up in fullscreen by setting the environment
variable `CCLESTE_START_FULLSCREEN` to "1", or by creating the file
`ccleste-start-fullscreen.txt` in the game directory (contents don't
matter).

## TAS playback and the fixed point question

In order to playback a TAS, specify it as the first argument to the
program when running it. On Windows you can drag the TAS file to the
.exe to do this.  The format for the TAS should be a text file that
looks like "0,0,3,5,1,34,0,", where each number is the input bitfield
and each frame is ended by a comma.  The inputs in the TAS should start
in the first loading frame of 100m (neglecting the title screen). When
playing back a TAS the starting RNG seed will always be the same.

Most other Celeste Classic ports use floating point numbers, but PICO-8
actually uses 16.16 fixed point numbers.  For casual play and RTA
speedrunning, the discrepancies are minor enough to be essentially
negligible, however with TASing it might make a difference.  Defining
the preprocessor macro `CELESTE_P8_FIXEDP` when compiling celeste.c will
use a bunch of preprocessor hacks to replace the float type for all the
code of that file with a fixed point type that matches that of
PICO-8. The use of this preprocessor macro requires compiling celeste.c
with a C++ compiler, however (but not linking with the C++ standard
library).

Using make you can compile this fixed point version with `make
USE_FIXEDP=1`.

When playing back TASes made with other tools that work under the
assumption of ideal RNG for balloons (since their hitbox depends on
that), you can ensure that they do not desync by defining the
preprocessor macro `CELESTE_P8_HACKED_BALLOONS`, which will make
balloons static and always expand their hitbox to their full range.

Using make you can turn on this feature with `make HACKED_BALLOONS=1`.

You can combine both of these with `make HACKED_BALLOONS=1
USE_FIXEDP=1`.

## Credits

Sound wave files are taken from
[https://github.com/JeffRuLz/Celeste-Classic-GBA/tree/master/maxmod_data](https://github.com/JeffRuLz/Celeste-Classic-GBA/tree/master/maxmod_data),
music ogg files were obtained by converting the .wav dumps from pico 8,
which I did using audacity & ffmpeg.

All credit for the original game goes to the original developers (Maddy
Thorson & Noel Berry).
