# Astronaut UI presets

Wallpaper Lab 2.5 includes ten editable native UI presets based on
[Keyitdev's sddm-astronaut-theme](https://github.com/Keyitdev/sddm-astronaut-theme),
pinned at `abb3163c724935af888ba5ea9ac0c4f22afd8048`:

- Astronaut
- Black Hole
- Cyberpunk
- Hyprland Kath
- Jake the Dog
- Japanese Aesthetic
- Pixel Sakura
- Pixel Sakura Static
- Post-apocalyptic Hacker
- Purple Leaves

Open **Login screen → Login UI editor → UI preset**, select an **Astronaut · …**
entry and press **Load**. The wallpaper, image adjustments, monitor arrangement,
FPS and activation state remain unchanged. Save the edited UI under a new name
for a personal preset, or save a complete preset in the **Wallpapers** tab to
include its wallpaper and display configuration. Saving over a bundled name
creates a personal override; application files remain untouched.

**Live preview** tests the native rendering and input. **Apply at next boot**
publishes the selected design and required fonts to the independent greetd bundle.
Loading a preset alone does not replace the boot configuration.

## Design and editing

These are native adaptations of the source layouts, not a runtime integration
with SDDM. The full-height form strips, 40% monitor width, centered fields,
clock/date typography, colors, transparency, source icons and lower session
selector follow the source design. Pixel Sakura's two entries intentionally
share the same UI: their original difference is the wallpaper.

The login widget's **Layout → Astronaut** controls include field width and
spacing, field/button/session/system-action positions, button visibility,
virtual keyboard, system buttons and background blur. Its other text, color,
background and button controls remain editable. Clock and date remain independent
widgets and can be moved in front of the form strip. **Reset UI** returns to
Wallpaper Lab's base layout.

The native date formats reproduce the source long date, weekday/day, and Cyberpunk's
fixed `2077` year. The current locale still determines translated date names.
Decorative fonts use their actual face weight: unlike Pango, the Qt reference did
not synthesize a bold face for these single-face fonts. The editable bold toggle
can still apply synthetic weight if desired.

Open Sans, Electroharmonix and Fragile Bombers Attack are bundled with their
licenses. ESPACION, KogniGear, Thunderman, Pixelon and ArcadeClassic are used if
installed; their original files are not redistributed. A missing-font notice
appears when loading a preset that requires substitution. The private deployment
bundle includes the selected installed fonts so the greeter can read them before
login. See [attribution and font licenses](../assets/login/astronaut/NOTICE.md).

## Native interaction and limits

Authentication remains greetd/PAM. The session selector is a dropdown of installed
Wayland sessions. The eye icon toggles password visibility. The optional native
on-screen keyboard has a compact ASCII layout, Shift, space, backspace and Enter;
it is not Qt's predictive/multilingual keyboard. Existing physical keyboard and
compose handling continue to work.

System buttons open confirmation before requesting a fixed action from [logind](https://www.freedesktop.org/software/systemd/man/latest/org.freedesktop.login1.html).
The system's permissions determine whether an action is allowed. Preview mode
never executes a system action or authenticates. Native account entry remains a
text field; SDDM's account chooser and its user database are not imported.

The original QML and a native reference render were compared at 1920×1080. Font
rasterization, hover behavior, native menus and the GPU blur kernel can differ
from Qt. These presets preserve the designs, but are not pixel-identical QML
screens. At other sizes the native layout scales to the output; it also retains
Wallpaper Lab's panoramic/independent/mirrored wallpaper and UI-output behavior.
Original theme backgrounds are not included or selected by a UI preset.

## Maintenance

The source-only reference submodule is `vendor/astronaut-design`. It is not linked
or executed by Wallpaper Lab. `scripts/import-astronaut-presets.py` translates
reviewed theme values and rasterizes the GPL icons with `rsvg-convert`. Its pin
check requires an explicit review before regeneration from newer upstream code.
The source packaging allowlist excludes upstream Fonts, Backgrounds and Previews;
only redistributable application fonts are shipped separately with their notices.

See [test results](ASTRONAUT_PRESETS_TEST_RESULTS.md) for actual validation.
