# SDDM integration preview

This is the experimental Astronaut integration, including the visual library
and image editor introduced in development version 1.3.0. Development version
1.4.0 adds native scene previews and panoramic backgrounds across monitors.
Wallpaper Lab configures and tests a real SDDM greeter in **test mode**. It does
not change the active system theme, authentication, services or autostart.
The downloadable 1.0 stable installer does not include this feature.

## Use

Install SDDM Qt6 and [Astronaut](https://github.com/Keyitdev/sddm-astronaut-theme)
with its documented Qt6 multimedia, SVG and virtual keyboard dependencies.
Keep Arch packages consistent through a full system upgrade when needed.
Wallpaper Lab also uses FFmpeg/ffprobe to validate selected media and create
poster thumbnails. Source builds need Qt6 Shader Tools (`qt6-shadertools` on Arch)
for `qsb`; the compiled shader is included in installed runtimes. A build without
that optional tool can run the desktop app, but SDDM activation reports the missing
shader. Offscreen captures use Qt RHI/OpenGL with Mesa software rendering; the
Qt Quick `software` scene-graph backend cannot display shader effects.

Build from the initialized submodules, then open the launcher. Under
**Settings → Integrations → SDDM**, enable the integration. Activation prepares
an isolated theme and runs a bounded offscreen greeter check. A successful check
reveals **Integrations → SDDM** in the launcher. The Integrations tab also appears
when Caelestia is enabled; each panel is shown only while its integration is enabled. Missing dependencies,
invalid media or QML load failures prevent first activation. The previous verified
selection survives a failed replacement. Diagnostics stay in the local state directory.

The SDDM panel inside Integrations contains three views:

- **Wallpapers:** a card grid using the main library's theme, card sizing and
  inspector controls. It combines installed Astronaut `Backgrounds`, the configured
  local library and configured/discovered Steam Workshop roots. Search titles,
  filter by origin/type or favorites, and browse 50 items per page. Import uses the
  shared local library; SDDM favorites remain independent of the desktop carousel.
  Compatible card selection updates the prepared background. Unsupported/incomplete
  cards show their project poster and a limitation, without replacing the background.
- **Login UI:** cards captured from the real greeter with each Astronaut preset's
  original background and defaults. Selecting one preserves the chosen wallpaper
  and its edits. The inspector offers form position and login blur overrides.
- **Final preview:** a cached real-greeter capture of the selected wallpaper,
  saved image adjustments, login preset and UI overrides. Choose **Display → Mirror**
  for the same background on each screen or **Panoramic** for one canvas spanning
  the connected screens. A panoramic capture includes all screen regions. Open the bounded live
  preview to see animation, current time and the actual interactive layout.

The wallpaper inspector updates its image preview immediately while editing.
**Save and apply** validates and saves the changes for that SDDM wallpaper;
**Cancel** restores its saved values. **Reset image** restores defaults in the
editor, which are saved only when applied. Changing wallpaper is blocked while
there are unsaved edits, and reopens its inspector. Desktop wallpaper profiles
and the original media files are not modified. Returning to a previously edited
SDDM wallpaper restores its saved image controls.

Controls match the desktop image editor's ranges: brightness, contrast,
saturation, temperature, hue, gamma, vignette, rotation, horizontal/vertical
mirroring, 25–300% zoom, horizontal/vertical crop position and fill/fit. Colour
operations follow the desktop renderer; the launcher preview and greeter share
one shader implementation. The native greeter keeps image/GIF/video playback
while applying these controls. Native scene frames also pass through these controls.
A login preset may reserve a separate form panel
or apply its own dimming/blur; the final preview shows that composition.

The inspector's **Preview** opens only the selected wallpaper; the final preview
button opens the real login layout. Both use greeter test mode for at most 60
seconds and can be closed early. No authentication is needed. Poster thumbnails
and preset/final captures are static; the live preview verifies animation and
screen-size-dependent layout. Captures are generated in a background worker and
cached locally, including the current user's test-mode login display name.

## Native scenes and multiple monitors

Selecting a valid scene project starts a separate `wallpaper-lab --render` process
for validation. It renders through a hidden OpenGL window and supplies an atomic
RGB frame feed to the QML background. This is actual scene rendering, not a video
export or a Workshop thumbnail. The same renderer supplies every greeter view;
the existing desktop renderer is not reused or replaced. Wallpaper Engine's
installed assets and the original project must remain accessible.

The scene preview is currently capped at **30 FPS and 2560×1440**, preserving the
virtual canvas aspect ratio. For example, a 4480×1440 desktop produces a
2560×823 feed. It is an experimental CPU-readback bridge, not a zero-copy GPU
texture integration. Audio reactivity, sound, pointer interaction and
parallax are disabled; native non-interactive animation and particles remain.
Desktop property overrides and dependency materialization are not imported.
Scenes that the native engine cannot load fail validation and preserve the last
verified selection. Scene cards/editor still use project posters; the final and
live previews use the rendered scene.

Panoramic cropping uses the greeter's complete Qt screen geometry, including
negative positions and unequal sizes. Each view samples its own region after
applying the image controls to the whole canvas. The login UI remains independently
laid out on each monitor. This follows logical compositor coordinates, not the
desktop editor's custom physical-size/bezel calibration. Screen discovery for
the captured preview and scene render size currently comes from the active
Hyprland session. Validate again after changing that layout.

Video views seek toward a shared clock in panoramic mode. GIFs retain Qt's
normal playback. Exact frame lock across displays/refresh rates is not promised.
SDDM creates a separate view and screen model for each screen, so relying only
on `screenModel.geometry(-1)` would not recover the whole desktop in current
SDDM. The implementation uses Qt's complete screen list instead.
See [SDDM GreeterApp](https://github.com/sddm/sddm/blob/develop/src/greeter/GreeterApp.cpp)
and [Qt Application.screens](https://doc.qt.io/qt-6/qml-qtquick-application.html#screens-prop).

## Compatibility and limits

| Source | Current behavior |
| --- | --- |
| Installed Astronaut PNG/JPEG/WebP/GIF/video | Copied and used as the background |
| Wallpaper Lab local image/GIF/video project | Uses `lab_media_file`, not the thumbnail or HTML wrapper |
| Workshop video project | Uses the project's `file` media |
| Local/Workshop scene project | Validated native renderer feed in bounded test-mode previews |
| Workshop web/application project | Listed, selection rejected with an explicit explanation |
| Other SDDM UI themes | Not yet supported; the UI list contains Astronaut presets |

Native supported extensions: PNG, JPG/JPEG, WebP, GIF, AVI, MP4, MOV, MKV, M4V,
WebM. Qt decoder availability still matters. Other local image formats remain
unsupported here even if the desktop renderer supports them. One background is
active at a time; playlists and different backgrounds per login monitor are not
implemented. Desktop image adjustments are not imported automatically: SDDM has independent
profiles. Project properties and desktop audio settings are not exported. The Astronaut video path has no audio output.

The health check tests media metadata, QML component loading and the shader in
offscreen RHI/OpenGL mode. It is not proof of successful authentication, boot-time access,
all GPU drivers or every video codec. Scene selection additionally checks real
renderer readiness and delivery of a valid frame to QML. The bridge currently
runs in the logged-in user's session. Pre-login deployment still needs an
appropriate renderer owner, readable exported assets and display/GPU access.

## Storage and lifecycle

- `$XDG_CONFIG_HOME/wallpaper-lab/sddm.json`: opt-in flag, `wallpaper` selection
  and per-source image profiles, `wallpaper.display_mode`, and independent `ui`
  selection and overrides.
- `$XDG_CONFIG_HOME/wallpaper-lab/sddm-library.json`: independent SDDM favorites.
- `$XDG_STATE_HOME/wallpaper-lab/sddm/prepared`: theme, including
  `Backgrounds/wallpaper.<extension>` and merged `theme.conf`.
- `scene.json`: local renderer/project/assets references and bounded feed dimensions
  for a selected native scene. The project/assets are not copied. A rendered PPM
  fallback is retained, so a manually opened theme without the helper shows a
  rendered still rather than starting another process itself.
- Active scene frames live in a private temporary directory under `XDG_RUNTIME_DIR`
  (the OS temporary directory when unavailable), renewed at up to 30 FPS and
  removed on normal completion. No frame stream is written into Workshop projects.
- `previous`: the previous prepared theme after a successful replacement.
- `previews`: generated wallpaper posters and real greeter captures, keyed by
  source metadata, preset/template/shader revisions and saved settings.
- Temporary capture themes are isolated from the prepared theme and removed when
  their bounded worker finishes. Failed captures leave the prepared theme intact.
- `verified.json`: local check receipt. Theme/template or greeter/Qt library
  changes invalidate it. The launcher rechecks status periodically.
- `greeter-check.log`, `greeter-preview.log`, `scene-check.log`, `scene-preview.log`,
  `last-error.json`: local diagnostics.

Standard XDG defaults apply when variables are unset. System theme discovery
honors SDDM `ThemeDir` configuration precedence, then looks for
`sddm-astronaut-theme`. No machine paths are compiled into the application.

The packaged template allowlist includes Main.qml, components, icons, preset
configs, metadata, the Wallpaper Lab background component/compiled shader and the
upstream license. Backgrounds, previews and upstream
fonts are excluded from the binary package. The locally installed Astronaut Fonts
directory is copied into the local prepared theme when present.

All helper calls use argument arrays. A nonblocking interprocess lock serializes
SDDM mutations. Staging and validation finish before replacing the current bundle;
configuration is saved after validation. The helper owns and reaps its bounded
greeter child processes. Desktop renderer ownership and handoff code is unchanged.
The scene child is stopped only after checking PID, start time, executable and
render mode; pidfds avoid reuse races. A parent-death signal also terminates it
if its helper is killed abruptly. Failed scene preparation never publishes its
candidate theme. The active desktop wallpaper stays running throughout.
Disabling removes the SDDM panel and retains the prepared files for inspection.
The Integrations tab remains while Caelestia is enabled. If both are disabled,
the tab disappears and an open Integrations page returns to the library.

## CLI and next phase

```sh
wallpaper-lab --sddm status
wallpaper-lab --sddm enable
wallpaper-lab --sddm wallpaper /absolute/path/to/library/project
wallpaper-lab --sddm preset black_hole
wallpaper-lab --sddm position left
wallpaper-lab --sddm crop true
wallpaper-lab --sddm blur false
wallpaper-lab --sddm display-mode span
wallpaper-lab --sddm display-mode mirror
wallpaper-lab --sddm preview-wallpaper
wallpaper-lab --sddm preview
wallpaper-lab --sddm check
wallpaper-lab --sddm disable
wallpaper-lab --sddm-settings
wallpaper-lab --integrations
```

`--integrations` opens the shared management page, or activation settings when
no integration is enabled. `--sddm-settings` remains an alias that selects the
SDDM panel when enabled.

Image edits can also be submitted as a complete profile using
`--sddm edit '{"source":"/absolute/project/path","values":{"brightness":0.2,"zoom":1.2}}'`.
Omitted values use defaults; source must match the selected wallpaper. Invalid
values and failed validation preserve the previous verified configuration.

For an installed Astronaut background, pass its absolute media path instead of
a project directory. CLI responses are JSON. `check` performs preparation and
validation and enables the integration on success, just like `enable`.

System publication is deliberately a future step: install the prepared theme in
a system-readable SDDM theme directory, back up configuration, verify access as
the `sddm` user and only then activate it. Native image/video playback would then
be independent of Wallpaper Lab or Steam starting after login. Reboot, real login,
permissions under the SDDM account and encrypted-home scenarios are untested.
Native scenes also require packaging their dependencies and supervising the hidden
renderer under the greeter's account/display session; copying only `prepared`
is insufficient. The login/password/PAM code remains upstream.

See [media validation results](SDDM_TEST_RESULTS.md) and the
[shared Integrations UI checks](INTEGRATIONS_TEST_RESULTS.md), and the
[visual library/editor checks](SDDM_LIBRARY_TEST_RESULTS.md), plus the
[scene and panorama checks](SDDM_SCENE_TEST_RESULTS.md).
