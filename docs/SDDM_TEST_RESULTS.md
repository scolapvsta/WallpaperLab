# SDDM integration validation — 2026-11-19

Development version: 1.2.0. Environment: Arch Linux x86_64, Hyprland Wayland,
SDDM 0.21.0-7, Qt6 6.12.0, installed Astronaut theme. Upstream template pin:
`abb3163c724935af888ba5ea9ac0c4f22afd8048`. No engine/raylib pins or patches changed.

## Automated checks

- Release CMake build completed.
- CTest: **20/20 passed**, including catalog, preferences, popup, source resolution,
  awww process lifecycle, patch preservation and the new SDDM integration suite.
- SDDM unit suite: five tests cover native/local/scene catalog separation, rejected
  path traversal, missing prerequisites, independent wallpaper/UI selection,
  failed QML replacement, tamper invalidation, source removal after copying,
  disable behavior and rollback after publication failure.
- Localization audit: 450 entries, 144 diagnostic routes, 22 interface languages.
- Existing native process integration: 106 catalog entries, successful scene/video
  previews, and unrelated process preserved by Stop. Desktop replacement was not
  repeated because renderer behavior is unchanged.
- Python compilation and `git diff --check` passed.

## Real greeter and graphical checks

`tests/sddm_graphics_integration.py` ran with isolated XDG configuration/state.
It generated test media locally and exercised the built application's CLI plus
`/usr/bin/sddm-greeter-qt6 --test-mode`:

- Activation and prepared-theme validation passed.
- Local PNG, H.264 MP4 and animated GIF rendered behind the login form.
- Captured video/GIF frames changed between samples, confirming animation.
- A synthetic Workshop video project resolved its `file` and passed validation.
- Changing Astronaut UI to `black_hole` preserved the selected background.
- The real launcher exposed SDDM after successful activation and hid the tab after
  disable. Screenshots were inspected. Settings navigation fits a short window;
  the content panel scrolls to additional controls.
- The runtime installation allowlist contains the helper, QML/components/icons,
  presets and license. It excludes Astronaut backgrounds, fonts and previews.
  The helper from that installed template tree also passed a real greeter check.

All screenshots, media, logs, test state and build outputs remain in ignored
`local/` directories. Public test fixtures are generated or minimal synthetic data.
The existing user launcher and desktop wallpaper were left running.

## Limits

No display-manager restart, logout, reboot, PAM login or authentication attempt was
performed. The system SDDM configuration and active theme were not changed.
A graphical preview runs under the current user, not the system `sddm` account.
Boot-time permissions, encrypted homes, alternate display servers, other GPUs,
all codecs, native engine scenes/web content and arbitrary non-Astronaut themes
remain unverified or unimplemented as described in [SDDM.md](SDDM.md).
The installed stable application was not upgraded and no new binary release was
published; this feature is available in the development build/source.

SDDM test mode reports an expected missing daemon socket. This system also reports
a missing C-locale Hunspell dictionary and an upstream QML property override warning;
neither prevented component loading or the tested native media playback.
