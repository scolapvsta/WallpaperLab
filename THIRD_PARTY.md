# Provenance and licenses

Wallpaper Lab is a derivative application distributed under GPL-3.0. The name is
provisional. Upstream notices remain in the source tree and release payloads.
Wallpaper Engine and Workshop content belong to their respective owners and are
not included in the source repository or application runtime.

## Engine and interface

- **Almamu/linux-wallpaperengine**, GPL-3.0: native renderer, PKG/TEX parsing,
  scenes, video, web and desktop integration. `vendor/engine` pins the upstream
  commit. All application changes are recorded in the ordered patches listed in
  `build-manifest.json`.
- **danihek/hellpaper**, copyright 2025 danihek, and **Belial02/hellpaper**, MIT:
  selector foundation, shaders, four layouts and animation. `src/selector.c` is
  modified for Wallpaper Lab; `vendor/hellpaper` pins the original. Full notice:
  `assets/LICENSE.hellpaper`.
- **raylib**, Ramon Santamaria and contributors, zlib/libpng: UI and image
  handling. Version 5.5 is pinned in `vendor/raylib`; license:
  `assets/LICENSE.raylib`. GIF decoding uses raylib's vendored stb_image with
  its source notices.
- **FreeType**, **Fontconfig**, **Pango/Cairo**: system font rendering, discovery,
  Unicode shaping, bidi and fallback, under their respective upstream licenses.
  Fontconfig registration of bundled fonts is limited to the application process.
- **Chromium Embedded Framework** and renderer dependencies retain their upstream
  licenses and attributions. CEF is downloaded by the upstream build. Beta
  packages include its notices and `CREDITS.html`.
- **libcurl**, curl license: bounded HTTPS transfers for static Steam CDN previews,
  provided as a system dependency resolved through pkg-config.
- **FFmpeg/ffprobe**: separately installed system tools for media metadata and
  thumbnails, subject to their package licenses. Imported media are not distributed.

## Patch inventory

| Patch | Purpose |
| --- | --- |
| `engine-integration.patch` | Native engine library integration |
| `engine-build-paths.patch` | CMake path quoting |
| `engine-audio-web.patch` | Stereo audio, web/SceneScript bridge, CEF lifecycle and OpenGL cover compatibility |
| `engine-display-layout.patch` | Physical panorama calibration |
| `engine-wallpaper-placement.patch` | Per-project placement |
| `engine-live-controls.patch` | Live controls and reduced renderer feed |
| `engine-scene-interaction.patch` | Scene parsing, clipping shaders and SceneScript interaction |
| `engine-project-controls.patch` | Output coordinates, crop and zoom |
| `engine-web-compatibility.patch` | Web and legacy parser compatibility |
| `engine-sound-controls.patch` | Scene playback, audio lifecycle and covered outputs |
| `engine-transitions-media-text.patch` | Transitions, media events and text integration |
| `engine-stable-transitions-image-controls.patch` | Transition stability and image controls |
| `engine-custom-animation-image-transform.patch` | Image transforms and shared transition customization |
| `engine-transition-handoff.patch` | Per-output captures and synchronized controller/renderer handoff |
| `engine-compositor-transitions.patch` | RGBA composition and antialiased iris |
| `engine-live-audio-mute.patch` | Live CEF/MPV audio mute |
| `raylib-app-id.patch` | Application desktop identity |

Patches live in `patches/`; application-side compositor rules are in
`src/desktop_surface.hpp`. Pins and SHA-256 hashes are in `build-manifest.json`.

Implementation contracts and reference documentation:

- [CEF OnPaint](https://cef-builds.spotifycdn.com/docs/131.3/classCefRenderHandler.html)
- [SceneScript sound transport](https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/ISoundLayer.html)
- [Media timeline events](https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/MediaTimelineEvent.html)
- [Media playback events](https://docs.wallpaperengine.io/en/scene/scenescript/reference/class/MediaPlaybackEvent.html)
- [Adobe transition categories](https://helpx.adobe.com/premiere/desktop/add-video-effects/effects-and-transitions-library/list-of-video-transitions.html),
  consulted for categories without importing Adobe code or assets.

## Steam Workshop

**Steamworks / Valve** provides the native Workshop API. The minimal flat ABI in
`src/steam_abi.hpp` was compared with Valve headers exposed through
Steamworks.NET (`CodeGen/steam`): ISteamUGC020, manual dispatch and Linux 4-byte
packing. References: [ISteamUGC](https://partner.steamgames.com/doc/api/ISteamUGC)
and [Steam API](https://partner.steamgames.com/doc/api/steam_api).

The proprietary `libsteam_api.so` is excluded from GPL source and runtime payloads.
The beta installer offers a separate download from Steamworks.js 0.4.0, with URL,
archive and library hashes pinned in `packaging/steam-runtime.json`. The Valve
component retains its own terms. No npm or JavaScript code is executed by this
preparation step.

WallpaperX was consulted for UGC query, tag, subscription and download behavior.
Wallpaper Lab does not launch its JavaScript bundle or executable. Downloads
remain managed by Steam in its library folders.

## Visual references and optional integration

Historical palette references include [Catppuccin Latte/Mocha](https://catppuccin.com/palette/),
[Dracula](https://draculatheme.com/contribute) and
[Rosé Pine](https://rosepinetheme.com/palette/) (MIT palettes). Adaptations are
independent rather than official ports.

Historical UI references include [Midnight](https://github.com/refact0r/midnight-discord),
[System24](https://github.com/refact0r/system24) and
[Comfy](https://github.com/Comfy-Themes/Spicetify). CSS and image assets from those
projects are not bundled. Current theme drawings, Pixel characters, backgrounds
and effects are implemented in application code. Additional visual references
are documented in [Themes](docs/THEMES.md).

[Caelestia CLI](https://github.com/caelestia-dots/cli) and
[Caelestia shell](https://github.com/caelestia-dots/shell) are installed separately.
The initial integration used `wallpaper --print`; the current helper generates
Material You palettes directly and publishes a compatible `scheme.json` without
replacing Caelestia package files.

The optional Python helper uses [materialyoucolor](https://github.com/T-Dynamos/materialyoucolor-python)
(MIT), based on [Material Color Utilities](https://github.com/material-foundation/material-color-utilities)
(Apache-2.0), and [Pillow](https://python-pillow.org/) (MIT-CMU/PIL notices).
These are prepared separately on feature activation, outside the base runtime.
The helper and additional surface tint are Wallpaper Lab code.

## Bundled fonts

Unmodified originals from Google Fonts are pinned by commit and SHA-256 in
`assets/fonts/manifest.json`. Full SIL Open Font License 1.1 notices, descriptions
and font files are under `assets/fonts/`, installed in the runtime's `fonts/`.

| Font | Attribution |
| --- | --- |
| Orbitron | The Orbitron Project Authors |
| Oxanium | Severin Meyer |
| Rajdhani | Indian Type Foundry |
| Silkscreen | The Silkscreen Project Authors / Jason Kottke |

No game or anime fonts, images or sprites are extracted or redistributed.

## Packaging boundary

The beta installer uses an allowlist of build outputs: application, engine, CEF,
kissfft, helpers, fonts and licenses. Other runtime dependencies are Arch system
packages. Development prefixes, user settings, Workshop assets, Steam account
data and credentials are excluded. Steamworks remains a separate optional
download. See [Releasing](docs/RELEASING.md) for source and binary packaging checks.
