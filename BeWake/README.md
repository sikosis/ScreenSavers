# BeWake

BeWake is a native Haiku screen saver that draws a fading wake between two
independently bouncing points. The result continuously folds into fans,
triangles and narrow beams against a black background.

![BeWake screen saver](BeWake.png)

This maintained port is based on BeWake 1.1 by Mattia Tristo and Gabriele
Biffi. It preserves the original 50-line trail and saved-setting keys while
updating the implementation for current Haiku systems.

## Features

- Cycling, periodically random or user-selected fixed colours
- Responsive configuration panel with a visual hue selector
- Bounds-safe movement on full screens, previews and very small views
- Automatic off-screen-buffer recreation after display or preview resizing
- Deterministic and portable movement and HSL colour tests
- Flicker-free native rendering at the original 50 updates per second
- Haiku R1/beta6-compatible build and HPKG packaging

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
cp src/BeWake "/boot/home/config/non-packaged/add-ons/Screen Savers/BeWake"
screen_blanker BeWake
```

Then open **Preferences → ScreenSaver** and select **BeWake**.

## Packaging

Create and verify an installable package on Haiku:

```sh
./build-package.sh
```

The script runs the tests, performs a clean native build and creates
`dist/bewake_screensaver-1.2.0-1-<architecture>.hpkg`.

Install it with HaikuDepot or from Terminal:

```sh
pkgman install dist/bewake_screensaver-1.2.0-1-$(getarch).hpkg
```

Remove any copy from
`/boot/home/config/non-packaged/add-ons/Screen Savers` before testing the
packaged version, otherwise the unpackaged add-on may shadow it.

## Version and licence

- Version: 1.2.0
- Original 2003 version: Mattia Tristo
- Haiku update from 2009: Mattia Tristo and Gabriele Biffi
- Licence: GNU General Public License, version 2
