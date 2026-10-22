# ttfx-text

`ttfx-text` is a native Haiku screen saver that renders configurable text as a
5×7 solid-block display and cycles it through animated text effects. It uses
the same native effect engine as `ttfx-time`, inspired by
[`ttfx-haiku`](https://github.com/sikosis/ttfx-haiku) and the Omarchy screen
saver.

The default text is `HAIKU`. Open the screen saver settings to enter up to 16
characters. Letters are displayed in uppercase; A–Z, 0–9, spaces, `!`, `?`,
`-`, `.`, and `_` have native block patterns. Other characters use the question
mark pattern.

## Features

- Custom text configured directly in ScreenSaver preferences
- A 5×7 solid-block alphabet whose individual cells participate in effects
- Nine effects: Expand, Slide, Wipe, Rain, Decrypt, Scattered, Waves, Colour
  shift and Matrix
- Random or sequential effect order
- Configurable effect and transition durations
- Optional effect-name label for debugging or effect identification
- Seeded random colour palettes and continuous Colour shift animation
- Double-buffered, 60 frames-per-second native rendering
- Full-screen and preference-panel preview modes
- Haiku R1/beta6-compatible build and HPKG packaging

## Building and testing

Build on Haiku with the development tools installed:

```sh
make -C tests test
make -C src
```

The block-art and effect tests use standard C++17 and can also run on macOS or
Linux. The native add-on requires Haiku headers and produces `src/ttfx-text`.

Install an unpackaged copy for local testing:

```sh
mkdir -p "/boot/home/config/non-packaged/add-ons/Screen Savers"
cp src/ttfx-text "/boot/home/config/non-packaged/add-ons/Screen Savers/ttfx-text"
```

Then reopen **Preferences → ScreenSaver** and select **ttfx-text**.

## Packaging

Create and verify an installable package on Haiku:

```sh
./build-package.sh
```

The script runs the tests, performs a clean native build, and creates
`dist/ttfx_text_screensaver-1.0.0-1-<architecture>.hpkg`.

Install it using HaikuDepot or Terminal:

```sh
pkgman install dist/ttfx_text_screensaver-1.0.0-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version and licence

- Version: 1.0.0
- Licence: MIT
- Effect inspiration: `sikosis/ttfx-haiku`, `omacom/ttfx`, and
  ChrisBuilds' TerminalTextEffects
