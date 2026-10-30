# Substrate

Substrate is a native Haiku screen saver that grows intersecting cracks and
deposits translucent coloured sand into an evolving crystalline pattern.

This maintained port is based on
[`HaikuArchives/Substrate`](https://github.com/HaikuArchives/Substrate), which
Josef Gajdusek adapted from XScreenSaver's Substrate implementation and Jared
Tarbell's original generative artwork.

## Features

- Straight and circular crack growth with collision-driven branching
- Earthy translucent sand palette with light and dark backgrounds
- Configurable crack count, duration, sand density and circular-crack ratio
- Responsive Haiku layout-based configuration panel
- Monotonic real-time scene duration in full-screen and preview modes
- Resize-safe, bounds-checked simulation storage
- Haiku R1/beta6-compatible build and HPKG packaging

## Building and testing

Build and run the portable maths tests:

```sh
make -C tests test
```

Build the screen saver on Haiku with the development tools installed:

```sh
make -C src
```

The resulting add-on is `src/Substrate`. Copy it into the per-user screen saver
directory for development:

```sh
mkdir -p "/boot/home/config/non-packaged/add-ons/Screen Savers"
cp src/Substrate "/boot/home/config/non-packaged/add-ons/Screen Savers/Substrate"
screen_blanker Substrate
```

## Packaging

Create and verify an installable package from the project directory on Haiku:

```sh
./build-package.sh
```

The script runs the tests, performs a clean native build, and creates
`dist/substrate_screensaver-1.1.0-1-<architecture>.hpkg`. Package artifacts are
immutable: increase the revision before rebuilding a package that has already
been distributed.

Install it with HaikuDepot or from Terminal:

```sh
pkgman install dist/substrate_screensaver-1.1.0-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version and licence

- Version: 1.1.0
- Original Haiku port: Josef Gajdusek
- Licence: MIT
