# SDDM visual library and image editor validation

Source version: **1.3.0**, 2026-11-19. This extends the
[SDDM media checks](SDDM_TEST_RESULTS.md) and
[Integrations routing checks](INTEGRATIONS_TEST_RESULTS.md).
Generated media, captured login display names, screenshots, logs and installation
backups remain in ignored local or XDG directories.

## Automated checks

- Release application build, Qt shader compilation and **20/20 CTest checks**
  passed. The SDDM unit suite contains ten tests covering catalog containment,
  activation, missing dependencies/shader, rollback, independent wallpaper/UI
  settings, per-source image profiles, rejected invalid/stale edits, separate
  favorites, cached posters and handled preview-close exits. Those unit tests use a fake greeter; the native
  checks below exercise the real Qt6 greeter.
- Translation audit passed for **456 messages across 22 locales**, including the
  new library/editor messages. This is not a visual layout check of every locale.
- Native catalog/process smoke checks passed with **106 catalog entries**, scene
  and video previews, and preservation of a foreign process with a matching
  recorded start time. Existing engine/raylib worktrees and pins were retained.

## Real greeter and image controls

`tests/sddm_graphics_integration.py` passed with an isolated local image, H.264
video, animated GIF and synthetic Workshop video project. Video and GIF captures
changed between frames. The launcher health gate showed the integration only
after successful activation, and hid it after disabling.

`tests/sddm_image_graphics_integration.py` renders the actual QML background with
the compiled shader using Qt RHI/OpenGL and Mesa software rendering. Pixel checks
cover unchanged geometry, brightness, grayscale saturation, horizontal mirroring,
zoom-out black margins, rotation/hue/temperature/gamma/vignette, transparency and
the combined login composition. Captures use a fixed 1280×720 child item, since
the test greeter controls its root window size.

All **10 Astronaut UI presets** also produced real cached greeter captures with
their installed background media and preset defaults. This confirms capture
generation, not successful authentication or every interactive control.

Mean absolute RGB errors on the 0–255 scale were **0.0461** for the unchanged
image, **0.00023** for brightness, **0** for mirroring and **0.0239** for
transparency composited over black; the acceptance threshold was 2. The other
checks passed their geometry, channel-equality or composition assertions.

The launcher and greeter use the same colour/geometry shader source, with separate
handling for raylib's straight-alpha textures and Qt's premultiplied textures.
Qt documents these rendering requirements in
[ShaderEffect](https://doc.qt.io/qt-6/qml-qtquick-shadereffect.html) and the capture
API in [Item.grabToImage](https://doc.qt.io/qt-6/qml-qtquick-item.html#grabToImage-method).

## Launcher checks

The isolated SDDM library pointer suite exercised local/source filters,
independent favorites, immediate draft brightness changes, explicit saving,
unsupported scene inspection, UI selection without changing the wallpaper
profile, login position, final capture and a real live greeter window. Screenshots
were inspected at 1400×940 and at a 1264×900 half-screen size, including the
Cyberpunk header and resizable editor. Desktop profiles and renderer state were
not created by the isolated suite.

The initial complete run exposed error text when closing the greeter; a later
run reproduced the cause: this SDDM build handles SIGTERM and exits with status
255. The helper now accepts that status only when the log confirms both successful
QML loading and SIGTERM. A separate native offscreen child-process check passed
with the final helper, preserving readiness after closing. Unit checks reject
status 255 without either marker. Status-bar text no longer queries the same
mutable diagnostic buffer repeatedly while drawing.

Later pointer reruns covered saving and the final capture, but a complete final
SDDM-library pointer rerun is not claimed: desktop focus/cursor changes interrupted
some runs, and one earlier window exited during navigation without a diagnosed
cause. The harness now waits for compositor focus transitions and verifies focus
and cursor position before clicking. Repeated tab navigation did not reproduce
the exit. The final **shared Integrations pointer suite passed completely**,
covering both toggles, conditional routing, independent choices, disabling the
last integration and re-enabling retained preferences.

## Local installation

The installed user launcher was upgraded from 1.2.2 to **1.3.0** using a separate
versioned runtime. Its helper, QML and shaders matched the verified build; runtime
dependency resolution, `--version` and a 106-entry catalog check passed. Existing
configuration and the previous prepared SDDM theme were backed up first.

The same desktop wallpaper started successfully in the new runtime before the
old renderer was retired. PID, start time and executable were verified and
signaling used pidfds. The installation receipt and menu launcher target were
updated. Settings, SDDM choices and desktop image-profile files remained
byte-for-byte unchanged. The installed SDDM health check passed with **118
wallpapers and 10 UI presets**, and the actual installed launcher reopened on
Integrations → SDDM. The previous runtime and configuration backups were retained.

## Scope and limits

The grid and inspector follow the main library's styling and sizing. Wallpaper,
UI-preset and final captures are static; the bounded live preview exercises
animation and the interactive login layout. Native Workshop scenes/web projects
have catalog posters but remain unsupported as greeter backgrounds.

Tests cover the available Linux/Hyprland, Qt6 and installed Astronaut environment.
They do not establish compatibility with every GPU, decoder, preset/locale
combination or SDDM theme. There was no PAM authentication, system-theme
publication, reboot or pre-login test. Desktop profile editing, autostart and
shortcuts are outside this change.
