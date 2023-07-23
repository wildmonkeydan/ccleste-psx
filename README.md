# ccleste psx

[![Celeste 1](https://raw.githubusercontent.com/ngagesdk/ccleste/master/media/screenshot-01-tn.png)](https://raw.githubusercontent.com/ngagesdk/ccleste/master/media/screenshot-01.png?raw=true "Celeste 1")
[![Celeste 2](https://raw.githubusercontent.com/ngagesdk/ccleste/master/media/screenshot-02-tn.png)](https://raw.githubusercontent.com/ngagesdk/ccleste/master/media/screenshot-02.png?raw=true "Celeste 2")

This C source port of the [original
Celeste](https://www.lexaloffle.com/bbs/?tid=2145) for the PICO-8 was
originally developed by
[lemon32767](https://github.com/lemon32767/ccleste).

This version is built off of ngage-sdk's work.

PLEASE NOTE: Pausing and Using Save States does not work and attempting to save state WILL CRASH

This game doesn't work on DUCKSTATION as of version 0.1-5485-gc6a57273. PCSX-Redux or Real Hardware is recommmened instead

## Compiling

### PlayStation

Install [psn00bsdk](https://github.com/Lameguy64/PSn00bSDK), change cmd path to the base dir of the project and type cmake --build ./build

## Controls

|PC                |Action              |
|:----------------:|-------------------:|
|LEFT              | Move left          |
|RIGHT             | Move right         |
|DOWN              | Look down          |
|UP                | Look up            |
|CROSS             | Jump               |
|CIRCLE            | Dash               |

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
