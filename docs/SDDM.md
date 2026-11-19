# SDDM integration preview

This is the first development phase of the optional Astronaut integration.
Wallpaper Lab configures and tests a real SDDM greeter in **test mode**. It does
not change the active system theme, authentication, services or autostart.
The downloadable 1.0 stable installer does not include this feature.

## Use

Install SDDM Qt6 and [Astronaut](https://github.com/Keyitdev/sddm-astronaut-theme)
with its documented Qt6 multimedia, SVG and virtual keyboard dependencies.
Keep Arch packages consistent through a full system upgrade when needed.
Wallpaper Lab also uses FFmpeg/ffprobe to validate selected media.

Build from the initialized submodules, then open the launcher. Under
**Settings → Integrations → SDDM**, enable the integration. Activation prepares
an isolated theme and runs a bounded offscreen greeter check. A successful check
reveals the **SDDM** navigation tab and settings section. Missing dependencies,
invalid media or QML load failures prevent first activation. The previous verified
selection survives a failed replacement. Diagnostics stay in the local state directory.

The SDDM page has two independent groups:

- **Wallpaper:** installed Astronaut `Backgrounds`, the configured local library
  and the configured/discovered Steam Workshop roots. Select one active background;
  the catalog is paginated in groups of 100. Fill-screen mode belongs to this group.
- **Login UI:** the pinned Astronaut presets, form position and login blur.
  Changing the preset preserves the selected wallpaper and explicit overrides.

The preview button opens the real greeter in test mode for at most 60 seconds.
No password entry or authentication is needed for preview testing. Closing the
preview early returns control immediately. Image/GIF/video backgrounds are copied
into the generated theme, preserving their animation where Qt supports the codec.

## Compatibility and limits

| Source | Current behavior |
| --- | --- |
| Installed Astronaut PNG/JPEG/WebP/GIF/video | Copied and used as the background |
| Wallpaper Lab local image/GIF/video project | Uses `lab_media_file`, not the thumbnail or HTML wrapper |
| Workshop video project | Uses the project's `file` media |
| Workshop scene/web/application project | Listed, selection rejected with an explicit explanation |
| Other SDDM UI themes | Not yet supported; the UI list contains Astronaut presets |

Native supported extensions: PNG, JPG/JPEG, WebP, GIF, AVI, MP4, MOV, MKV, M4V,
WebM. Qt decoder availability still matters. Other local image formats remain
unsupported here even if the desktop renderer supports them. One background is
active at a time; playlists and different backgrounds per login monitor are not
implemented. Desktop image adjustments, project properties and audio settings
are not exported. The Astronaut video path has no audio output.

The health check tests media metadata and QML component loading in offscreen
software mode. It is not proof of successful authentication, boot-time access,
all GPU drivers or every video codec. The native desktop engine is not started
in the greeter. A future renderer bridge needs a separate pre-login process,
GPU/display ownership and a compositor-compatible surface. Merely starting the
user's launcher earlier would not solve those requirements.

## Storage and lifecycle

- `$XDG_CONFIG_HOME/wallpaper-lab/sddm.json`: opt-in flag, `wallpaper` selection
  and settings, and independent `ui` selection and overrides.
- `$XDG_STATE_HOME/wallpaper-lab/sddm/prepared`: self-contained theme, including
  `Backgrounds/wallpaper.<extension>` and merged `theme.conf`.
- `previous`: the previous prepared theme after a successful replacement.
- `verified.json`: local check receipt. Theme/template or greeter/Qt library
  changes invalidate it. The launcher rechecks status periodically.
- `greeter-check.log`, `greeter-preview.log`, `last-error.json`: local diagnostics.

Standard XDG defaults apply when variables are unset. System theme discovery
honors SDDM `ThemeDir` configuration precedence, then looks for
`sddm-astronaut-theme`. No machine paths are compiled into the application.

The packaged template allowlist includes Main.qml, components, icons, preset
configs, metadata and the upstream license. Backgrounds, previews and upstream
fonts are excluded from the binary package. The locally installed Astronaut Fonts
directory is copied into the local prepared theme when present.

All helper calls use argument arrays. A nonblocking interprocess lock serializes
SDDM mutations. Staging and validation finish before replacing the current bundle;
configuration is saved after validation. The helper owns and reaps its bounded
greeter child processes. Desktop renderer ownership and handoff code is unchanged.
Disabling hides the tab and retains the prepared files for inspection.

## CLI and next phase

```sh
wallpaper-lab --sddm status
wallpaper-lab --sddm enable
wallpaper-lab --sddm wallpaper /absolute/path/to/library/project
wallpaper-lab --sddm preset black_hole
wallpaper-lab --sddm position left
wallpaper-lab --sddm crop true
wallpaper-lab --sddm blur false
wallpaper-lab --sddm preview
wallpaper-lab --sddm check
wallpaper-lab --sddm disable
wallpaper-lab --sddm-settings
```

For an installed Astronaut background, pass its absolute media path instead of
a project directory. CLI responses are JSON. `check` performs preparation and
validation and enables the integration on success, just like `enable`.

System publication is deliberately a future step: install the prepared theme in
a system-readable SDDM theme directory, back up configuration, verify access as
the `sddm` user and only then activate it. Native image/video playback would then
be independent of Wallpaper Lab or Steam starting after login. Reboot, real login,
permissions under the SDDM account and encrypted-home scenarios are untested.

See [validation results](SDDM_TEST_RESULTS.md).
