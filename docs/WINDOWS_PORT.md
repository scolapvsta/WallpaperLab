# Windows port assessment

Wallpaper Lab is currently verified on Arch Linux with Hyprland. A Windows build
requires a dedicated port. No Windows build or release has been verified.

## Reusable components

The catalog, profiles, translations, parts of the raylib UI, scene parsing and
shaders can be reused. The application currently depends on POSIX processes,
`/proc`, signals, file locks, XDG paths, shared objects, Hyprland and Linux output
selection. Windows-related CEF build conditions in the upstream engine do not
provide a complete Windows desktop backend for this application.

## Required work

Platform adapters are needed for desktop wallpaper hosting and Explorer restart
recovery, process lifecycle, global shortcuts, file selection, mixed-DPI monitors,
dependency loading and packaging. System audio capture needs a Windows mechanism
such as [WASAPI loopback](https://learn.microsoft.com/en-us/windows/win32/coreaudio/loopback-recording).
Monitor support must account for
[process DPI awareness](https://learn.microsoft.com/en-us/windows/win32/hidpi/setting-the-default-dpi-awareness-for-a-process)
and [mixed-DPI window hosting](https://learn.microsoft.com/en-us/windows/win32/hidpi/high-dpi-desktop-application-development-on-windows).
Media metadata and Steam integration also need Windows implementation and testing.
Caelestia integration would remain a Linux option.

The practical starting point is to separate OS APIs, build the UI with image/video
rendering, then add scene support and platform integrations.

## Historical planning estimate

The 0.17 assessment estimated 4–6 weeks for an image/video prototype and 3–5 months
in total for a broader beta approaching the Linux feature set, assuming one
full-time developer, Windows hardware and continuous testing. These are unverified
planning estimates, not a release schedule. Driver behavior, CEF/mpv rendering,
Explorer integration and project compatibility can substantially change the work.
