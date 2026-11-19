# Astronaut native preset validation — 2.5.0

Date: 2026-11-19. Platform: Arch Linux x86_64, Hyprland/Wayland, two outputs.
Reference: Keyitdev/sddm-astronaut-theme at
`abb3163c724935af888ba5ea9ac0c4f22afd8048`.
Evidence, reference captures and user-specific data remain in ignored `local/`
or XDG directories. The repository contains no Workshop assets or original theme
wallpapers.

## Reference and configuration

- Captured all ten original QML designs using an isolated offscreen Qt 6 probe,
  a generated checkerboard background, fixed time and fake user/session models.
  No SDDM service was started and no authentication was attempted.
- Compared the native clock/date/form positions and icon geometry at 1920×1080.
  This found synthetic bold differences in decorative single-face fonts; preset
  weights were corrected to match the source rendering. Open Sans retains its
  actual bold face. Captures were inspected for every distinct theme.
- All requested font families resolve on the tested desktop. Open Sans,
  Electroharmonix and Fragile Bombers Attack are the only added bundled families;
  their embedded license notices and source hashes are retained. Optional-font
  fallback on a machine lacking the other families was not visually evaluated.
- **23/23 CTest passed**, including catalog, configuration, authentication,
  deployment, patch preservation and localization coverage across 22 locales.
- The new preset test loads all ten entries, verifies UI-only scope, saves and
  reloads their editable data, tests personal overrides without modifying bundled
  JSON, and rejects an out-of-range blur value.
- Native rendering passes at 640×360, 1920×1080, 2560×1440 and 1280×1024. Pixel
  buffers remain premultiplied. Field, session, system-confirmation and virtual
  keyboard hit regions agree with their shared drawing geometry.
- Color suggestion/undo restores the complete document, including all extra
  Astronaut colors, and preserves every non-color value.
- Source reference archive allowlist passed: 31 entries, with Themes, Components,
  Assets and license; upstream Fonts, Backgrounds and Previews are excluded.

## Editor and interactive preview

The isolated editor suite passed at 1264×900 and 1000×740:

- All ten shipped presets loaded through the real dropdown without changing the
  current wallpaper, image edits, output mode or FPS.
- Existing search/filter, image adjustment, X/Y bounds, text editing, reset,
  RGB input, invalid RGB rejection, eyedropper and cancellation checks passed.
- UI-only and complete preset persistence, restart, external JSON discovery,
  independent button text/effect, and color-only suggestion/undo passed.

Native base and Astronaut interactive suites passed:

- GIF frames changed, and disposable preview input was not persisted or logged.
- Session dropdown keyboard selection and Escape behavior passed.
- Astronaut system confirmation opened and canceled in preview. The virtual
  keyboard opened, accepted a character and closed with Escape.
- A mock greetd server completed create/respond/start-session with physical
  keyboard input, masked passwords, Home/End, insertion and backspace.
  Only the simulated username/session identity was retained.
- After the final runtime adjustment, the installed candidate repeated the
  Astronaut preview and mock-greetd suite successfully.

The GPU backdrop test compared the same static asymmetric color/checkerboard
fixture with blur disabled and enabled. Detail inside the form decreased
(red-channel standard deviation 70.27 → 14.65), average colors were preserved,
and the image outside the form was unchanged. Windowed preview readiness was
0.905 s without blur and 0.705 s with blur in this run; these are startup samples,
not a throughput benchmark or a claim that blur improves startup speed.

## Desktop lifecycle and local upgrade

The real catalog contained 106 projects. Lifecycle checks passed native scene
and video previews, foreign-process preservation, scene replacement and retaining
the previous renderer when a replacement failed.

The local launcher/runtime was upgraded to 2.5.0 after configuration backups and
a verified renderer handoff retaining the current wallpaper. The shipped presets
are available in the existing UI selector. Loading a preset still requires the
user's normal **Apply at next boot** action to publish that chosen design.

## Limits

No cold boot, real PAM authentication or real power/suspend/hibernate action was
performed for this change. Power buttons use logind policy without bypassing
inhibitors; system support and permission failures remain possible. Existing
boot configuration was preserved. Desktop tests and mock greetd are not proof
of a cold-boot result.

The original offscreen reference used Qt's software backend, which does not
establish equivalence of its MultiEffect blur. Native blur was validated
separately on the GPU. Cairo/Pango rasterization, native menu/hover behavior,
account text entry and the compact ASCII on-screen keyboard differ from QML/SDDM.
Pixel Sakura and Pixel Sakura Static deliberately share one UI design. Original
backgrounds are not part of these UI-only presets.
