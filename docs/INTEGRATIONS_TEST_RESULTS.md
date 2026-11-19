# Shared Integrations UI validation — 2026-11-19

Development version: 1.2.1. Arch Linux x86_64 / Hyprland Wayland.
The existing SDDM greeter and desktop renderer behavior is unchanged.

## Behavior

Settings contains independent Caelestia and SDDM switches. The Integrations
launcher tab follows Workshop and exists only while at least one integration is
enabled. Its sidebar contains only enabled integrations. Caelestia configuration
and SDDM wallpaper/login controls moved here from Settings. SDDM retains its health
check before enabling controls. Disabling the visible panel selects the remaining
integration. Disabling the last returns the main page to Library without closing
an open Settings dialog. Previously saved integration preferences are preserved.

## Checks

- Release build succeeded, including the optional Caelestia helpers now copied
  alongside the development binary as they already are in installed runtimes.
- CTest: all 20 tests passed, including catalog, process lifecycle, preferences,
  localization, Caelestia publication/restore and SDDM staging/rollback checks.
  The Caelestia legacy fixture explicitly overrides the optional helper path so
  it remains isolated when a real helper exists beside the test executable.
- Native lifecycle integration passed: 106 catalog entries, native scene/video
  preview startup and preservation of an unrelated process on Stop.
- `tests/integrations_ui_integration.py`: real launcher, pointer input and isolated
  XDG paths tested no integrations, Caelestia only, SDDM only and both enabled;
  navigation placement, saved palette/preset changes, disabling the current panel,
  hiding the last tab while retaining Settings, re-enabling saved preferences,
  and preventing the library search shortcut from entering a hidden search field.
- Screenshots of activation settings and the separate management panels were
  inspected at 1400 × 900. The page uses the existing scrollable panel controls.
- Python syntax and `git diff --check` passed. Existing localization is reused.

The UI test substitutes only Caelestia dependency preparation with a local fixture;
there is no active wallpaper in its isolated state and no palette is published to
the user's shell. Real Caelestia publication behavior remains covered by CTest.
SDDM activation in the UI test uses the real installed Qt6 greeter in test mode.
The earlier [SDDM media report](SDDM_TEST_RESULTS.md) records image/GIF/video playback;
media backend behavior was not changed for this UI reorganization.

Screenshots, logs and generated data remain under ignored `local/` directories.
The development preview is separate from the installed user launcher. No system
SDDM settings, autostart entries or shortcuts were enabled or modified.
