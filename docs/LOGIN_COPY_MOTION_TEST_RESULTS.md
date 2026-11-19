# Login Library import and motion validation — 2.5.1

Date: 2026-11-19. Platform: Arch Linux x86_64, Hyprland/Wayland, two outputs.
Artifacts, screenshots, logs and machine-specific configurations are retained in
ignored `local/` or XDG directories, not in the repository.

## Configuration and rendering

- **23/23 CTest checks passed**, including catalog, configuration, authentication,
  deployment, upstream patch preservation and localization coverage.
- Library import copies image adjustments, normalized position/zoom, a matching
  resolution variant and valid creator options. Tests verify no desktop/UI/display
  changes, selected-output image scope, default-profile replacement and prepared
  native renderer data.
- Imported creator options remain a snapshot after the desktop profile changes.
  Complete presets retain them; legacy complete presets load without inheriting an
  unrelated snapshot. UI-only presets preserve wallpaper settings.
- A private copy of the current Workshop scene's real configuration imported its
  non-default position and zoom and prepared successfully. Original user files
  were not modified by this probe.
- All ten Astronaut presets render at 640×360, 1920×1080, 2560×1440 and 1280×1024.
  Newly enabled Welcome text renders with short, multilingual and long labels;
  the clock region stays unchanged. Dynamic greetings render as well. Captures
  of every preset were inspected.
- Animation redraw regions were compared with full-frame rendering for all ten
  presets at 640×360 and 1920×1080, including open session menus and virtual
  keyboards. Maximum per-channel difference was 2/255 from Cairo clipped-group
  and scaled-mask rounding. No larger pixel differences were accepted.
- Timing tests cover concurrent new dots during rapid typing, deletion indices,
  timing cleanup and pulse endpoints. The timing tracker retains no password copy.

## Interactive checks

The isolated graphical editor suite passed **18 checks**, including:

- Copy settings from Library through the actual inspector button, visible image
  changes, backup creation and unchanged Library/UI/display data.
- Search/shared filters, image edits, position bounds, reset, text editing,
  RGB fields, invalid values, eyedropper selection and cancellation.
- Independent Sign in label/effect, color-only suggestion/undo, UI/complete preset
  save/load, restart persistence and external preset discovery.
- All ten bundled presets loaded through the selector, then Welcome enabled and
  its text edited through the actual field at a compact 1000×740 window size.

Native base and Astronaut suites passed on the final installed candidate:

- Animated GIF playback, disposable masked preview input, session dropdown
  selection and Escape behavior.
- Astronaut virtual keyboard open/type/close and system confirmation/cancel.
- Mock greetd create/respond/start-session using physical keyboard input,
  Home/End, insertion and backspace. Only simulated username/session identities
  were persisted; test passwords were absent from logs.

GPU blur checks passed: high-frequency detail decreased inside the form, colors
and orientation were retained, and pixels outside the form were unchanged.
These checks are functional verification, not a frame-rate benchmark.

## Lifecycle and installation

The real catalog contained 106 projects. Native scene/video previews, foreign
process preservation, scene replacement and retaining the previous renderer on
replacement failure all passed in isolated state.

The local app was upgraded to 2.5.1 with configuration/launcher backups and a
verified renderer handoff retaining the selected desktop wallpaper. The current
login draft and personal presets were preserved. System publication remains the
normal **Apply at next boot** action.

No cold boot, real PAM authentication or real power operation was performed for
this change. The graphical preview and mock greetd checks do not establish those
outcomes. Existing authentication/session behavior and next-boot publication
configuration were preserved.
