# Native login validation — 2026-11-19

Scope: source 2.0.0, Arch Linux x86_64, Hyprland 0.56.2, one 2560×1440 and one
1920×1080 output. The SDDM preview implementation has been removed. Logs,
wallpapers, generated fixtures and captures remain in ignored local/XDG storage.

## Automated checks

- Engine shared-library target and application build: passed. The separate
  upstream glslang standalone default target has a pre-existing script-path issue
  in a workspace with parentheses; the application build does not require it.
- CTest: 22/22 passed, including catalog, preferences, process lifecycle,
  localization, Caelestia, display layout and patch-series checks.
- Native login configuration: opt-in default, persistence, five template canvases,
  premultiplied alpha, widget containment, required form minimum size, image-profile
  shape, independent placement after the renderer polls an existing desktop
  profile, preserved scene creator properties, initial username/session hints,
  local-media conversion without changing the desktop wrapper, rejected
  escaping media paths and panoramic/mirror renderer arguments passed.
- Fake greetd IPC: partial headers/bodies, multiple secret/visible/info prompts,
  authentication and session-start errors, cancellation before retry, trusted desktop
  entries and preservation of quoted command arguments passed. No system account
  is authenticated by these tests.
- Publication helper: no-follow copy boundaries, changed payload hashes, parent/file
  symlinks, special files, self-contained path rewriting, bundled font, config and
  service restoration passed under an isolated fake system root. Host services
  are not modified by CTest.
- Native renderer PID/start-time/executable guard: a mismatched identity was
  rejected without killing the child; the matching identity was retired.
- Localization audit covers new editor/canvas labels and diagnostic routes across
  22 languages. This checks catalog coverage, not linguistic review of every label.

## Graphical checks

- Native scene preview: the same engine renders the scene and the app-owned widget
  texture in one graphics context. No Qt greeter or continuous PPM bridge is used.
- Real two-output checks: panoramic wallpaper with UI on both outputs, mirrored
  wallpaper with UI on the primary, and single-output wallpaper with UI only on
  the secondary passed. Native readiness was 0.70–0.75 seconds in these runs.
  Esc closed each preview cleanly. The personal desktop renderer state was unchanged.
  These timings describe one installed scene, not every Workshop project.
- Native imported GIF: screenshots observed both generated animation colors;
  preview rejected credential text and closed with Esc.
- Actual native greeter entry point against a fake greetd socket: keyboard username
  and masked password entry, session request, successful exit and persisted
  username/session ID passed. The test password was absent from application logs.
- Editor: shared wallpaper cards, native template thumbnails, widget canvas and
  monitor controls were inspected at 1264×900; the recorded editor frame rate was
  90 FPS. Widget dragging persisted normalized positions in the direction of the
  observed pointer movement; the workspace was also captured at 1000 pixels wide.
  The editor uses cached posters, not rendered scene thumbnails.
- Integration UI automation: both features disabled, Caelestia alone, independent
  login activation, template persistence, disabling/re-enabling and routing back
  to Library passed. The library-search shortcut did not leak into management pages.
- Existing normal scene/video previews and foreign-process preservation passed
  with the new runtime; the catalog still contained the installed library entries.
- Generated dedicated Hyprland Lua configuration passed `Hyprland --verify-config`.

## Local runtime and boot boundary

The immutable local 2.0.0 runtime was installed and the desktop renderer was handed
over only after replacement readiness, identity and wallpaper checks succeeded.
The application-menu launcher points to the new runtime. User preferences were
backed up, the old runtime was retained and desktop autostart/shortcuts were not changed.

A real scene export produced an independent allowlisted runtime/project/assets/font
bundle in 0.7–1.4 seconds across the recorded runs. Its native preview rendered with an empty HOME
and isolated desktop XDG paths. The root-owned installed payload was readable by
the package's unprivileged greeter user, which also executed the installed binary's
version check successfully. greetd/agreety 0.10.3-2 were installed; configuration,
next-boot service alias, configuration checksum and the original display-manager
backup were verified. A subsequent publication retained the original restoration
target. The first-login hint preserves the publishing desktop's UWSM session.
The currently running desktop session was not restarted.

An additional attempt to render under the greeter UID in a separate headless
Hyprland instance failed at compositor backend creation, before a native frame
could be drawn. Shortening the test runtime/socket path removed a socket-length
error but did not resolve the backend failure. This test therefore does **not**
validate pre-login graphics. It was isolated from the active VT and is recorded as
a limitation, not as a passing system-login test. This is distinct from real PAM/VT/cold-boot validation. The graphics and
IPC tests above do not prove a cold boot or successful login to every desktop.
A physical restart and genuine PAM login remain an explicit manual verification
boundary; no restart or logout is performed by the test suite.
