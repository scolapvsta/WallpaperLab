# Native login library and editor polish — 2026-11-19

Source version: 2.3.0. Tests ran on Arch Linux / Hyprland, with two differently
sized monitors. Generated fixtures, screenshots, logs and deployment records stay
under ignored `local/login-polish/` or XDG directories.

## Automated and graphical checks

- CTest: 22/22 passed, including catalog, project properties, localization,
  lifecycle fixtures, login authentication/configuration and deployment boundaries.
- Login configuration checks cover retired clock migration, UI-only/widget reset,
  image/display preservation, independent resolution export, renderer arguments,
  palette button contrast in both light and dark modes, and native monitor profiles.
- `tests/login_editor_integration.py`: passed with isolated local image/GIF
  fixtures. Pointer and keyboard checks cover search, the same Library type/source
  filters, image-profile isolation, Y reaching the monitor edge, visible text
  editing, both palette modes, visual colour selection, manual override and reset
  backups. Layouts were inspected at 1264×900 and 1000×740.
- `tests/native_login_graphics.py`: passed. Animated GIF frames change, preview
  accepts disposable masked input, session dropdown keyboard selection works,
  the mock greetd flow succeeds, and only username/session metadata is persisted.
- `tests/integration.py --desktop`: passed in isolated state with the real
  106-project catalog. Native scene/video previews work, scene replacement retires
  the verified previous renderer, a failed replacement preserves it, and a foreign
  process is never signaled.
- Real catalog UI probe: 106 projects, with 93 available and 13 unsupported
  (12 Web, one perspective 3D scene). Search finds the two visualizers; the Scene
  filter returns 74 supported entries plus the unsupported 3D scene. Compact
  Cyberpunk layouts and the digital clock were visually inspected.
- The normal Library inspector's shared colour picker, live/persistent zoom,
  creator reset preserving placement, and display-popup checks passed. The legacy
  full script subsequently timed out in its separate general-font-menu step;
  that menu is unchanged and is outside this change's verification scope.
- Localization source audit: 549 catalog entries across 22 locales. Image/crop
  controls and HSV/RGB picking share implementation with the normal library.

## Verification scope

The editor palette samples a cached native still (or the labelled thumbnail while
that frame loads), applies image adjustments, and derives contrasting saved tones.
It does not continuously resample the animated wallpaper at the login screen.
Manual colour edits disable automatic palette updates. Sampling is limited to
five times per second during continuous image adjustments.

Graphical checks use isolated settings and synthetic credentials. They do not
perform a real system login or validate every Workshop project. A new cold boot
is not part of this test run. The prior working greetd/PAM/session configuration
is retained during runtime publication.

## Local publication

The installed launcher and boot bundle report version 2.3.0. The desktop renderer
was handed off only after the replacement was verified alive on the same selected
wallpaper. Configuration/launcher backups were created before activation. The
boot bundle is root-owned, not group/world writable, and all 3,080 files are
readable by the greeter account. The greetd configuration hash matches its
installation record and the original rollback backup chain is retained.

A standalone native scene preview from the system bundle, with isolated XDG
directories, became ready in 0.702 seconds. The current desktop session was
not restarted; a real cold boot remains untested for this version.
