# Native login screen (experimental)

Source version 2.3 extends the native editor introduced in 2.0, which replaced the SDDM preview experiment. Wallpaper Lab supplies
its own editor, native wallpaper rendering, widgets and login controls. greetd
provides PAM authentication and session startup; Hyprland provides the dedicated
pre-login Wayland compositor. The normal desktop launcher is not required at boot.

## Editor and preview

1. Enable **Settings → Integrations → Login screen**. New installations leave this
   optional feature disabled. Caelestia remains independent.
2. Open the **Login screen** tab and select a compatible local or Steam wallpaper.
   Import images/GIFs/videos with the library import button. Steam projects must
   already be downloaded; compatible Workshop scenes use the native engine.
3. The **Wallpapers** tab combines the library, monitor preview and shared library image inspector.
   Search supports Ctrl+L. The filter panel uses the same source, type, audio,
   resolution groups and AND/OR rules as Library. Display controls include single/mirror/panorama, primary
   output, login UI placement and FPS. Single mode exposes output tabs; each
   output retains its own wallpaper and image edits. Mirror/panorama use the
   shared source. Image controls reuse colour, crop/fit, position, zoom, rotation
   and mirroring without modifying desktop profiles.
4. Available wallpapers appear first. **Unsupported wallpapers** follow at the
   end of the filtered/paginated library, with each project's actual reason.
   Web projects require HTML/JavaScript, which the native greeter does not run.
   An audio-visualizer title or capability alone does not reject a scene. Audio
   processing is disabled at login, so reactive elements do not use desktop audio.
5. Open **Login UI editor**. Only **Clock, Date and Login UI** remain. Select a
   widget using the persistent selector above its **Widget / Appearance** tabs.
   Drag widgets with the optional grid and magnetic snapping; configure the grid
   spacing or use left/center/right placement buttons. X/Y sliders show pixels on
   the reference monitor and cover the full legal widget position range. Grid/snap are editor aids
   and do not appear in the real login screen.
6. Each widget has its own font (Automatic inherits the base font), text colour,
   bold/italic/underline, alignment, letter spacing, shadow/3D relief, opacity,
   background, rounded corners and border. The digital clock supports 12/24-hour
   format and optional seconds; retired analog styles migrate automatically.
   Dates support long, short, ISO and weekday formats plus uppercase.
7. Login UI's Widget controls contain the welcome toggle and optional time-based
   greeting with the current username. Custom labels cover welcome, username,
   password, session and sign-in. Empty labels use localized defaults. Appearance
   adds button/text/field colours, focus border, field opacity/radius/height,
   internal padding, label/button size and password-dot size/animation duration.
   Real PAM challenge text remains authoritative for additional authentication
   prompts; custom password labels only replace the normal password placeholder.
8. Run **Live preview · 60 seconds** to inspect real native rendering and type
   disposable test text into the fields. Password dots/caret animate normally;
   Sign in only simulates the preview interaction. Preview never connects to
   greetd, authenticates, launches a session or saves the entered text. Esc closes
   it (first closing an open session menu). The desktop wallpaper keeps running.

**Wallpaper palette** extracts a dominant colour from the edited wallpaper frame,
with separate light/dark tones. The colour sample is cached until the wallpaper,
image edits or mode change. Text and panel tones are paired for readability; button
text has at least 4.5:1 contrast against its fill. The palette is saved in the draft
and exported with the login bundle. A manual colour edit disables automatic colours.
Visual HSV and RGB pickers are shared with the library's project colour controls.

**Reset widget** restores the selected widget; **Reset login UI** restores all
widgets, base styling and grid settings. Both save a draft backup under the state
backups directory and preserve wallpapers, image profiles, monitors and FPS.
Text labels use bordered input fields with a caret, selection and clipboard support.

The native session selector opens an actual dropdown. Click an entry or use
Up/Down, Home/End and Enter; Escape dismisses the menu without changing the choice.
The real greeter starts only installed, trusted Wayland session entries after
successful authentication.

The editor renders and caches a neutral still frame of the selected project in
its original aspect ratio. Image controls are then applied by the editor shader,
so adjusting a slider does not reload the scene. Cached stills load immediately;
first-time capture runs asynchronously in a bounded hidden renderer. While it
loads, the editor labels the fallback as a Workshop thumbnail. Such thumbnails
may differ from the full project. The live preview shows actual animation and
output geometry; it uses a process identity guard that survives completion of the
background launch worker.

The monitor map uses the saved desktop panorama calibration, independently of any
unsaved desktop-layout draft. UI widget geometry is local to each output. Password
fields draw animated dots, show a blinking caret and support UTF-8 cursor movement,
Home/End and deletion; the interactive preview keeps its test input only in memory.

Draft preferences are stored in `$XDG_CONFIG_HOME/wallpaper-lab/login-screen.json`.
Saving a draft does not change an already published system login. Migration
removes the retired calendar/media/message widgets and preserves the remaining
layout, translating the old global form styling to Login UI settings. The login
session starts muted by default.

## System prerequisites

The current compositor adapter targets Arch Linux x86_64 and Hyprland with the Lua
configuration API (validated with 0.56.2). Older Hyprland releases and other login
compositors are not supported by this adapter. The editor remains usable separately.

Install a complete Wallpaper Lab runtime, keep the system packages up to date,
and install optional prerequisites through the distribution package manager:

```sh
sudo pacman -S --needed greetd greetd-agreety hyprland polkit dbus
```

The greeter user, PAM service and greetd systemd unit are supplied by the packages.
A working graphical polkit authentication agent is needed for the app's Apply
button. Do not run the desktop application as root. If GNOME Keyring's PAM module is installed, publication adds missing optional
keyring authentication/session hooks to the distribution's greetd PAM file, after
backing it up. Passwords remain inside the normal PAM exchange; Wallpaper Lab does
not read keyring contents or change keyring passwords. Rollback restores the
original PAM file and refuses to overwrite later external changes. The installed Wayland sessions under `/usr/share/wayland-sessions`
provide the desktop choices; this version does not launch X11 sessions.

## Publishing for the next boot

**Apply at next boot** first prepares a private staging bundle and then invokes
administrator authentication. It copies:

- the allowlisted app, engine/runtime dependencies, fonts and licenses;
- the materialized selected project and installed Workshop dependencies;
- the native engine asset tree, chosen widget fonts and validated rendering/UI profiles;
- a static native fallback background and the dedicated compositor supervisor.

Projects containing symlinks or special files are rejected. External scene file
properties must be imported into the selected project before publication. The
bundle must be readable without the desktop user's home, Steam or mounted library.
No Steam account data, credentials or unrelated library content is copied.

The immutable release goes under `/usr/local/lib/wallpaper-lab-login/releases`.
The helper snapshots `/etc/greetd/config.toml`, the system display-manager alias
and its own previous metadata, then enables greetd **for the next boot**. It does
not restart SDDM/greetd or terminate the current desktop. Subsequent publications
retain the original pre-Wallpaper-Lab backup for feature deactivation. A failed
publication preserves the last successfully installed version.

At boot greetd starts Wallpaper Lab's dedicated Hyprland session through
`start-hyprland`, the compositor's watchdog launcher, as the package's
unprivileged greeter user. Widgets and wallpapers share the native renderer's
OpenGL context. Authentication follows greetd's secret/visible/informational PAM
messages, including multiple prompts. The selected session starts only after
successful authentication and after the greeter compositor exits, as specified by
[the greetd protocol](https://man.archlinux.org/man/greetd-ipc.7.en).

The first login is seeded with the publishing desktop’s username and session
choice (including UWSM when in use). Session hints only select an installed trusted
desktop entry. Successful logins take precedence over these initial hints.
Only the last username and desktop session ID are remembered under
`/var/lib/wallpaper-lab-login`; passwords are neither persisted nor logged. The
last published UI/wallpaper is retained across boots. Changing the desktop
wallpaper does not automatically change the login wallpaper.

## Recovery and removal

If the selected scene fails to initialize, the supervisor tries the static native
background with the same login UI. If the private compositor or native greeter
cannot start, it falls back to the package's terminal agreety on the allocated VT.
The compositor startup deadline is 75 seconds, including a possible 30-second
renderer attempt and native-background fallback. A graphics driver/VT/PAM failure
can still prevent these fallbacks; they are not a replacement for a recovery TTY.

Disabling **Login screen** after publication authenticates the restoration of the
original display-manager configuration before hiding the editor. If unavailable,
use a TTY and the backup path recorded in
`/usr/local/lib/wallpaper-lab-login/installed.json`:

```sh
sudo python3 /usr/local/lib/wallpaper-lab-login/current/runtime/login_deploy.py rollback /usr/local/lib/wallpaper-lab-login/backups/RELEASE_ID
```

Rollback changes the next-boot configuration; it does not terminate a running
session. Existing system SDDM packages and historical application settings are
not removed. Immutable releases/backups remain available for recovery and may be
removed manually once no longer in use.

Greeter diagnostics are in `$XDG_RUNTIME_DIR/wallpaper-lab-login` for the greeter
user; service diagnostics are available with `journalctl -u greetd -b`. Draft
preview logs and publication staging remain under the desktop user's
`$XDG_STATE_HOME/wallpaper-lab/login-screen`. Review logs before sharing them.

## Current limitations and verification

The [customization test report](LOGIN_CUSTOMIZATION_TEST_RESULTS.md),
[refinement test report](LOGIN_REFINEMENT_TEST_RESULTS.md) and
[initial report](NATIVE_LOGIN_TEST_RESULTS.md) distinguish real desktop output
checks, fake greetd IPC tests and actual system-boot validation. A fast, successful
preview alone does not validate PAM login, VT handoff or a cold boot.

- Workshop compatibility is that of the existing native engine. Web wallpapers
  are excluded from the login catalog; local image/GIF wrappers use native mpv.
- Still frames represent one point in the scene's animation, not an interactive
  scene preview. The explicit live preview remains the animation check.
- Independent outputs share the same native engine process and authentication UI;
  their creator properties, placement and image adjustments are selected per output.
- No desktop autostart entry is needed for the login screen. Installing a bundle
  does not enable Wallpaper Lab's separate desktop autostart or shortcuts.
- This is a login greeter, not a secure desktop lock screen. Do not use preview as
  a lock screen. On-screen keyboards, accessibility integration and X11 sessions
  are not implemented.
