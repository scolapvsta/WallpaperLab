# Development history

This public summary records implementation milestones through the 0.24 open beta.
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
