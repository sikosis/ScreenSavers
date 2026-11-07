# KanjiSaver

KanjiSaver is a native Haiku screen saver that displays a large, randomly
selected Japanese kanji on a black background. Each character appears in a
new colour at a configurable interval.

![KanjiSaver screen saver](KanjiSaver.png)

This maintained port is based on the original 1999–2000 KanjiSaver by Jens
Kilian. It preserves the original font selection and saved `family` and
`style` settings while updating the implementation for current Haiku systems.
The optional effects use the BView-native ttfx adapter shared with this
repository's ttfx screen savers. The upstream `ttfx-haiku` engine targets ANSI
terminals, so its visual behaviour is adapted to Haiku drawing primitives
rather than starting a terminal renderer inside ScreenSaver.

## Features

- Random characters from the CJK Unified Ideographs block
- Random hue, saturation and lightness for every character
- Selection from installed fonts that advertise CJK coverage
- Per-character glyph checks to reduce missing-glyph boxes
- Optional 16×16 block assembly using expand, slide, rain, scattered, waves,
  colour shift and Matrix-style ttfx transitions
- Double-buffered animation to prevent flicker while the blocks move
- Configurable character interval from 1 to 15 seconds
- Responsive configuration panel and safe saved-setting validation
- Fixed-buffer UTF-8 glyph handling for reliable long-running animation
- Deterministic, portable UTF-8, colour and random-selection tests
- Haiku R1/beta6-compatible build and HPKG packaging

KanjiSaver needs at least one installed font with CJK Unified Ideographs.

## Building and testing

Run the portable engine tests:

```sh
make -C tests test
```

Build the native screen saver on Haiku with the development tools installed:

```sh
make -C src
```

Install an unpackaged development copy:

```sh
mkdir -p "/boot/home/config/non-packaged/add-ons/Screen Savers"
cp src/KanjiSaver "/boot/home/config/non-packaged/add-ons/Screen Savers/KanjiSaver"
screen_blanker KanjiSaver
```

Then open **Preferences → ScreenSaver** and select **KanjiSaver**.

## Packaging

Create and verify an installable package on Haiku:

```sh
./build-package.sh
```

The script runs the tests, performs a clean native build and creates
`dist/kanjisaver_screensaver-1.2.1-1-<architecture>.hpkg`.

Install it with HaikuDepot or from Terminal:

```sh
pkgman install dist/kanjisaver_screensaver-1.2.1-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version and licence

- Version: 1.2.1
- Original author: Jens Kilian, 1999–2000
- Licence: GNU General Public License, version 1 or any later version
- ttfx effect adapter: MIT-licensed code adapted from `ttfx-text` and
  `ttfx-haiku`; see `THIRD_PARTY.md`
