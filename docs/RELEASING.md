# Publishing a beta

The repository is public at `scolapvsta/wallpaper-engine-customized`, explicitly
authorized for the open beta on 29 October 2026. Never publish personal library
assets, settings, credentials, raw development runtimes or logs.

1. Implement changes; preserve the user's renderer and preferences. Update CMake
   version, build-manifest.json, documentation and actual test results.
2. Keep upstream pins fixed. Record all engine/raylib changes in ordered patches.
   Check the patch/font hashes and run CTest plus relevant graphical integrations.
3. Build the allowlisted package with:

   ```sh
   python3 scripts/package-beta.py --output local/releases
   ```

   CMake installs app and engine with relative runtime paths. Only CEF's known
   files, kissfft, the app/font/helper files and licenses are copied. No installed
   personal runtime directory is copied. System libraries are required Arch
   packages in `packaging/arch-dependencies.json`. Steamworks is an optional,
   separately downloaded dependency pinned in `packaging/steam-runtime.json`.
4. Test installer integrity/tamper rejection, clean install, upgrade and failed
   upgrade rollback with isolated HOME/XDG; smoke-test the actual released
   runtime (CLI, UI, scene/video/web and mute as applicable). Test installation
   twice and verify preferences survive. Clearly document platforms not tested.
5. Before making previously private history public, inspect all historical paths
   and blobs for credentials and unexpected private media/configuration. Do not
   assume .gitignore retroactively removes history. No force-push is needed.
6. Check `git diff --check`, stage only intended files, commit and push. Create
   a beta prerelease and publish the .run, SHA256SUMS and full source archive
   (including initialized pinned submodules, not GitHub's root-only autoarchive).
   Use `scripts/package-source.py` for that archive. Audit source archive paths.
7. Package the final commit so the binary manifest identifies its source. Attach
   licenses to the payload, including CEF CREDITS.html and kissfft notices. Keep
   SHA256SUMS in the release; hashes detect corruption, not publisher identity.
8. Verify visibility, release links and downloadable asset hashes. Autostart
   changes remain explicit user actions. Retain old runtime backups on upgrade.

The initial beta targets up-to-date Arch x86_64 with Hyprland. Future package
manager/SONAME changes can require a rebuild. Do not advertise Windows/macOS or
all Linux compositors as supported by this artifact.
