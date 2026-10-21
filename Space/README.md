# Space

Space is a native Haiku screen saver that renders a smooth, perspective-correct
flight through a three-dimensional star field.

![Space screen saver](Space.png)

This is a maintained port of David Enderson's Space 1.2 screen saver for BeOS.
It retains the original visual model and compatible saved-setting keys while
updating the implementation for Haiku R1/beta6-era systems.

## Features

- Configurable direction, star count, star diameter and speed
- Responsive configuration panel using the Haiku layout API
- Monotonic, frame-rate-independent animation at up to 60 frames per second
- Support for both full-screen and preference-panel preview modes
- Bounds-checked settings and safe handling of delayed frames

## Building

Build on Haiku with the development tools installed:

```sh
cd src
make
```

Copy the resulting `Space` add-on into the per-user screen saver directory:

```sh
mkdir -p "$HOME/config/non-packaged/add-ons/Screen Savers"
cp objects.*/Space "$HOME/config/non-packaged/add-ons/Screen Savers/Space"
```

Then open **Preferences → ScreenSaver** and select **Space**.

## Version and licence

- Version: 1.3.0
- Original author: David Enderson
- Licence: GNU General Public License, as specified by the original release

