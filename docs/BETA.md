# Open beta — Arch Linux x86_64

Version: **0.24.0-beta.1** (application reports 0.24.0).
The package includes the native app, patched renderer, web support and themed
switchers. Settings, wallpaper libraries, Steam account data and credentials
are excluded.

## Install

1. Update Arch normally before testing; the build needs glibc >= 2.44.
2. Download `Wallpaper-Lab-0.24.0-beta.1-Arch-x86_64.run` from the
   [release](https://github.com/scolapvsta/wallpaper-engine-customized/releases/tag/v0.24.0-beta.1).
3. From a terminal in that folder, run:

   ```sh
   bash Wallpaper-Lab-0.24.0-beta.1-Arch-x86_64.run
   ```

Run as your desktop user. The installer asks before installing missing Arch
packages through sudo/pacman. It does not perform an unattended system upgrade.
It offers to prepare Workshop support by downloading the separate Valve runtime
from the pinned steamworks.js package; both archive and library hashes are
checked, and no npm/JavaScript code is executed. Internet is needed for missing
packages and this optional download. This is a third-party Valve component with
its own terms, not an app source dependency covered by GPL.

Open **Wallpaper Lab (Open Beta)** from your applications menu. The launcher is
`~/.local/bin/wallpaper-lab`; use that absolute path if `.local/bin` is not on PATH.
Full desktop operation is verified on Hyprland. The installer warns on other
desktops; it does not install a compositor or modify its configuration.

For Workshop, open Steam, sign in with an account owning Wallpaper Engine, and
install Wallpaper Engine via Steam. The app detects its library/assets. The
Steam runtime alone does not grant access to the game or its Workshop content.
An existing native Steam install is recommended for this beta; Flatpak Steam
catalog paths are detected, but native Steam API connectivity under Flatpak has
not been certified.

## Updates and rollback

Run a newer installer in the same way. Releases are stored separately in
`$XDG_DATA_HOME/wallpaper-lab/releases/` (default `~/.local/share`). Existing
preferences and libraries are retained. If a wallpaper is active, the installer
starts and verifies its replacement before terminating the old renderer,
checking PID, start time and executable. If startup fails, it restores the old
launcher/state and keeps the old renderer. Previous runtime directories remain
available; the previous launcher and menu entries are saved under
`wallpaper-lab/installer-backups/`. Close old UI windows after upgrading and open
the new launcher. No changes to autostart or global shortcuts are automatic.

To restore a saved launcher, close the current UI, stop the new wallpaper using
`~/.local/bin/wallpaper-lab --stop`, copy the selected backup `launcher` over
`~/.local/bin/wallpaper-lab`, then run it with `--restore`. Keep the runtime path
referenced by that launcher. Preferences are not downgraded by this operation.

## Verify or uninstall

```sh
sha256sum -c SHA256SUMS
bash Wallpaper-Lab-0.24.0-beta.1-Arch-x86_64.run --check
bash Wallpaper-Lab-0.24.0-beta.1-Arch-x86_64.run --uninstall
```

`--check` verifies the embedded payload, dependencies and CLI, without replacing
an installation or downloading Steamworks. `--uninstall` stops this app's current
renderer and removes the beta launcher/menu; it retains preferences, imported
wallpapers and release backups. It requires a beta installer receipt and will
not remove an unrelated developer installation.

Other flags: `--no-launch`, `--skip-steam`, `--yes` (accept installer prompts;
pacman may still request authentication/confirmation). Dependencies remain normal
system packages. The installer intentionally does not remove them on uninstall.

## Troubleshooting and feedback

- **Library/GLIBC symbol missing:** update Arch fully, install the dependency list
  printed by the installer, then retry. Do not partially upgrade glibc alone.
  This rolling-release binary may require a rebuilt release after future SONAME
  changes; an AppImage-like compatibility promise is not made.
- **Workshop unavailable:** check Steam is open, ownership and game installation;
  rerun installer without `--skip-steam`. The app checks Steam ownership itself.
- **UI does not open:** launch from a terminal. Installer launch output is saved
  in `$XDG_STATE_HOME/wallpaper-lab/beta-launch.log` (default `~/.local/state`).
- **Background not supported:** report its public Workshop ID and project type.
  Not all scene/SceneScript features from Windows Wallpaper Engine are supported.
- **Other desktop:** include compositor/version; Hyprland is the tested baseline.

Use the repository bug-report form. Include app/compositor/GPU versions and
reproduction steps, and review any logs before sharing. Reports are manual;
no telemetry or automatic log upload is added by the installer.
