<p align="center"><img src="assets/wallpaper-lab.svg" width="96" height="96" alt="Wallpaper Lab icon"></p>

# Wallpaper Lab

Animated wallpapers, a native Steam Workshop browser, and a customizable desktop
carousel for **Arch Linux x86_64**. One application combines a raylib interface
with the linux-wallpaperengine renderer. GPL-3.0.

**Current version: 0.25.0-beta.1 — open beta, not a stable 1.0 release.**
This beta consolidates all work previously labeled 1.x through 2.5.1, including
the native login-screen editor. The lower number changes the release policy,
not the feature set or preference format. Future builds remain beta until the
maintainer explicitly approves stable 1.0.

See [current status, known issues and the version mapping](docs/STATUS.md).
Historical tags and reports retain their original labels. The former `v1.0.0`
release is now classified as a legacy experimental prerelease; it is not the
recommended installer and does not contain the current features.

## Download and install

[**Download the Arch Linux beta installer**](https://github.com/scolapvsta/WallpaperLab/releases/download/v0.25.0-beta.1/Wallpaper-Lab-0.25.0-beta.1-Arch-x86_64.run)
 · [Release notes and checksums](https://github.com/scolapvsta/WallpaperLab/releases/tag/v0.25.0-beta.1)

Open a terminal in the download folder and run:

```sh
bash Wallpaper-Lab-0.25.0-beta.1-Arch-x86_64.run
```

Alternatively, download [Install-Wallpaper-Lab.sh](https://github.com/scolapvsta/WallpaperLab/releases/download/v0.25.0-beta.1/Install-Wallpaper-Lab.sh) and run `sh Install-Wallpaper-Lab.sh`. This small helper downloads this exact beta installer, checks its published SHA-256 checksum and starts the guided setup. Checksums detect corruption; download only from the official repository.

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

This beta also supports **awww for local images and animated GIFs**. Choose
it under **Settings → Playback**, or import a supported `awww` wallpaper hook
from Caelestia CLI. Transition controls then use awww's effects, duration, step,
FPS, origin and Y inversion. Videos and Workshop projects keep the native renderer.
Step and duration sliders appear directly below the transition selector. Optional
colours for awww images follow the installed Caelestia CLI's scheme and smart
selection, including its theme hooks.
The beta includes these controls; awww itself is an optional external dependency.

[Installation, updating, removal and troubleshooting →](docs/BETA.md)

## What is included

- Native scene, video and web wallpapers, local media imports and live controls.
- Workshop search, filters, creator information and Steam downloads.
- Independent monitor assignments, panoramic and mirror layouts, live image edits,
  position/zoom, rendering GPU selection and output-aware fullscreen pause.
- Monitor reconnection handling and optional persistent carousel controls.
- Independent Caelestia and Login screen activation in Settings.
- Four animated carousel layouts: Grid, River, Vertical and Wave.
- Default, Strawberry core, 2077 and Arcade: **38 palettes**, 16 app
  backgrounds, suggested fonts and an optional general font override.
- Plain switchers with 20 color choices, editable animations, desktop transitions and live mute.

Compatibility depends on the project features used;
not every Wallpaper Engine scene is supported. Optional Caelestia integration
requires Caelestia separately. Windows and macOS are not supported by this build.
Autostart and global shortcuts are configured explicitly inside the app.

## Native login screen

Enable **Settings → Integrations → Login screen** to reveal its separate launcher
tab. Caelestia remains independently configurable under **Integrations**.

The native editor includes a shared wallpaper library, an editable base UI and ten
[Astronaut UI presets](docs/ASTRONAUT_PRESETS.md),
movable clock/date/login widgets, magnetic alignment and independent fonts,
colours and text effects. The wallpaper tab also contains display settings.
It saves independent image adjustments and single/mirror/panoramic output choices,
with independent wallpapers in single mode and the login UI on the primary,
secondary or all monitors. Local images, animated GIFs, videos and compatible
Steam scenes use Wallpaper Lab's own renderer. Cached native stills preserve the
scene aspect ratio; **Live preview** renders the real project
for up to 60 seconds. Its interactive fields accept test text without authenticating
or saving the input. Named UI-only and complete wallpaper/UI presets can be saved
and reloaded. **Copy settings from Library** imports the selected project's desktop
image edits, position/zoom, resolution and creator options into the login draft.
Numeric RGB controls, a screen eyedropper, color-only suggestions with Undo and
widget/UI reset actions complete the editor. Password dots, focus and menus animate
without rebuilding the entire overlay each frame.

**Apply at next boot** exports a self-contained, administrator-owned bundle and
configures greetd with a dedicated Hyprland session. greetd handles PAM and starts
the selected desktop; Wallpaper Lab draws and controls the login screen directly.
The desktop launcher does not have to run. No SDDM, QML or frame-file bridge is used.
This is an experimental, opt-in beta feature: read the [setup, recovery and limitations](docs/LOGIN_SCREEN.md)
before activation. A successful desktop preview is not evidence of a successful
cold boot. A reported Hyprland/Aquamarine crash during greeter teardown remains
under investigation; see [known issues](docs/STATUS.md#known-issues-and-limits).

## Report a bug

[Open a bug report](https://github.com/scolapvsta/WallpaperLab/issues/new?template=beta-bug.yml)
with your app version, desktop/compositor, GPU/driver, steps and expected result.
Include a public Workshop ID if relevant. Review logs before posting; remove
personal paths/account data and do not upload private wallpapers.

## Build from source

```sh
git clone --recurse-submodules https://github.com/scolapvsta/WallpaperLab.git
cd WallpaperLab
sudo pacman -S --needed base-devel cmake ninja git python pkgconf curl freetype2 pango fontconfig sdl2-compat ffmpeg mpv glew glfw libxkbcommon lz4 libpulse wayland libx11 libxrandr nss at-spi2-core cups
./scripts/build.sh
python3 scripts/install.py
```

The optional system login feature additionally requires `greetd`, `greetd-agreety`,
Hyprland with Lua configuration support, D-Bus and polkit. Its editor and preview
work without greetd. See [native login setup](docs/LOGIN_SCREEN.md).

The upstream engine build downloads its pinned CEF distribution. System GPU
libraries remain supplied by Arch and your driver. See [release tooling](docs/RELEASING.md)
for building the same release installer, and the release's complete source archive
for the app plus pinned submodules.

## Documentation and credits

See [contributor instructions](AGENTS.md) for development conventions, process
lifecycle requirements and release checks.

[User guide](docs/USER_GUIDE.md) · [Themes](docs/THEMES.md) ·
[Architecture](docs/ARCHITECTURE.md) · [Tests](tests/RESULTS.md) ·
[Beta status and roadmap](docs/STATUS.md) · [Development history](docs/HISTORY.md) · [Windows port notes](docs/WINDOWS_PORT.md)

Built on [linux-wallpaperengine](https://github.com/Almamu/linux-wallpaperengine),
[Hellpaper](https://github.com/Belial02/hellpaper), raylib and Chromium Embedded
Framework. [Licenses and attribution](THIRD_PARTY.md). Wallpaper Lab is independent
of Valve and the Wallpaper Engine authors. Wallpaper Engine and Workshop assets
remain the property of their respective owners.
