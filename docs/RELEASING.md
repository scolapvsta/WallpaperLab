# Publishing a beta release

Keep the repository public. Never publish user library assets, settings,
credentials, raw development runtimes or logs. Documentation-only changes can be
committed and pushed without changing the application version or rebuilding
existing release artifacts.

1. Implement changes; preserve the user's renderer and preferences. Set the full
   `0.x.y-beta.N` version and `channel: beta` in `build-manifest.json`; CMake and both
   packagers read that identity. Update the pinned download-helper version,
   README/BETA links, bug-report example, documentation and actual test results.
   The release-version check rejects accidental stable/post-1.0 identities.
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
   If those Arch libraries are extracted rather than installed on the build host,
   pass `--dependency-libs /path/to/extracted/usr/lib` for the version smoke check.
   That directory is never copied into the payload. Record this testing limit.
4. Test installer integrity/tamper rejection, clean install, upgrade and failed
   upgrade rollback with isolated HOME/XDG; smoke-test the actual released
   runtime (CLI, UI, scene/video/web and mute as applicable). Test installation
   twice and verify preferences survive. Clearly document platforms not tested.
5. Before making previously private history public, inspect all historical paths
   and blobs for credentials and unexpected private media/configuration. Do not
   assume .gitignore retroactively removes history. No force-push is needed.
6. Check `git diff --check`, stage only intended files, commit and push. Package
   that final commit so the binary manifest identifies its source. Include
   licenses in the payload, including CEF CREDITS.html and kissfft notices.
   Run `scripts/package-source.py` for the full source archive (including pinned
   submodules, not GitHub's root-only autoarchive). Audit source archive paths.
7. Create a GitHub **prerelease** named/tagged with the full beta version and
   publish the .run, full source archive, helper and SHA256SUMS. Hashes detect
   corruption, not publisher identity. Verify the final artifacts match the
   tested runtime before publishing.
8. Verify visibility, release links and downloadable asset hashes. Autostart
   changes remain explicit user actions. Retain old runtime backups on upgrade.

The initial beta targets up-to-date Arch x86_64 with Hyprland. Future package
manager/SONAME changes can require a rebuild. Do not advertise Windows/macOS or
all Linux compositors as supported by this artifact.

Attach `scripts/download-install.sh` as `Install-Wallpaper-Lab.sh` and include its
hash in SHA256SUMS. Use exact `/releases/download/v0.x.y-beta.N/…` URLs: GitHub's
stable-only `/releases/latest` endpoint does not select prereleases. The helper is
pinned to the release beside it; it must not silently fetch an older stable build.
Keep the versioned installer filename and full source archive in the release.

The old `v1.0.0` tag/assets are retained as a legacy experimental prerelease; do not
replace their binaries, hashes or tags. Its corrected notes direct readers to the
current beta. Older application version strings remain historical. A future stable
1.0 requires an explicit maintainer decision and deliberate version-policy changes,
not simply changing a GitHub release checkbox.

The icon master is `assets/wallpaper-lab.svg`. Run
`python3 scripts/generate-icon.py` with librsvg installed to regenerate the PNG
and embedded window icon. Wayland launchers use the desktop entry SVG; window
icons are also set where the window system supports them.
