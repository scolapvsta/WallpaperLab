# SDDM native scene and panoramic preview validation

Source version: **1.4.0**, 2026-11-19. This extends the
[visual library/editor checks](SDDM_LIBRARY_TEST_RESULTS.md). Fixtures, screenshots,
scene media, runtime frame feeds, machine configuration and logs remain local.

## Build, catalog and process checks

- Release engine/application builds and Qt shader compilation passed.
- **20/20 CTest checks passed**, including 12 SDDM unit tests. New coverage checks
  packed-scene discovery, missing renderer/assets without losing the prior
  selection, display-mode persistence, invalid-mode rejection, negative screen
  positions, duplicate screen rectangles and bounded portrait/panorama feed sizes.
- Translation coverage passed: **457 messages, 146 diagnostic routes, 22 locales**.
- The ordered engine patch series validated with the new scene-feed patch last.
  Existing local engine/raylib changes and upstream pins were preserved.
- Native lifecycle smoke checks passed over **106 catalog entries**, including
  scene and video previews and preservation of a foreign process. The personal
  desktop wallpaper was not replaced by the isolated preview tests.

## Scene bridge

A locally installed animated 2D Workshop scene produced distinct native RGB frames
without a mapped renderer window. `tests/sddm_scene_graphics_integration.py`
then selected it through the SDDM helper, validated it, enabled panorama and ran
two real QML login views using one hidden renderer.

The two captured greeter frames changed by **1.0153** summed mean RGB levels;
both views reported successful scene-image loading. The underlying feed changed
as well. This verifies actual animation reaching the greeter, not a poster or
pre-recorded video. The hidden renderer had no mapped compositor window.

Normal completion stopped/reaped the renderer and removed the temporary feed.
A deliberately malformed scene failed preparation while preserving the previous
verified configuration. Abruptly killing an owned helper also stopped its renderer
through the parent-death signal. Renderer signaling checks PID, start time,
executable and render mode and uses pidfds.

## Panoramic geometry and physical monitors

`tests/sddm_span_graphics_integration.py` compared a complete virtual canvas with
two adjacent greeter views using a synthetic asymmetric image and image controls.
Their **mean pixel difference was 0**. Left and right views displayed different
regions. Negative coordinates, unequal sizes and a vertical offset passed, as
did a composition with two actual login forms. A changing-colour H.264 clip
showed **0 summed RGB difference** at matching samples on both views after clock
correction; this is not a guarantee of frame lock on arbitrary hardware.

A bounded live test on two physical outputs created **two greeter windows** at
1920×1080 and 2560×1440. Both reported the same 4480×1440 desktop rectangle and
their respective screen rectangle. One 2560×823 scene feed supplied both windows,
with no extra visible renderer. Captures were visually inspected for continuous
background geometry and separately positioned login forms. The greeter and
temporary renderer were closed after the test.

Qt screen enumeration is used because current SDDM provides one `screenModel`
per view, as seen in [GreeterApp](https://github.com/sddm/sddm/blob/develop/src/greeter/GreeterApp.cpp).
The relevant Qt properties are documented under
[Application.screens](https://doc.qt.io/qt-6/qml-qtquick-application.html#screens-prop)
and [Screen](https://doc.qt.io/qt-6/qml-qtquick-screen.html).

## Launcher and local installation

`tests/sddm_panorama_ui_integration.py` passed at 1264×900: the final-preview
toggle persisted panoramic mode, retained a ready integration and generated a
native combined scene capture. The capture was visually inspected. Each login
view is laid out at its screen's logical size before being scaled into the
thumbnail, preserving the form and font proportions. The panorama pixel suite
and SDDM unit checks passed again after this capture adjustment.

An initial pointer-driven UI attempt ended before reaching Final preview, with
no exit code recorded or crash diagnostic. Three subsequent independent runs
completed, including the final adjusted capture. The initial exit's cause remains
unresolved; the successful runs do not establish that it cannot recur.

The user-local installation was upgraded from **1.3.0 to 1.4.0**, with a complete
configuration/prepared-theme backup and a separate runtime. The replacement
desktop renderer was verified alive with the same wallpaper before the old
renderer was signaled. Settings, SDDM selections and desktop wallpaper profiles
remained byte-identical to their backups. The installed launcher reported 1.4.0,
SDDM validation remained ready, and the updated integration page was reopened.

## Regressions and limits

The original image shader pixel suite passed, including brightness, grayscale,
mirroring, zoom margins, additional transforms and transparent PNGs. Mean errors
were **0.0290** unchanged, **0.00023** brightness, **0** mirroring and **0.0158**
transparency on the 0–255 scale. Native PNG/H.264/GIF/Workshop-video checks passed;
video and GIF frames changed. Integration enable/disable routing still passed.

This validates the tested native 2D scene, not every Workshop project. Unsupported
engine features, missing assets and unresolved project dependencies can still fail
validation. Scene previews are limited to 30 FPS and a 2560×1440 bounding box and
perform CPU readback; they do not share a GPU texture with Qt. The frame rate is
a cap, not a measured performance guarantee. Audio, mouse interaction and parallax
are disabled; desktop scene properties and custom physical monitor calibration
are not exported. Web/application projects remain unsupported.

There was no system-theme/PAM/service/autostart modification and no real login or
reboot. Pre-login file access, encrypted homes, display/GPU ownership and supervision
under the SDDM account remain a separate deployment phase. Scene projects/assets
are referenced locally rather than copied into a self-contained system bundle.
