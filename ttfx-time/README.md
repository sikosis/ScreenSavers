# ttfx-time

`ttfx-time` is a native Haiku screen saver that fills the screen with the
current time and rotates through animated text effects. It takes its visual
language from the Omarchy screen saver and adapts effects from
[`ttfx-haiku`](https://github.com/sikosis/ttfx-haiku) for Haiku's `BView`
drawing API.

The upstream `ttfx-haiku` renderer currently targets ANSI terminals. This
screen saver therefore uses a small, non-blocking native effect core rather
than starting a terminal process. The clock remains responsive to Haiku's
screen saver host and works in both full-screen and preference-panel preview
modes.

## Features

- A 5×7 solid-block clock face whose individual cells participate in effects
- Nine rotating effects: Expand, Slide, Wipe, Rain, Decrypt, Scattered, Waves,
  Colour shift and Matrix
- Seeded random colour palettes that remain stable for each effect
- Random or sequential effect order
- Configurable effect and transition durations
- Optional seconds and 12- or 24-hour time
- Double-buffered, 60 frames-per-second native rendering
- Deterministic, host-testable effect geometry
- Haiku R1/beta6-compatible build and HPKG packaging

## Building and testing

Build on Haiku with the development tools installed:

```sh
make -C tests test
make -C src
```

The effect tests use standard C++17 and can also run on macOS or Linux. The
screen saver add-on itself requires Haiku headers and produces `src/ttfx-time`.

Copy the add-on into the per-user screen saver directory:

```sh
mkdir -p "/boot/home/config/non-packaged/add-ons/Screen Savers"
cp src/ttfx-time "/boot/home/config/non-packaged/add-ons/Screen Savers/ttfx-time"
```

Then open **Preferences → ScreenSaver** and select **ttfx-time**.

## Packaging

Create and verify an installable package from the project directory on Haiku:

```sh
./build-package.sh
```

The script runs the effect tests, performs a clean native build, and creates
`dist/ttfx_time_screensaver-1.0.0-1-<architecture>.hpkg`. Package artifacts
are immutable: increase the revision before rebuilding a package that has
already been distributed.

Install the package using HaikuDepot or from Terminal:

```sh
pkgman install dist/ttfx_time_screensaver-1.0.0-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version and licence

- Version: 1.0.0
- Licence: MIT
- Effect inspiration: `sikosis/ttfx-haiku`, `omacom/ttfx`, and
  ChrisBuilds' TerminalTextEffects
