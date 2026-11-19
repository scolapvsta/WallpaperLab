# Native login presets and color controls — 2.4.0

Date: 2026-11-19. Platform: Arch Linux x86_64, Hyprland/Wayland, two outputs.
Evidence remains in ignored `local/` folders and isolated XDG directories.
No Workshop assets, screenshots, preset references or machine-specific state are
included in the repository.

## Automated checks

- Release build and **22/22 CTest passed**. The catalog and process/authentication
  suites remain part of this regression gate.
- Login configuration tests cover UI-only and complete preset round trips,
  invalid/missing wallpaper rejection, scope preservation, same-name updates,
  backup storage and safe filenames even when preset names contain slashes.
- Color suggestions are tested against dark, light, mixed and saturated frames.
  Restoring their color-only snapshot exactly reproduces the original document,
  proving that no non-color settings changed. A split light/dark frame verifies
  that each widget's own region controls text contrast. Button text meets 4.5:1
  against its nominal fill in these fixtures.
- Pixel comparisons verify that the Sign in button does not inherit the widget's
  3D effect, while enabling its own effect changes the rendered button.
- Migration removes retired palette/light switches and preserves saved styles.
  Localization coverage passes across 22 locales.

## Editor pointer and keyboard checks

The isolated local-image/GIF fixture suite passed at 1264×900, with final visual
checks at 1000×740:

- Search and shared type/source filters; independent login image adjustments.
- X/Y widget sliders reaching monitor bounds; editable labels and reset backups.
- Color-only suggestion and exact undo, retaining all opacity/layout/style values.
- Manual RGB entry, invalid channel rejection, screen eyedropper matching a
  captured desktop pixel, and Escape cancellation preserving the existing color.
- Editable Sign in label and independent None/3D effect selection.
- UI-only save/load preserving the wallpaper and image adjustments.
- Complete save/load restoring the wallpaper, its image edits and widgets.
- Persistence after app restart, and discovery/loading of an externally copied
  JSON preset when reopening the preset selector.

Reset actions sit below Grid/Snap. Appearance sections, RGB fields and preset
controls were inspected in screenshots. Opening library filters reduces the
monitor overview height so the filter panel has more room. Save/Update action
width follows the translated label rather than clipping it.

One earlier test run encountered a partial diagnostic JSON read while the UI was
rewriting its test-only geometry file. The harness now retries these reads; the
complete verification run passed. Hyprpicker's quiet mode suppresses its selected
color on this installation, so the adapter deliberately does not request it.

## Real catalog, native rendering and local installation

- The real installed catalog contained 106 projects: 93 supported, 12 Web
  exclusions and one scene with an explicit renderer limitation. Search/type
  filtering, the compact themed editor and the new color suggestion passed using
  installed scene artwork, without altering the user's saved draft.
- The focused shared-Library inspector check passed visual color persistence,
  live/persisted zoom and creator reset preserving position. This exercised the
  shared picker in a project that reloads after a property edit.
- Native GIF frames changed over time. Interactive preview accepted disposable
  text, animated password input and the session dropdown without authentication or
  persistence. The mock-greetd keyboard/IPC suite passed, including cursor editing;
  only the simulated identity was retained.
- Catalog/lifecycle checks passed native scene/video previews, foreign-process
  preservation, verified scene replacement and retaining the previous renderer
  when its replacement failed.
- The local launcher/runtime was upgraded to 2.4.0 with configuration backups and
  a verified renderer handoff preserving the current wallpaper. The root-owned
  next-boot bundle was republished with the current draft and retained the original
  rollback chain. All 3,080 bundle files were readable by the greeter account;
  version, ownership and the recorded greetd configuration hash were verified.
- The installed standalone native scene preview reached readiness in **0.802 s**
  using isolated XDG directories. The running greetd service and desktop session
  were not restarted. **No cold boot or real PAM login was performed for 2.4.0**;
  preview and mock IPC results do not establish those outcomes.

## Limits

A color suggestion samples one edited still frame (or the labeled thumbnail
fallback while capture is pending). Colors alone cannot guarantee contrast for
transparent widgets over every frame of moving imagery. Opacity is never raised
automatically to conceal that limitation. The eyedropper currently requires
Hyprpicker on Wayland; no X11/portal fallback was validated. Presets reference
installed assets and local output names; they do not redistribute wallpapers.
