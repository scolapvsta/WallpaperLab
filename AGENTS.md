# Wallpaper Lab contributor instructions

- Write repository documentation, code comments, release notes and test reports in
  English. Keep instructions applicable to any contributor and installation.
- Wallpaper Lab combines the animated selector derived from Hellpaper with the
  native linux-wallpaperengine renderer in a separate, customizable application.
  The application name is provisional.
- Ship one application executable, `wallpaper-lab`. Its `--render` mode links the
  engine library directly; it must not launch upstream application executables.
- Preserve all four selector layouts and their animation behavior. Distinguish
  Workshop thumbnail previews from full scene rendering. Describe desktop
  transitions and project compatibility only to the extent implemented and tested.
- Pin upstream dependencies through submodules. Record engine/raylib changes in
  ordered patches and update pins only after comparison and testing. Preserve
  existing local changes when applying patches or updating submodules.
- Before signaling a renderer, verify its PID, start time and executable. Keep
  the previous wallpaper running if its replacement fails to start.
- Keep backups, logs, screenshots, builds and library data in `local/` or XDG
  directories. Never commit Workshop assets, credentials or machine-specific data.
- Run catalog and process lifecycle tests plus relevant graphical checks. Record
  actual outcomes and limitations. Back up configuration before autostart changes;
  enabling autostart or shortcuts requires an explicit user action.
- Keep the repository public. For each completed change, update documentation and
  test results, commit intended files and synchronize GitHub within the authorized
  task scope. Documentation-only changes do not require a new application version
  or a rebuilt binary release.
- Beta releases may include the runtime prepared and verified by the packaging
  allowlist. Exclude Workshop assets, user data and raw development builds.
  Clearly document partial features and unsupported platforms.
- Preserve upstream copyright and license notices. Use generic paths and examples
  in public documentation; retain canonical project links where needed.
