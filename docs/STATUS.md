# Current beta status

As of 2026-11-21, the current release is **0.25.0-beta.1**. Wallpaper Lab is in
active testing and is **not a stable 1.0 product**. All development work through
the old 2.5.1 label is included; this is a release-policy correction, not a feature
or configuration downgrade.

## Version mapping and release policy

| Label | Meaning |
| --- | --- |
| 0.24.0-beta.1 | Earlier public beta, retained unchanged |
| v1.0.0 | Legacy experimental binary snapshot, formerly mislabeled stable; tag/assets retained and GitHub release reclassified as prerelease |
| 1.1.0–2.5.1 | Historical development labels used in commits and test reports |
| **0.25.0-beta.1** | Current consolidated beta, including those development features |

The executable, build manifest, installer, full source archive and release tag
use the same complete version. Subsequent releases use `0.x.y-beta.N` and GitHub's
**Pre-release** classification. Stable 1.0 requires an explicit maintainer decision;
a feature milestone or successful preview does not promote the project automatically.
Historical tags and test reports keep their original names. Old runtime directories
and published login bundles keep their original identity until explicitly upgraded.

## Included functionality

| Area | Current implementation |
| --- | --- |
| Library | Local image/GIF/video import, installed Workshop projects, shared search/filters, favorites, native previews, image/crop/position/zoom and creator properties |
| Steam Workshop | Native search, filters, creator information and downloads with Steam ownership/runtime requirements |
| Desktop rendering | Native scene/video/web engine, independent/mirror/panoramic outputs, GPU selection, monitor reconnection handling and fullscreen-pause policies |
| Carousel | Grid, River, Vertical and Wave layouts; hold mode and optional persistent mode |
| App appearance | Four designs, 38 palettes, app backgrounds, fonts, 22 interface languages and responsive navigation |
| Integrations | Independent Caelestia and native login activation; optional awww for local images/GIFs |
| Login wallpapers | Shared Library controls, independent login profiles, per-output selection, UI monitor placement, FPS, cached native stills and explicit live preview |
| Login editor | Clock, Date and Login UI; grid/snap, typography, colors, text effects, editable labels, dynamic Welcome and reset backups |
| Login presets | Ten editable native Astronaut designs, UI-only and complete wallpaper/UI presets; saved JSON collections |
| Login polish | Copy desktop settings for the same wallpaper; numeric RGB, optional eyedropper, color-only suggestions/Undo, per-character password animation and smooth control transitions |
| Pre-login runtime | Self-contained native wallpaper/UI bundle, greetd authentication, dedicated Hyprland session, publication/rollback and fallback paths |

The old SDDM integration, calendar/media widgets, analog clock choices and automatic
palette/light switches are retired. SDDM supplies design references only; the current
login implementation uses native rendering and greetd, not an SDDM/QML runtime.

## Known issues and limits

- Arch Linux x86_64 with Hyprland is the tested baseline. Other compositors,
  Windows and macOS are not certified. Rolling system-library changes can require
  rebuilding the binary; portable cross-distribution compatibility is not promised.
- Workshop compatibility depends on each project. Desktop web support does not
  imply web support at login: the native greeter excludes HTML/JavaScript projects.
  Login runs muted, so audio-reactive elements do not receive desktop audio.
- Native login remains optional and experimental. A real local greetd login and
  keyring unlock were observed, but hardware/driver/cold-boot coverage is limited.
  A Hyprland/Aquamarine DRM teardown crash after authentication remains unresolved.
  The desktop started in that observation; it is not evidence that every handoff
  is reliable. Separately reported machine resets have no established causal link
  to Wallpaper Lab and are not claimed fixed by this beta.
- A successful preview or mock greetd test does not validate PAM/VT handoff,
  system power operations or recovery on every installation. Read the login
  [setup and recovery guide](LOGIN_SCREEN.md) before activation.
- Workshop thumbnails and cached stills are not full animation. Use Live preview
  for real rendering. Color suggestions use one frame and cannot guarantee text
  contrast over every frame of a moving background.
- Optional Astronaut fonts may fall back when absent. Only redistributable fonts
  are bundled; original theme wallpaper assets are excluded.
- awww panoramic output is unsupported. Its actual daemon and every mixed-DPI
  handoff were not revalidated in the latest desktop test pass.

## Verification and work before stable 1.0

[Test results](../tests/RESULTS.md) link the actual dated checks. They distinguish
unit/integration coverage, isolated desktop tests, mock authentication and real
boot observations. Private logs and Workshop assets are excluded from the repository.

Before a stable release, prioritize resolving compositor teardown failures,
repeated cold-boot/login/fallback/rollback checks, physical monitor reconnect and
suspend/resume coverage, and clean installation/upgrades on additional supported
machines. Document remaining project compatibility limits and recovery behavior.
Only then should a maintainer explicitly approve stable 1.0.
