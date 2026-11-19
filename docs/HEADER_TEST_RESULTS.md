# Cyberpunk header and local installation validation — 2026-11-19

Development version: 1.2.2. Arch Linux x86_64 / Hyprland Wayland.

## Change

The original 2077 wordmark occupied 86 vertical pixels from y=10, crossing the
toolbar at y=89. The wordmark, subtitle, barcode and animated glitch offsets now
scale together into a 72-pixel allocation. Font measurements include the optional
general font override. Available width after navigation determines whether the
decoration is shown; the optional Integrations tab participates in that layout.

## Checks

- Release build and all 20 CTest tests passed, including catalog, process
  lifecycle, preferences, localization, Caelestia and SDDM staging/rollback.
- Native lifecycle integration passed with 106 catalog entries, native scene and
  video preview startup, and preservation of an unrelated process on Stop.
- `tests/header_layout_integration.py` passed at 1000, 1264, 1400 and 1900 pixels
  wide, each with Integrations hidden and visible. The test checks that the
  decoration stays above the toolbar and clear of the settings, power and action
  buttons, samples multiple animation phases, and checks a general font override.
  Screenshots at half-screen width and at the 1000-pixel minimum were inspected.
  With the tested English labels, the decoration fits at 1000 pixels without
  Integrations and hides at that width when the extra tab is present.
- The real-pointer Integrations UI regression passed again: no integrations,
  Caelestia only, SDDM only, both enabled, saved choices, disabling the current
  panel, disabling the last integration, and re-enabling saved preferences.
  Only Caelestia dependency preparation is substituted in these isolated UI tests;
  SDDM activation uses the installed Qt6 greeter in test mode.
- Python syntax and `git diff --check` passed.

## Installed runtime

The user launcher was upgraded from 1.1.0 to a verified 1.2.2 local source build.
The runtime was staged in a new versioned directory, with configuration and the
old launcher backed up. Both the staged executable and its dynamic dependencies
were checked, and its catalog contained the same 106 entries. The replacement
renderer was verified alive on the same wallpaper before switching the launcher
and retiring the old process. PID, start time, executable and renderer arguments
were checked before signaling. The previous runtime remains available for rollback.

The installed GUI resolves to the new runtime and displays the Integrations page
using existing preferences. At a tiled 1263 × 1358 window size, real pointer input
opened Library, the integration switches in Settings, and the management page;
the header and activation controls were inspected. The saved settings were
unchanged. The applications
menu and existing autostart entry use the updated user launcher; neither autostart
nor shortcuts were enabled or edited. SDDM activation remains an explicit choice
in Settings, and system greeter configuration was not modified.

Logs, screenshots, fixtures and the machine-specific installation record remain
in ignored `local/` and XDG directories. This verification does not publish a new
binary release or certify all font/language combinations. The experimental SDDM
scope remains as described in the [SDDM report](SDDM_TEST_RESULTS.md).
