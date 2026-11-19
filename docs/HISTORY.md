# Version 2.3.0 — login library and editor polish

- Reuse Library filters and image/crop controls in the native login workspace,
  with an inspector preview and independently stored resolution variants.
- Make search focus, caret and text labels explicit; space the two workspace tabs
  evenly and place grid tools alongside the canvas.
- Add shared visual HSV/RGB pickers, cached wallpaper-derived light/dark palettes,
  UI/widget reset with backups and pixel-based widget positioning.
- Remove analog/ring clocks and their selector, migrating older settings to digital.

# Version 2.2.0 — native login customization

- Combine wallpapers, output configuration and monitor preview in one tab.
  Group unsupported wallpapers at the end and explain actual project limitations.
- Keep Clock, Date and Login UI widgets; add grid snapping, independent fonts,
  colours, text effects, backgrounds, borders and alignment controls.
- Add analog clock variants, 12/24-hour digital formats, seconds and date formats.
  Move welcome controls into Login UI with dynamic greetings and editable labels.
- Make preview fields interactive without authentication or input persistence.
  Replace session cycling with a native dropdown and keyboard navigation.
- Export per-widget fonts for pre-login use. See
  [customization validation](LOGIN_CUSTOMIZATION_TEST_RESULTS.md).

# Version 2.1.0 — native login refinement

- Use start-hyprland for the private login compositor and restore optional GNOME
  Keyring PAM unlock with backups and conditional rollback.
- Replace the preset gallery with one editable base UI and remove the calendar.
  Separate Widget/Appearance tabs, tighten monitor controls and add animated
  password dots, caret navigation, field styling and visibility controls.
- Keep live previews alive after their launch worker exits. Cache native scene
  stills with the source aspect ratio and use saved panorama geometry.
- Store connector IDs and support different wallpapers, creator properties and
  image adjustments for independent outputs.
- See [refinement validation](LOGIN_REFINEMENT_TEST_RESULTS.md) for actual outcomes.

# Version 2.0.0 — native login screen (experimental source update)

- Replace the SDDM/Qt integration with a separate optional native editor and
  a greetd-backed login session using the same application executable.
- Share catalog thumbnails and cache native UI canvases for immediate editing;
  draw real scene/video/GIF previews directly in the engine graphics context.
- Add five original templates, movable widgets, fonts, appearance and independent
  wallpaper image profiles. Keep single/mirror/panorama and UI monitor selection.
- Export a standalone pre-login runtime, wallpaper, assets and font. Publish
  immutable system bundles after administrator authentication, with backup and
  restoration of the original display manager. Never restart the live session.
- Preserve Caelestia, all four carousel layouts and the responsive Cyberpunk header.
- See [native login verification](NATIVE_LOGIN_TEST_RESULTS.md) for tested behavior
  and boot/session limitations. The downloadable stable installer remains 1.0.0.

# Version 1.4.0 — source update

- Render compatible native scenes inside SDDM previews through one hidden,
  bounded Wallpaper Lab renderer and an atomic RGB feed shared by greeter views.
- Add Mirror/Panoramic controls and combined multi-monitor captures. Apply image
  controls to the whole virtual canvas before each monitor samples its region.
- Verify renderer readiness and delivered frames before publishing a selection;
  preserve the previous theme on failure. Verify child identity before stopping
  it and terminate the scene renderer if its helper dies.
- Keep scene rendering experimental: up to 30 FPS/2560×1440, original project and
  assets required, no scene interaction/audio or desktop property/dependency export.
  Web projects and real pre-login deployment remain unsupported.
- See [native scene and panorama checks](SDDM_SCENE_TEST_RESULTS.md). The public
  downloadable stable installer remains 1.0.0.

# Version 1.3.0 — source update

- Replace SDDM's selection dropdowns with separate wallpaper and login UI card
  catalogs, search, filters, independent favorites and a resizable inspector.
- Add per-wallpaper image adjustments and crop/zoom controls, with immediate
  editor feedback and validated saving. The launcher and Qt greeter share colour
  and geometry shader code; GIFs and videos retain animation.
- Generate actual preset and combined greeter captures in a background worker.
  Offer bounded live wallpaper-only and full-login previews.
- Keep desktop profiles, original media and system SDDM configuration separate.
  Native engine scenes/web content and boot-time publication remain unsupported.
- Fix repeated diagnostic queries invalidating status-bar text during drawing.
- See [library/editor validation](SDDM_LIBRARY_TEST_RESULTS.md). The public
  downloadable stable installer remains 1.0.0.

# Version 1.2.2 — source update

- Keep the animated Cyberpunk 2077 wordmark, subtitle and barcode within the
  header's available height, using measured font dimensions. Hide the decoration
  when the navigation leaves insufficient horizontal room, including the optional
  Integrations tab.
- Includes the 1.2.0 experimental SDDM Astronaut preview and 1.2.1 shared
  Integrations page: enable Caelestia/SDDM in Settings and manage enabled
  integrations after Workshop. SDDM remains a test-mode preview; system greeter
  installation and engine scene playback before login are not implemented.
- A local source installation was upgraded and verified separately from the
  development preview. The public downloadable stable installer remains 1.0.0.
- See [header and local installation validation](HEADER_TEST_RESULTS.md).

# Version 1.1.0 — source update

- Integrated kylan11's output reconnect fix (#2), rendering GPU selector (#3)
  and Hyprland fullscreen visibility fix (#4).
- Integrated Belial02's optional awww backend, Caelestia CLI colour matching,
  persistent carousel and editable transition timing (#6, superseding #5).
- Follow-up fullscreen filtering preserves active-window and app-exclusion
  policies. Overlapping patch upgrades preserve existing local source edits;
  engine build flags support source directories containing spaces.
- Default backend, carousel hold behavior, autostart and shortcuts stay opt-in
  as documented. awww requires an external daemon and does not support panorama.
- This source update does not publish a replacement for the 1.0.0 installer.
  See the dated verification record for locally repeated tests and limitations.

# Version 1.0.0

- First stable numbered release for Arch Linux x86_64; Hyprland remains the tested desktop.
- Minimal high-contrast landscape icon in the repository, application windows and launcher.
- Includes all source changes through 0.29: four animated selector layouts, multilingual interface, named monitors, live zoom and optional Caelestia integration.
- Stable download links and checksum-verifying installer helper; existing settings and rollback behavior retained.
- Windows/macOS and complete Wallpaper Engine scene compatibility remain unsupported.

# Development history

This public summary records implementation milestones through version 0.29.
Historical behavior and test counts apply to their named versions; current usage
is in the [user guide](USER_GUIDE.md), and verification limits are in
[test results](../tests/RESULTS.md).

## 0.1–0.3.5 — Native application and animated selector

Combined a Hellpaper-derived selector with the linux-wallpaperengine library in
one `wallpaper-lab` executable. Rendering uses a separate `--render` instance.
Added catalog discovery, native scene/video/web previews, persistent settings,
safe renderer replacement and favorite navigation. The selector gained Grid,
River, Vertical and Wave layouts, transparent compact surfaces, rounded masks,
zoom, fades and continuous movement. Desktop crossfade was not implemented at
this stage. Stereo music-response capture and web/SceneScript audio bridges
followed in 0.3.5.

## 0.4–0.5 — Persistent library UI

Introduced a shared library shell, filters, live-scene actions, display selection,
recoverable trash, shortcut capture and Unicode font fallback. The UI and
selector gained consistent controls and independent scrolling. Early mouse tests
blocked by a locked compositor were explicitly left unverified. At this stage,
one-monitor selection still replaced the active renderer group.

## 0.6 — Physical display calibration

Added a panel-size display canvas, logical-pixel fallback, snapping, alignment,
overlap validation and named-connector persistence. The renderer receives an
immutable layout snapshot; compositor monitor/window/cursor geometry is unchanged.
Calibration applies to panorama and preserves native output resolution.

## 0.7–0.8 — Placement, library and scene interaction

Added per-project pan/zoom, live controls, filtering, active-wallpaper feedback and
carousel integration. Scene compatibility work covered padded textures, legacy
JSON recovery, clipping shaders, hidden compose layers and SceneScript visibility,
vectors, angles and cursor events. Creator properties gained validation,
per-project persistence and renderer argument forwarding. Mouse coordinates were
corrected for nonzero output origins and display scaling.

Advanced file/directory/scenetexture controls, compound conditions and full 3D
scene support remained incomplete.

## 0.9–0.11 — Customization, compatibility and audio

Expanded themes, languages, fonts, refresh behavior and inspector controls.
Catalog audits exercised single, mirror and panorama modes and identified
perspective-camera/MDL projects as a remaining unsupported class. Dependency-based
presets gained local resolution without modifying Steam files. Web compatibility
work improved file URLs, CEF dirty-region uploads and event delivery. Scene sound
controls, creator volume, audio lifecycle, graphical color picking and live zoom
were improved. Audit success meant readiness and brief survival, rather than
certification of every visual effect.

## 0.12 — Native Workshop browser

Added Steam ISteamUGC020 queries, manual callbacks, a dedicated worker and HTTPS
thumbnail fetching. Search, type/resolution/audio filters, author lookup, direct
IDs, details, dependency downloads and subscriptions use the running Steam
client. Downloads require an account owning Wallpaper Engine. Active deletion
selects a replacement safely and supports rollback on failure. Wallpaper Lab
does not launch WallpaperX or upstream application executables.

## 0.13 — Independent outputs, media and desktop transitions

One-monitor mode gained independent renderer assignments and group restore.
MPRIS metadata/timelines reached scene and web scripts, including late script
initialization. Menus became overlays with input blocking. Desktop effects use
native snapshots of the previous wallpaper; a missing capture falls back to
switching without animation. Locale coverage expanded to 22 languages, with
partial advanced translations at this stage.

## 0.14 — Image controls and navigation

Stabilized eight transition effects and added live brightness, contrast and
saturation profiles. Fixed text-cache eviction with queued drawing, expanded
pagination to 50/100/200 results and introduced grouped-resolution queries.
Per-project playback controls use global-disable precedence. UI transparency
became independent of opaque media/text. Localization checks cover UI keys and
format safety; they do not replace native-speaker review.

## 0.15 — Local media, active anchor and optional Caelestia

Added staged image/video imports, generated metadata/thumbnails, deduplication,
configurable library roots and source filters. Selecting an active wallpaper
became a no-op. The switcher starts at the active project, including a temporary
nonfavorite anchor. Theme presets became independent of transparency. Optional
Caelestia palette publication gained conditional restoration and stale-result
cancellation, initially using the project thumbnail.

## 0.16 — Animation customization and image transforms

Added switcher size/zoom/timing/inversion/Wave preferences and shared transition
duration/curve/direction/zoom controls. Extended image tuning with hue,
temperature, gamma, vignette, rotation and flips. Improved icon rendering and
Pango text batching. The optional Material You helper gained four generators,
private dependency preparation and rendered-frame sampling with thumbnail
fallback. Palette publication remains global and event-driven.

## 0.17 — Per-output transition handoff

Separated inspector frames from immutable transition snapshots using session and
output identity. A shared monotonic start gate synchronizes animation after
renderer readiness and retirement of the previous group. Added direct flip
switches and atomic image reset. Expanded translation/rendering checks across
22 locales. A Windows port assessment was documented; no Windows port was built.

## 0.18 — Compositor isolation and temporary anchors

Desktop pixel tests identified compositor slide/fade being added after native
framebuffer composition. A targeted Hyprland runtime rule now isolates
`wallpaper-lab-background`. Added defensive RGBA state handling, antialiased iris
edges and corrected pointer radius. Temporary nonfavorite anchors now load their
thumbnails and are skipped on subsequent wraps in all layouts. Historical theme
collections grouped 29 presets.

## 0.19 — Binary controls

Unified two-option choices and booleans with toggle components, preserving
creator values, independent multi-select states and larger dropdowns. Input
clipping and overlay blocking remain shared. Renderer behavior and profile
formats were unchanged.

## 0.20–0.21 — Theme designs and app backgrounds

Introduced Strawberry core as a complete UI design, followed by 2077, scoped
palettes and remembered backgrounds. Imported app backgrounds are copied as
static PNGs, with rejection preserving the prior selection. Historical palettes
left the active menu while migration support remained. Native drawings implement
visual references without distributing reference assets.

## 0.22 — Fonts, web mute and Workshop performance

Bundled pinned OFL fonts, contextual font menus and four backgrounds per theme.
Theme mode became fixed. Text measurement was separated from texture creation,
thumbnail upload was bounded and Workshop scroll behavior was improved. Live
host mute now covers CEF media/Web Audio/iframes and MPV/SDL without renderer
restart; reactive audio capture remains independent.

## 0.23 — Four themes and themed switchers

Added Default and Pixel Theme, reaching four designs, 21 palettes and 16 native
backgrounds. Added Silkscreen, per-theme suggested fonts and an opt-in General
Fonts override. All four switcher layouts follow theme, palette and font in both
preview and real popup. Pixel drawings are original; reduced motion freezes
decorative animation.

## 0.24 — Public Arch beta

Published `v0.24.0-beta.1` with the same native feature set as 0.23, a guided
Arch x86_64 installer, dependency checks, separate hash-pinned Steamworks download,
active-renderer upgrade handoff and rollback. Binary packaging uses an allowlist;
the full source archive includes pinned submodules and patches. Public guides,
bug reporting and release procedures accompany the beta. Verification is limited
to the documented Arch/Hyprland environment.

The subsequent documentation cleanup standardized repository prose in English,
removed contributor-specific session notes and generalized examples. Application
translations, upstream attributions and canonical project URLs are retained.

## Repository rename — 2026-10-30

The public GitHub repository is now named `WallpaperLab`. Clone, release and bug
report links use the new canonical location. Application branding, executable
(`wallpaper-lab`), configuration paths and existing release artifacts are
unchanged; existing checkouts can update their `origin` remote to the new URL.

## 0.25 — Independent switcher themes and Arcade

Added Theme and Color palette to switcher customization, with separate saved
preferences and per-theme palette recall. Removed automatic UI-to-switcher
appearance changes. All four layouts share decorative tray, card and margin
artwork in preview and real popups. Simplified Strawberry core's rabbit and added
miniature cakes. Renamed Pixel Theme to Arcade without changing saved IDs;
redesigned its four scenes with neon cabinets, prize machines and candy/starlit
pixel scenery. Added a reduced-motion-aware scripted Pac-Man chase, power pellet,
frightened ghosts, capture scores and returning eyes.

## 0.26 — More Default colors and plain switchers

Expanded Default from three to 20 palettes: 10 light and 10 dark. Light palettes
now drive light surfaces and dark text consistently. Removed switcher themes,
illustrations, decorative margin bands and card motifs. All four switcher
layouts retain their geometry and animation and expose only Color palette for
appearance, independently of the UI. Old themed switcher colors migrate to the
closest Default accent in the same mode. Other UI themes remain available for
later design work.

## 0.27 — Monitor controls and application lifetime

Added named monitor dropdowns, model/description fallbacks and bounded map labels.
The live image zoom range is now 25–300%, with black padding when shrinking.
Stop and quit stops every owned output and restores the previous Caelestia palette
conditionally. Duplicate library launches focus the existing instance; renderer
and CLI lifetimes stay independent. Caelestia can use a saved source monitor for
its global palette, independently of the application's current monitor selection.
Upstream and installed Caelestia still use a global Colours singleton.

## 0.28 — Compact power control and vivid Arcade artwork

Replaced the stop text button with a power icon and reserved header space across
all four designs. Narrow windows use compact navigation and decorative art stays
clear of controls. Arcade retains its palette IDs with more saturated colours,
new animated pixel tributes, coins and prize claws, a visible header parade and
raised rectangular or round cabinet-style buttons. Reduced motion freezes all
automatic Arcade decoration. The four carousel layouts keep their plain styling.

## 0.29 — Consistent interface localization

Expanded the localization audit to C++ diagnostics and dynamic labels. Steam
initialization/download failures, Caelestia and controller errors now use localized
summaries with original details in a local log. Fixed Workshop taxonomy labels,
appearance controls and native file filters follow the interface language.
Stored notices and existing worker errors relocalize after a language change.
The catalog contains 429 entries across 22 languages; metadata and named artwork
remain original. Hidden native captures exercise Steam failures in every locale.

## 1.2.0 development — SDDM integration preview

Added a verified opt-in SDDM tab, an installed/local/Workshop media catalog and
independent Astronaut login UI presets, position and blur. Native media is copied
into an isolated theme and checked with the real greeter before publication.
Failures preserve the prior selection. Bounded test-mode previews work without
logging out. System installation, boot activation and native engine scenes in the
greeter remain separate future work. See the SDDM guide and test report.

## 1.2.1 development — Shared Integrations page

Settings now contains independent Caelestia and SDDM activation switches. Enabling
either exposes one Integrations tab after Workshop, with only the enabled panels.
Caelestia controls and the SDDM editor moved into that page. Disabling the current
panel selects the remaining integration; disabling the last returns to Library.
Preferences survive panel navigation and toggling. The SDDM health gate remains.
