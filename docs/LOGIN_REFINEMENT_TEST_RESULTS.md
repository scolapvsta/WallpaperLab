# Native login refinement validation — 2026-11-19

Scope: source/local runtime 2.1.0. This refines the native greetd login introduced
in 2.0.0; the public downloadable installer remains a separate release.

## Environment and baseline

Validation used Arch Linux x86_64, Hyprland 0.56.2, greetd 0.10.3, GNOME Keyring
51.1 and two physical Wayland outputs (1920×1080 and 2560×1440). The engine and
raylib pins remain unchanged; pre-existing dependency edits were preserved.

The user confirmed an actual boot/login with 2.0.0. The current boot journal also
records the greetd greeter and subsequent authenticated desktop sessions. That
establishes the previous implementation's real login path, while the reported
launcher warning and missing keyring unlock motivated this refinement.

## Automated and graphical results

- Release engine/application builds passed. **22/22 CTest checks passed**, including
  catalog, process lifecycle, source containment, translations, patch ordering,
  login configuration, fake greetd protocol and deployment/rollback tests.
- Schema migration removes the calendar and preset identifier while retaining
  other widget/layout/image preferences. Independent output projects and profiles
  are prepared without changing the desktop profiles.
- Deployment fixtures verify separate exported output projects and profile paths,
  optional keyring hooks, idempotent updates, original-backup retention, exact PAM
  restoration and refusal to overwrite a later external PAM edit.
- `tests/native_login_graphics.py`: actual GIF frames changed; preview rejected
  credentials and closed on Escape. The native greeter completed a fake greetd
  exchange through keyboard input. Home/End, arrows, insertion and Backspace
  produced the expected response. A captured password field shows dots, focused
  border and caret. Only username/session identity was persisted; the fixture
  password was absent from logs.
- `tests/login_outputs_graphics.py`: generated local fixtures rendered different
  colours on two outputs. Changing one output's brightness left the other intact.
  Mirror used the shared source, and a panoramic image crossed the physical
  output boundary once instead of being duplicated or stretched per monitor.
- `tests/integrations_ui_integration.py`: feature activation/routing, separate
  Widget/Appearance tabs, saved widget preferences and independent Caelestia
  toggles passed. Disabling/re-enabling retained the native editor choices.
- An isolated editor check with an installed Workshop scene verified the native
  still cache at the source aspect ratio, panoramic composition, explicit output
  tabs in Single mode and a dropdown for primary output selection. Captures were
  inspected at 1264×900 and 1000×740, including the fixed inspector tabs and
  compact Cyberpunk header.
- The installed runtime's live preview remained alive after its asynchronous
  launch worker completed, rendered on both physical outputs and closed on Escape.
  This directly exercises the thread-lifetime regression behind the failed preview.
- Catalog/native lifecycle checks passed over 106 installed entries: scene/video
  previews, scene replacement, failed-scene preservation and foreign-PID protection.
  Test renderers used isolated state and were stopped afterwards.

## Local installation and system login

The installed desktop application reports 2.1.0. Its launcher and menu point to
an immutable runtime. A verified replacement renderer retained the current desktop
wallpaper before the old process was retired. Configuration and the previous
runtime were backed up. The saved login draft migrated connector labels to stable
IDs and retained the user's current mode, UI placement, widgets and image edits.

System publication completed through the installed helper. The root-owned 2.1.0
bundle contains both independently prepared output projects. The greeter UID
could read all 3,087 bundle files and execute the bundled app. The bundle's
runtime/project/profile/font paths are independent of the desktop user's home.
A native scene window with an empty HOME reached its ready frame in 0.802 seconds
on this workstation. The published single-output-assignment configuration also
rendered its scene and UI on both physical outputs with an empty HOME, reached
readiness in 0.752 seconds and closed on Escape. These are single local
measurements, not performance guarantees.

The greetd configuration hash and enabled display-manager alias matched the
installation record. The optional GNOME Keyring auth/session hooks are present;
the saved original PAM file and applied-file hash were verified. The original
rollback chain was retained. Publication used next-boot activation and did not
restart greetd or terminate the active desktop.

## Remaining verification boundaries

- The new `start-hyprland` compositor launch and PAM keyring changes require a new
  real login to verify their complete boot/session behaviour. The current desktop
  was not logged out or rebooted. Fake greetd tests do not prove PAM unlock.
- Automatic keyring unlock still depends on the login keyring's password matching
  the account password and the distribution's PAM stack. No keyring was reset and
  no stored secrets were read.
- Still captures represent one moment of a scene; animation and native project
  compatibility must be assessed with the explicit live preview. Initial capture
  is bounded/asynchronous, not guaranteed instantaneous for every Workshop scene.
- Output tests used the connected hardware. Other GPU drivers, rotated/mixed-scale
  layouts and physical hotplug during authentication were not certified here.

Logs, screenshots, generated images, Workshop data and machine-specific backups
remain under ignored `local/` or XDG/system state directories and are not committed.
