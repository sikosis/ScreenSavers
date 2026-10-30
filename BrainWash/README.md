# BrainWash

BrainWash is a native Haiku screen saver that grows layers of mathematical
colour fields outwards from randomly selected centres. Each layer combines
spirals, circles, waves, grids, lenses and twists while two luminous palettes
continuously cycle beneath it.

This maintained port is based on the archived BrainWash 1.3.1 source by Stephan
Aßmus and Ingo Weinhold. It replaces the BeOS-era `BWindowScreen`, direct 8-bit
framebuffer writes and worker-thread network with a safe `BScreenSaver` renderer
for current Haiku systems.

## Features

- Original randomly combined mathematical transformations and two-layer design
- Continuously animated 256-entry colour model rendered through a modern bitmap
- Configurable colour-cycle speed, drawing speed and delay between patterns
- Full-screen and ScreenSaver preference-panel preview support
- Automatic use of the host view's current resolution and resize handling
- Deterministic, bounds-checked portable rendering core
- Haiku R1/beta6-compatible build and HPKG packaging

The obsolete resolution selector has intentionally been removed. Modern Haiku
screen savers render at the resolution supplied by the host rather than opening
a second exclusive 8-bit display mode.

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
cp src/BrainWash "/boot/home/config/non-packaged/add-ons/Screen Savers/BrainWash"
screen_blanker BrainWash
```

## Packaging

Create and verify an installable package on Haiku:

```sh
./build-package.sh
```

The script runs the tests, performs a clean native build and creates
`dist/brainwash_screensaver-1.4.0-1-<architecture>.hpkg`.

Install it with HaikuDepot or from Terminal:

```sh
pkgman install dist/brainwash_screensaver-1.4.0-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version, attribution and licensing

- Version: 1.4.0
- Original authors: Stephan Aßmus and Ingo Weinhold
- Original release: BrainWash 1.3.1 for BeOS

The archived source contains copyright attribution but no explicit software
licence. See [COPYRIGHT](COPYRIGHT) for the licensing notice retained with this
port.
