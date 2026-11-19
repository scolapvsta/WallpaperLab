# Native login customization validation — 2026-11-19

Scope: source/local runtime 2.2.0 on Arch Linux x86_64, Hyprland 0.56.2, with
1920×1080 and 2560×1440 physical outputs. Existing dependency edits were preserved;
the new session-key mapping is recorded as an ordered engine patch. Public binary
installer releases remain separate from this local/source update.

## Implementation checks

- Application and engine-library Release targets built successfully. The unrelated
  upstream all-target build hits an existing unquoted glslang generator path when
  the source directory contains parentheses; the shipped engine-library target
  does not require that standalone generator target and built successfully.
- **22/22 CTest checks passed**, covering catalog/lifecycle, translations, patch
  ordering, login configuration, fake greetd protocol and export/rollback.
- Version 3 migration removes Calendar, Media and Message, keeps Clock/Date/Login,
  and migrates global form colours/appearance/welcome into the Login UI widget.
  Snapping and bounds are tested independently of the UI. Dynamic greetings were
  checked at morning and evening times with a username.
- Digital/minimal/classic/ring clocks rasterize through the native canvas. Session
  dropdown drawing and hit-testing share normalized geometry; a second-row hit
  selects the second session. Unsupported Web classification uses project type,
  not an audio-visualizer title. Native scenes with such a title remain eligible.
- Deployment fixtures check export of two independent widget fonts alongside the
  existing profile/project isolation, manifest containment, PAM backup and rollback.
- Translation source audit passed for **543 messages across 22 locales**. These
  short UI labels were included in the application build; not every locale was
  visually reviewed on every font.

## Graphical and process checks

- `tests/native_login_graphics.py`: native GIF frames changed; interactive preview
  accepted disposable password text, showed the session dropdown and supported
  keyboard choice/closure. Preview input was absent from logs and no remembered
  login identity was written. The preview branch does not construct a greetd
  connection. The actual greeter entry completed its separate fake greetd exchange
  and retained the password cursor/editing checks.
- An isolated Workshop-scene editor check inspected the unified wallpaper/display
  tab, per-widget inspector, analog clock, grid and dynamic greeting, including
  1264×900 and 1000×740 captures and real two-output live preview. Workshop posters
  remain distinct from the cached native scene still.
- Catalog/process lifecycle regression passed over 106 installed entries: native
  scene/video previews, replacement, failed-scene preservation and unrelated PID
  protection. Disposable test renderers used isolated state and were stopped.

- `tests/integrations_ui_integration.py` passed feature routing, exactly two tabs,
  independent widget font/italic/underline choices, magnetic placement after
  relative mouse motion, and preference retention through feature toggles.
- The two installed Audio Visualizer projects were confirmed as Web projects.
  Filtering them in the final installed-runtime UI showed a separate unsupported
  section with compact rows and readable HTML/JavaScript explanations at 1000×740.

## Local installation

The desktop launcher reports 2.2.0 and targets an immutable installed runtime.
The same desktop wallpaper was handed to a verified replacement process before
retiring the previous renderer. Configuration backups were retained. The current
login draft migrated to schema 3 with only Clock, Date and Login UI; its wallpaper,
monitor choices and remaining widget geometry were preserved.

System publication completed through the installed helper. The installed root-owned
bundle validates as schema 3 with exactly the three retained widgets. The greeter
UID could read all 3,079 bundle files and execute the 2.2.0 application. The greetd
configuration hash matched the publication record; the original rollback chain
was retained. The published native scene also rendered successfully with an empty
HOME in a standalone preview. No service restart or desktop logout was performed.

## Limits

The running desktop was not logged out or rebooted. A desktop preview and fake
IPC exchange do not prove a complete new boot/PAM/session handoff. This release
keeps the previously installed greetd and keyring integration unchanged.
Analog seconds update once per second; this is not a continuously swept second
hand. Text relief is a native layered text effect, not 3D geometry. Real additional
PAM prompts retain their own text. Initial native still capture remains asynchronous
and bounded. Workshop engine compatibility, Web exclusion and disabled pre-login
audio processing remain as documented in [Login screen](LOGIN_SCREEN.md).

All captures, private configuration, generated fixtures and Workshop assets remain
under ignored `local/` or XDG/system data directories.
