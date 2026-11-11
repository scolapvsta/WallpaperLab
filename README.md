<p align="center"><img src="assets/wallpaper-lab.svg" width="96" height="96" alt="Wallpaper Lab icon"></p>

# Wallpaper Lab

Animated wallpapers, a native Steam Workshop browser, and a customizable desktop
carousel for **Arch Linux x86_64**. One application combines a raylib interface
with the linux-wallpaperengine renderer. GPL-3.0.

Version 1.0 includes named monitor selection, 25–300% live zoom, a compact
**power icon**, a single library window, and a selectable monitor for Caelestia
colours. Arcade includes vivid colours, animated pixel characters and cabinet-style
buttons. Interface labels and application error summaries follow all 22 supported
languages, including live language changes.
See the [user guide](docs/USER_GUIDE.md#monitor-selection-zoom-and-stopping).

## Download and install

[**Download the Arch Linux 1.0 installer**](https://github.com/scolapvsta/WallpaperLab/releases/latest/download/Wallpaper-Lab-Arch-x86_64.run)
 · [Release notes and checksums](https://github.com/scolapvsta/WallpaperLab/releases/tag/v1.0.0)

Open a terminal in the download folder and run:

```sh
bash Wallpaper-Lab-Arch-x86_64.run
```

Alternatively, download [Install-Wallpaper-Lab.sh](https://github.com/scolapvsta/WallpaperLab/releases/latest/download/Install-Wallpaper-Lab.sh) and run `sh Install-Wallpaper-Lab.sh`. This small helper downloads the latest stable installer, checks its published SHA-256 checksum and starts the guided setup. Checksums detect corruption; download only from the official repository.

The guided installer verifies its payload, checks Arch dependencies, offers to
install missing packages with `sudo pacman`, and adds Wallpaper Lab to your app
menu. **Run it as your normal user, without sudo.** App files are installed in
your user profile. No compilation, Node.js or npm installation is needed.

- Keep Arch up to date; this build requires **glibc 2.44 or later**.
- Full desktop rendering, multi-monitor behavior and integrated shortcuts are
  currently verified on **Hyprland**. Other desktops are not certified.
- Steam must be running and your account must own **Wallpaper Engine** for the
  Workshop. Install Wallpaper Engine through Steam so its scene assets exist.
- The installer offers a separate, checksum-pinned Valve Steamworks runtime
  download for Workshop access. It is not part of this GPL source repository.
- Local image/video imports work without Steam. A file picker, FFmpeg and fonts
  are checked by the installer. Your wallpaper library is never included.

The source tree also supports **awww for local images and animated GIFs**. Choose
it under **Settings → Playback**, or import a supported `awww` wallpaper hook
from Caelestia CLI. Transition controls then use awww's effects, duration, step,
FPS, origin and Y inversion. Videos and Workshop projects keep the native renderer.
Step and duration sliders appear directly below the transition selector. Optional
colours for awww images follow the installed Caelestia CLI's scheme and smart
selection, including its theme hooks.
This addition is a source/custom-build feature; the linked 1.0 installer predates it.

[Installation, updating, removal and troubleshooting →](docs/BETA.md)

## What is included

- Native scene, video and web wallpapers, local media imports and live controls.
- Workshop search, filters, creator information and Steam downloads.
- Independent monitor assignments, panoramic and mirror layouts.
- Four animated carousel layouts: Grid, River, Vertical and Wave.
- Default, Strawberry core, 2077 and Arcade: **38 palettes**, 16 app
  backgrounds, suggested fonts and an optional general font override.
- Plain switchers with 20 color choices, editable animations, desktop transitions and live mute.

Compatibility depends on the project features used;
not every Wallpaper Engine scene is supported. Optional Caelestia integration
requires Caelestia separately. Windows and macOS are not supported by this build.
Autostart and global shortcuts are configured explicitly inside the app.

## Report a bug

[Open a bug report](https://github.com/scolapvsta/WallpaperLab/issues/new?template=beta-bug.yml)
with your app version, desktop/compositor, GPU/driver, steps and expected result.
Include a public Workshop ID if relevant. Review logs before posting; remove
personal paths/account data and do not upload private wallpapers.

## Build from source

```sh
git clone --recurse-submodules https://github.com/scolapvsta/WallpaperLab.git
cd WallpaperLab
sudo pacman -S --needed base-devel cmake ninja git python pkgconf curl freetype2 pango fontconfig sdl2-compat ffmpeg mpv glew glfw lz4 libpulse wayland libx11 libxrandr nss at-spi2-core cups
./scripts/build.sh
python3 scripts/install.py
```

The upstream engine build downloads its pinned CEF distribution. System GPU
libraries remain supplied by Arch and your driver. See [release tooling](docs/RELEASING.md)
for building the same release installer, and the release's complete source archive
for the app plus pinned submodules.

## Documentation and credits

See [contributor instructions](AGENTS.md) for development conventions, process
lifecycle requirements and release checks.

[User guide](docs/USER_GUIDE.md) · [Themes](docs/THEMES.md) ·
[Architecture](docs/ARCHITECTURE.md) · [Tests](tests/RESULTS.md) ·
[Development history](docs/HISTORY.md) · [Windows port notes](docs/WINDOWS_PORT.md)

Built on [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine),
[Hellpaper](https://github.com/Belial02/hellpaper), raylib and Chromium Embedded
Framework. [Licenses and attribution](THIRD_PARTY.md). Wallpaper Lab is independent
of Valve and the Wallpaper Engine authors. Wallpaper Engine and Workshop assets
remain the property of their respective owners.
