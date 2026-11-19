# Native login screen (experimental)

Source version 2.0 replaces the SDDM preview experiment. Wallpaper Lab supplies
its own editor, native wallpaper rendering, widgets and login controls. greetd
provides PAM authentication and session startup; Hyprland provides the dedicated
pre-login Wayland compositor. The normal desktop launcher is not required at boot.

## Editor and preview

1. Enable **Settings → Integrations → Login screen**. New installations leave this
   optional feature disabled. Caelestia remains independent.
2. Open the **Login screen** tab and select a compatible local or Steam wallpaper.
   Import images/GIFs/videos with the library import button. Steam projects must
   already be downloaded; compatible Workshop scenes use the native engine.
3. Choose **Orbit, Horizon, Neon, Paper or Canvas** under **Templates and widgets**.
   Drag enabled widgets inside a monitor preview or adjust normalized coordinates,
   dimensions and font size. Appearance includes font family, light/dark styling,
   accent, panel opacity and corner radius. The login form remains enabled and
   has a usable minimum size. Calendar, wallpaper playback, date, clock and
   message widgets can be toggled independently.
4. Choose single, mirrored or panoramic wallpaper output under **Monitors and
   startup**, independently of the primary/secondary/all login UI choice. When starting with only one
   connected output, a secondary-only UI falls back to that output.
5. Adjust wallpaper color, crop/fit, position, zoom, rotation and mirroring in the
   wallpaper inspector. These profiles do not alter the desktop library profiles.
6. Run **Live preview · 60 seconds** to inspect actual native rendering. Esc closes
   it early. The existing desktop wallpaper remains running. Preview refuses
   credential input and never opens a greetd connection.

Editor compositions use the cached catalog poster: a Workshop thumbnail is not a
rendered scene, and may show a different crop or art from the full project. The
live preview shows actual project compatibility, animation and output geometry.
Only an explicit live-preview action starts a renderer; changing cards/templates
updates the native editor without rebuilding an external theme. UI textures are
cached and recomputed when their content changes, rather than streamed through
frame files. Scene loading time still depends on project size and engine support.

Draft preferences are stored in `$XDG_CONFIG_HOME/wallpaper-lab/login-screen.json`.
Saving a draft does not change an already published system login. The media widget
controls the login wallpaper's playback; it does not expose a logged-in user's
music session before authentication. The login session starts muted by default.

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
button. Do not run the desktop application as root. No PAM files are generated
by Wallpaper Lab. The installed Wayland sessions under `/usr/share/wayland-sessions`
provide the desktop choices; this version does not launch X11 sessions.

## Publishing for the next boot

**Apply at next boot** first prepares a private staging bundle and then invokes
administrator authentication. It copies:

- the allowlisted app, engine/runtime dependencies, fonts and licenses;
- the materialized selected project and installed Workshop dependencies;
- the native engine asset tree, chosen font and validated rendering/UI profiles;
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

At boot greetd starts Wallpaper Lab's dedicated Hyprland session as the package's
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

The [test report](NATIVE_LOGIN_TEST_RESULTS.md) distinguishes real desktop output
checks, fake greetd IPC tests and actual system-boot validation. A fast, successful
preview alone does not validate PAM login, VT handoff or a cold boot.

- Workshop compatibility is that of the existing native engine. Web wallpapers
  are excluded from the login catalog; local image/GIF wrappers use native mpv.
- The editor uses posters, not continuously rendered scene thumbnails. Selecting
  another template resets widget layout/font/appearance to that preset.
- Single-output wallpaper mode currently initializes rendering on the connected
  outputs and clears inactive backgrounds. It preserves output geometry but is
  not yet optimized to avoid all work on the inactive output.
- No desktop autostart entry is needed for the login screen. Installing a bundle
  does not enable Wallpaper Lab's separate desktop autostart or shortcuts.
- This is a login greeter, not a secure desktop lock screen. Do not use preview as
  a lock screen. On-screen keyboards, accessibility integration and X11 sessions
  are not implemented.
