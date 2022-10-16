# ccleste

[![CMake](https://github.com/mupfdev/ccleste/actions/workflows/cmake.yml/badge.svg)](https://github.com/mupfdev/ccleste/actions/workflows/cmake.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/9ef529e6a22d409089bc35f1566fa269)](https://www.codacy.com/gh/mupfdev/ccleste/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mupfdev/ccleste&amp;utm_campaign=Badge_Grade)

[![Celeste 1](https://raw.githubusercontent.com/mupfdev/ccleste/master/media/screenshot-01-tn.png)](https://raw.githubusercontent.com/mupfdev/ccleste/master/media/screenshot-01.png?raw=true "Celeste 1")
[![Celeste 2](https://raw.githubusercontent.com/mupfdev/ccleste/master/media/screenshot-02-tn.png)](https://raw.githubusercontent.com/mupfdev/ccleste/master/media/screenshot-02.png?raw=true "Celeste 2")

This C source port of the [original
Celeste](https://www.lexaloffle.com/bbs/?tid=2145) for the PICO-8 was
originally developed by
[lemon32767](https://github.com/lemon32767/ccleste).

In the original version, attention was paid to portability, but there
was still some need for improvement.  Especially since I really wanted
to get the game to run on the **Nokia N-Gage** and was stuck with a very
old compiler.

In addition, the SDL 1.2 integration seemed obsolete to me, since SDL 2
now natively supports the **Nintendo 3DS**.

This fork aims to be more robust and portable than the original and
should, at least in theory, be compilable from all platforms officially
supported by SDL 2.

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
|SHIFT+D           |2                  | Load state         |
|SHIFT+S           |1                  | Save state         |
|Hold F9           |Hold C             | Reset              |
|F11               |                   | Fullscreen         |

## Credits

Sound wave files are taken from the
[Celeste-Classic-GBA](https://github.com/JeffRuLz/Celeste-Classic-GBA/tree/master/maxmod_data)
project, music *.ogg files were obtained by converting the *.wav dumps
from PICO-8, which I did using audacity & ffmpeg.

The frame for the N-Gage version is based on [Mountain at Dusk
Background](https://opengameart.org/content/mountain-at-dusk-background)
by ansimuz.

All credit for the original game goes to the original developers (Maddy
Thorson & Noel Berry).
