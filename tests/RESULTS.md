# Verification results

These are recorded outcomes for the versions named below. Historical tests were
not rerun merely to edit this document. The public summary retains technical
results and limitations while omitting workstation session notes and local
wallpaper choices. Logs, screenshots, generated fixtures and library assets stay
outside Git under `local/` or isolated XDG directories.

## Editable transition timing — 2026-11-11

- awww step is limited to 1–100 throughout preferences, hook import and backend
  arguments. Duration remains limited to 0.1–10 seconds and is now displayed in
  seconds rather than milliseconds. Existing oversized settings are bounded.
- Inline and customization timing controls accept direct numeric input and
  provide vector-drawn up/down buttons. Buttons change step by 1 or duration by
  1 second; fractional durations are preserved. Enter/outside click commits,
  Escape cancels only the field, and empty/nonfinite/malformed input is discarded.
- All **16 automated checks passed**, including numeric parsing, decimal comma,
  upper/lower bounds, invalid/nonfinite input and backend argument caps.
- Explicit Hyprland GUI checks passed for both control locations: typing integer
  and fractional values, button changes, keyboard Up, maximum disabled buttons,
  slider endpoints, click-away commit, invalid/empty input preservation and Esc
  keeping the settings/customization dialog open. Native screenshots were
  inspected; arrows use geometry rather than a font glyph.
- Installed the updated runtime with backups. Verified both timing bounds while
  preserving the wallpaper, shortcut configuration and carousel preference.

## Optional persistent carousel — 2026-11-11

- Added **Keep carousel open** beside the carousel shortcut settings. It defaults
  to off and persists independently of shortcut activation or the generated bind.
- Persistent mode ignores modifier-release confirmation, keeps subsequent
  shortcut taps in the same window, and closes on Enter/click confirmation or
  Esc/outside-click cancellation. It has no idle timeout. Both modes support
  arrows and Tab; keyboard rotation preserves the temporary nonfavorite anchor.
- All **16 CTest checks passed**, including catalog/lifecycle, backward wrap,
  grid-row navigation, transient-anchor exclusion and opt-in preference defaults.
- Explicit Hyprland desktop checks passed for all four carousel layouts using
  real virtual-keyboard input and isolated XDG state with a fake awww daemon:
  early shortcut release kept the persistent popup open, another shortcut tap
  advanced the same window, late old releases were ignored, arrows and Tab
  navigated, Enter applied and Esc cancelled. Hold-mode release still confirmed.
  The personal wallpaper and application state were unchanged by these checks.
- The GUI toggle round trip saved both on/off values without creating or changing
  shortcut configuration. Its placement was visually inspected. All 441 messages
  passed glyph checks across 22 languages. Installed the updated runtime with
  backups, retained the existing shortcut and wallpaper, and verified the new
  setting remains off by default.

## awww timing sliders and CLI palette parity — 2026-11-11

- Added Step and Duration sliders directly under the awww transition selector in
  Carousel settings. Values persist to the existing awww preferences and are
  passed as step and seconds; the customize dialog remains available.
- The local-image colour path now calls the installed Caelestia CLI on the media
  file, preserving its smart mode/variant, flavour, thumbnail/quantization cache
  and theme application. The CLI wallpaper hook is suppressed in memory so it
  cannot override GUI transition values or duplicate the imported trailing hook.
  CLI theme hooks remain enabled; no user CLI configuration is rewritten.
- Real CLI adapter tests passed in isolated XDG directories: generated colours
  and metadata matched `caelestia wallpaper -f` exactly for default and hard
  flavours. Theme hooks ran, wallpaper hooks were suppressed only by the adapter,
  wallpaper links updated, and CLI configuration stayed byte-identical.
- Controller lifecycle coverage checks that the CLI adapter receives the actual
  media file and publishes its returned variant instead of generating a preview
  tint. Native scene/video palette generation remains unchanged.
- All **16 CTest checks passed**. Hyprland virtual-pointer checks changed both
  inline sliders and verified persisted values; duration snaps to 100-ms steps.
  The inline controls were visually inspected. Pango checked 439 messages across
  all 22 languages without missing glyphs.
- Installed the updated runtime and reapplied the active image: the complete
  resulting scheme matched the CLI's expected colours and metadata exactly.
  Theme/cursor hooks completed, CLI configuration stayed byte-identical, and the
  previous runtime, preferences and scheme were backed up before handoff.

## Custom awww backend — 2026-11-11

- App rebuilt against the pinned source headers and the installed 1.0 engine/CEF
  libraries; the engine itself was not rebuilt or changed for this feature.
- All **15 CTest checks passed**, including catalog, preferences, parser,
  localization, installer/download checks, the new awww argument/import test,
  and the isolated controller lifecycle test.
- Lifecycle checks used a fake awww daemon and real application supervisor
  processes: CLI import and backups, actual media-path arguments, cursor
  coordinates, image handoff, carousel advance, saved assignment restore, failed launch retaining its
  predecessor, post-hook failure retaining its wallpaper, explicit Stop, and
  preservation of an unrelated process/image all passed. An external wallpaper
  change ended the app's supervisor without clearing the new image. Unsupported
  imports left settings unchanged. This is not a real multi-monitor certification.
- Hyprland/Wayland GUI checks with virtual pointer input passed: the backend
  dropdown, awww transition customization, origin/Y controls, and persisted
  144-FPS slider value. Playback and transition screenshots were visually checked.
- Pango/Cairo checked all 438 messages across 22 languages: no missing glyphs.
  Translation coverage and format checks passed. Technical awww effect/origin
  names remain literal command values.
- Real awww image application on one Hyprland output passed using outer, step 2,
  2-second duration, 144 FPS, cursor origin and Y inversion. The imported trailing
  hook also completed successfully. Native image/video preview windows remained
  functional, and video planning still selected the embedded renderer.
- Installed a separate custom runtime with the original launcher, preferences
  and runtime metadata backed up. The active native image was handed over to
  awww; the original Caelestia CLI configuration remained byte-identical.
- awww daemon startup remains the session's responsibility. Panorama is rejected
  for multiple outputs; mixed-DPI cursor alignment and real multi-output awww
  handoff were not verified. Native image transforms are not implemented by the
  awww backend. The published 1.0 installer was not rebuilt or replaced.

The verified desktop environment is Arch Linux x86_64 with Hyprland and a
two-output setup. Readiness, short survival, shader output, compositor pixels and
UI interaction are different checks; a pass in one is not certification of all
Wallpaper Engine projects, drivers or desktops. Performance measurements below
are observations from the test environment.

## 1.0.0 — Release, icon and installer — 2026-11-05

- Release build and CTest: **13/13 passed**, including catalog, preferences, installer integrity and checksum download helper (valid and tampered payloads).
- Actual packaged installer in isolated HOME/XDG: clean install, upgrade, preserved preferences/backups, live synthetic renderer handoff, failed-upgrade rollback, check mode and uninstall retention all passed. System package queries used the existing test stub with extracted dependency libraries; this is not a fresh-machine pacman installation test.
- Optional Steamworks hash verification and an actual Steam Workshop query passed.
- Native scene/video previews and preservation of an unrelated process passed. All four interface themes completed the graphical smoke sequence; the final header icon was visually inspected and its smoke test repeated. SVG/PNG icon inspected at 256 px. Wayland uses the desktop entry icon; an X11 window-manager icon was not independently verified.
- Full source archive: 12,252 members, safe paths, standalone patch application, and all 109 patched inputs identical to the built worktree.
- Tracked content and packaged payload checked for personal names/home paths: no matches. History audit checked 520 existing blobs without credential-pattern matches or unexpected private files. Canonical repository URLs and upstream attribution are retained. Installed app/renderer have no development RPATH.
- Logs, screenshots, fixtures and runtime artifacts remain local and are excluded from Git. No Workshop assets or user settings are packaged.
- Platform coverage remains Arch Linux x86_64 / Hyprland. Other compositors, Windows/macOS and complete Wallpaper Engine compatibility are not certified. Previous feature tests below remain historical unless explicitly repeated above.

## 0.29.0 — Interface localization audit — 2026-11-03

- Release build and all 12 CTest checks passed. The catalog now contains 429
  entries with all 21 translated columns plus canonical Italian. The source audit
  covers C/C++ and headers, conditional labels, formatted notices, dynamic tables
  and 133 explicitly registered diagnostic prefixes. Unregistered diagnostics,
  untranslated direct UI labels, missing cells and mismatched formats fail checks.
- All catalog entries and diagnostic routes were exercised in all 22 locales.
  Tests verify the reported Steam IPC initialization failure, unknown vendor
  errors, existing errors after a language switch, formatted notification changes
  and stable string lifetimes. UTF-8 notice arguments remain untouched.
- Pango/Cairo audited every message in every locale: zero missing glyphs. Native
  hidden framebuffer captures reproduced the Steam error in all 22 languages
  using a fake SDK, without Steam access or network requests. An English → German
  → English switch relocalized the existing error without pressing Retry.
  English, Arabic and Chinese native captures were visually inspected. Native
  capture replaced compositor screenshots that could be occluded by other apps.
- Fixed Workshop taxonomy labels, appearance/font controls and file picker
  filters are localized. Raw Steam query tags, creator metadata and named artwork
  remain unchanged. Technical error details are retained in the local diagnostic
  log; visible summaries and the log-location label follow the chosen language.
- The lifecycle integration scanned 106 catalog entries, opened native scene and
  video previews and preserved a foreign process during stop. Localization
  tests preserved the existing desktop renderer. The local 0.29.0 runtime was
  installed and restored the saved wallpaper.

Evidence remains under `local/i18n-029/`. These checks establish source coverage,
format safety, glyph availability and the tested rendered paths. They are not a
native-speaker certification of every translation or every possible UI layout.
CLI usage and raw developer/vendor logs are technical output; author content,
font family names, theme names and protocol identifiers are not translated.

## 0.28.0 — Power icon and Arcade refresh — 2026-11-03

- Release application build and all 12 CTest checks passed, including catalog,
  display layout, preferences, translations and Caelestia restoration.
- `arcade_header_integration.py` passed with isolated XDG state and a read-only
  fixture library. All four designs were captured at 1000, 1440 and 2200 pixels
  wide. The compact power hitbox stayed clear of the navigation and right-side
  decoration. Arcade character animation changed captured pixels; reduced motion
  produced identical settled header frames. Screenshots were visually inspected.
- The standalone artwork probe captured all four Arcade backgrounds, a character
  and button contact sheet, and eight chase stages. Sprite rows were validated as
  16 × 16. Small icon controls are round cabinet buttons; text controls have raised
  bezels. The four palettes retain their IDs with brighter, saturated accents.
- `monitor_session_integration.py --desktop` passed: clicking the new power icon
  stopped both owned outputs, exited the library and restored the original test
  palette. Monitor dropdowns, live zoom-out padding, independent palette source,
  failed replacement preservation, duplicate launches and crash reopening passed.
- The lifecycle integration scanned 106 entries, opened native scene/video
  previews and preserved a foreign process during stop. Tests used synthetic
  libraries and preserved the existing desktop renderer and palette.
- The hidden native popup regression passed 32 captures over 20 independent
  colours and all four layouts, retaining plain styling and transparent margins.
  Its initial ownership check overlapped runtime installation; the isolated rerun
  after installation passed with the personal renderer unchanged.
- The local runtime was installed as 0.28.0 and restored the saved wallpaper.

The first still-frame comparison caught compositor resize motion; the final
visual test waits for settled geometry before comparing frames. Character art is
hand-drawn in code, with no extracted game sprites or asset packs. The decoration
is scripted, not playable game logic. Visual checks cover the tested Hyprland
session and fixture library; performance and other desktops are not certified.
Evidence remains under `local/arcade-028/` and is excluded from Git.

## 0.27.0 — Monitor controls and application lifetime — 2026-11-03

- Release engine library and application builds passed. All 12 CTest checks passed,
  including catalog, display layout, preferences,
  Caelestia restoration, translations and installer checks. New assertions cover
  zoom-out geometry, rejection of zero/negative scales, persisted zoom below 100%
  and monitor model/description/connector fallbacks.
- `monitor_session_integration.py --desktop` passed on two Hyprland outputs with
  disposable XDG directories and local images. It verified monitor labels, named
  two-monitor dropdowns, an independently persisted palette source, changes through
  the actual Caelestia settings dropdown, two surviving renderers after a failed
  replacement, and a native 50% zoom frame with black borders and a visible center.
- Four duplicate launch paths exited while the existing library and renderers
  remained alive. Stop and quit closed the library, stopped both owned outputs and
  restored the original test palette. Reopening worked after normal exit and
  SIGKILL. The existing desktop renderers and palette were preserved during tests.
- The standard lifecycle integration scanned 106 catalog entries, opened native
  scene and video previews, and preserved a foreign process during `--stop`.
- Screenshots of monitor selection, Caelestia source selection and the zoomed
  framebuffer were inspected. Small monitor map cards retain connector labels;
  full model, connector and resolution remain available below the map and in the
  expanded dropdown. Evidence and generated fixtures remain under `local/`.
- The ordered engine patch series, including `engine-zoom-out.patch`, passed
  reverse verification without changing the dependency pins or pre-existing
  submodule edits. A standalone source archive passed safe-path validation and
  patch application; all 109 checked patched inputs matched the built worktree.
- The local runtime was installed as 0.27.0 and the saved desktop wallpaper was
  restored successfully. The GitHub repository remained public.

Caelestia upstream and the installed shell were checked: `Colours.qml` is a
singleton consuming one `scheme.json`. The source selector drives that global
palette; independent per-monitor palettes and fork-specific APIs are unsupported.
A disconnected source falls back to the first detected output; a source with no
owned renderer leaves the existing palette unchanged. The stop action reveals an
existing background surface; restarting a previously stopped third-party
background service is outside this change. Graphical checks cover the tested
Hyprland setup and fixture media, not all projects or compositors. Transient
carousel windows and renderer workers retain separate lifetimes from the single
library window. The published beta installer remains the separately named release
linked from the README; source and local runtime are version 0.27.0.

## Documentation cleanup — 2026-10-29

Repository documentation and contributor instructions were rewritten in English.
Personal references and machine-specific session narratives were removed; public
project URLs and upstream attribution remain. Application translations and
runtime behavior were unchanged. Validation for this change is recorded at the
end of this document.

## 0.1–0.2 — Initial integration (2026-10-26)

Initial checks covered catalog discovery, scene preview, application startup,
renderer persistence and panorama output. A preview fit-mode border defect was
corrected. An early persistence attempt ended unexpectedly; a subsequent test
passed, without establishing the original shutdown cause.

Version 0.2 added a persistent mouse-driven window and favorite carousel.
Graphical selector checks used opt-in test modes and did not establish all
desktop effects, fullscreen behavior or display modes. An interrupted early
shortcut run was not counted as passing. Native shortcut integration was tested
on Hyprland Lua; legacy Hyprlang output was generated but not verified on an older
compositor. Workshop browsing/downloading and independent output assignments were
not available at this stage.

## 0.3–0.3.4 — Popup animation (2026-10-26)

Popup checks covered all four layouts, compact transparent surfaces, rounded
masks, enlarged selection, entry/exit fades, repeated advances, wrap, cancellation
and reopening during fade-out. Process and configuration checks separated popup
navigation from desktop application. Later input behavior confirmed only on
modifier release; Esc/window close canceled.

Native alpha-canvas inspection was used where desktop captures contained unrelated
surfaces. Selection zoom extended outside the mosaic while fitting inside the
native popup. Continuous strip movement preserved velocity through rapid input.
One shared test environment crashed during a run; a separate isolated graphical
and session-handoff check completed. These results cover thumbnail selection,
rather than desktop crossfade, which was not implemented in these versions.

## 0.3.5 — Audio response (2026-10-26)

Stereo output-monitor capture, shader/SceneScript/web audio delivery, disabled
capture, web properties and document reload were exercised. Scene and web
playback shut down cleanly in the final regression. Wallpaper sound and reactive
capture remained independent. Full-library compatibility and physical headset
switching were not established.

## 0.4 — UI refresh (2026-10-28)

Release build and catalog/GIF/preferences/popup tests passed. Popup focus tests
ran 3,000 phases for 1, 2, 4 and 21 entries, preserving bounds, normalized weights
and cyclic-wrap invariance. All layouts passed native rapid-repeat, wrap, cancel
and fade-out/session-handoff checks with isolated state and 32 additional advances
per layout. No desktop renderer was started by those popup checks.

Canvas captures covered dark/light library, output selection, playback,
appearance and confirmation. Recoverable trash retained files and `.trashinfo`;
invalid projects and symlinks were rejected. Cross-filesystem GIO trash failure
had no permanent-delete fallback. Mouse injection was blocked by a locked desktop
and was explicitly not counted as passing. Scene preview still used a separate
window and one-monitor selection replaced the renderer group.

## 0.5 — UI redesign (2026-10-28)

Catalog/filter/metadata regressions and the existing unit targets passed. Native
canvas inspection covered a 1263 × 673 tiled surface, inspector, filters, settings
and trash. CJK titles were checked with FreeType/TTC fallback. Video dimensions
came from ffprobe and scene dimensions from projection metadata, rather than
thumbnail guesses.

All popup layouts passed rapid-repeat/cancel/wrap/session checks; Wave end caps
were inspected. Shortcut capture used actual modifiers and temporarily suspended
only the app-managed binding. Real mouse/recorded-key end-to-end checks remained
blocked by the desktop lock. Install validation retained configuration and
restored the running wallpaper with process identity checks.

## 0.6 — Physical calibration (2026-10-28)

Five CTest targets passed, including mixed DPI, alignment, negative offsets, seam
rounding, connector order, rotation, scale, missing size and invalid payloads.
A static HTML gradient on two outputs retained native 1920 × 1080 and
2560 × 1440 resolutions. Across 400 matched physical heights, mean absolute
red-channel difference improved from 18.92 with pixel layout to 0.18 with physical
calibration. The test used isolated state and verified process identities before
cleanup. No compositor geometry was changed.

## 0.7–0.8 — Placement, library and carousel (2026-10-28)

Checks covered live placement, persistence, display controls, library filtering,
favorites, carousel navigation, refresh and recoverable deletion using isolated
fixtures. Runtime replacement preserved settings and display calibration.
Unsupported creator control types remained visible as limitations rather than
being reported as complete implementations.

## Scene interaction and creator properties (2026-10-28)

Seven CTest targets passed after scene-parser/clipping fixes. Cases included
texture padding, malformed JSON recovery and actual GLSL compilation of corrected
UV expressions. SceneScript fixtures exercised init/applyUserProperties,
scriptProperties, visibility/vector/angle setters, effect visibility and world
cursor events. Audio regression passed web, stereo, disabled capture, SceneScript
and clean shutdown.

Workshop project `3257420414` rendered in one-output and physically calibrated
panorama modes. Two cursor positions moved the lens across outputs and exposed
the second video. Final runs had no shader/object-setup/script errors. An initial
fullscreen run did not reach readiness; repeating with the fullscreen application
minimized passed. Advanced event payloads, alpha-pixel picking, custom material
selection and universal visual equivalence remained unverified.

Eight CTest targets then passed creator conditions, combo/numeric validation,
inert HTML labels and UV limits. A nonzero-origin output fixture moved world X
from 192 to 448 as expected. Real-project single/mirror masks rendered without
shader/script errors. At 200% zoom, mean crop errors were 0.008 horizontal and
0.030 vertical color levels, with no black padding. Compound conditions,
file/directory/scenetexture pickers and advanced engine quality settings remained
incomplete.

## 0.9 — UI customization (2026-10-28)

Customization, shortcut configuration and popup checks used isolated settings and
simulated compositor interfaces where appropriate. Native popup and installed
runtime checks preserved process identity, configuration and display calibration.
These checks did not establish every scene's visual compatibility.

## 0.10 — Catalog compatibility and performance (2026-10-28)

An audit launched 102 installed projects in single, mirror and panorama modes:
306 real starts. The first pass found 10 problematic projects; nine corrected
projects were repeated (27/27) and 11 web projects were repeated after the file-URL
fix (33/33). Combined outcome: 303/306, or 101 projects starting in all three modes.
Each case checked readiness plus 0.6 seconds of survival at a 30 FPS target.

Project `3477054430` remained unsupported: null orthogonal projection,
perspective camera and `.mdl` models require 3D rendering/material support.
Seven presets depended on Audio Visualizer `893418273`; local dependency
resolution retained separate preset identity without editing Steam files.
Readiness is not a visual or interaction audit of every effect.

## 0.11 — Inspector, fonts and sound (2026-10-28)

Nine CTest targets passed, including creator JSON volume and `startsilent`.
Catalog startup audit: 100/101 in single mode, with the documented 3D project
excluded. Twelve additional mirror/panorama starts covered audio and
property-linked-volume projects. Final CEF/audio regressions passed after
lifecycle and message-pump changes.

Inspector checks covered HSV selection, live/persisted zoom, creator reset
preserving position, shared display controls and distinct font previews.
Customization covered 10 languages, theme variants, TTF/TTC, layout, shortcuts and
refresh. A 102-slider panel took up to 2.499 ms to draw during concurrent graphics
tests, with 70 FPS observed.

The property audit found 196 colors, 589 sliders, 334 booleans and 119 combos.
Twenty-nine file, nine directory and two scenetexture controls still needed
pickers. A limited visibility parser did not fully interpret 144 compound
conditions. Cross-process localStorage and scene-reloading properties remained
follow-ups. Unrelated renderers can overlap surfaces or consume resources;
tests did not establish them as the cause of application defects.

## 0.12 — Native Workshop (2026-10-28)

Nine CTest targets passed. Real Steam queries covered AND/OR, type/resolution,
author/text pagination, direct ID/link, metadata, page two and unavailable IDs.
Preset `1806366520` downloaded dependency `893418273`. Real downloads of
`3100052627` (35,578,473 bytes) and `1818234419` (217,520 bytes) completed, including
a one-click UI download and automatic library refresh. These checks performed
real subscriptions; they were separate from desktop rendering.

UI checks exercised UTF-8 clipboard editing, resizable/persisted filters,
settings, author details, search and download. Customization covered 10 languages,
10 themes × two variants, fonts and reload; a 102-property panel reached a maximum
0.767 ms draw time with 90 FPS observed. Active-delete fixtures verified favorite
fallback, library fallback, last-wallpaper stop, recoverable trash and rollback
when replacement or trash failed. Clicking during delayed deletion did not queue
an invalid reapply. Workshop shutdown during active requests exited cleanly.

A post-download catalog contained 102 items, 101 recognized/supported, without a
new full rendering audit. Global Steam display-name lookup returned HTTP 401;
author resolution used encountered identities, profile/vanity links or SteamID64
without extracting browser credentials. Ninety-two sampled projects declared
`schemecolor`; absent direct scene references did not prove scripts never used it.
Static thumbnails and successful downloads do not certify Linux rendering.

## 0.13 — Independent outputs, MPRIS and transitions

Nine CTest targets passed; 279 catalog messages covered 22 locales, with advanced
fallback text still partial. Two independent web renderers verified replacing
one without changing the other's PID/start identity, plus group stop/restore.
Iris start/completion was traced.

An isolated DBus/MPRIS player supplied multilingual metadata before renderer
startup. Position 30 seconds and duration 180 seconds were converted correctly
from microseconds. A real scene's two text scripts received the metadata and
rebuilt textures after fixing the separate text-layer event registry. This
verified the MPRIS contract rather than a new Spotify/YouTube playback session.

Input checks covered overlay blocking, shaped Arabic, automatic Workshop filters
and separate switcher/transition previews. A missing previous snapshot skips the
desktop effect. RTL panel mirroring and Spotify favorites import were not provided.

## 0.14 — Transitions, image profiles and navigation (2026-10-29)

Ten CTest targets passed, including UI key/printf checks for 305 messages × 22
locales. Eight effects used 25 captured frames each on the embedded renderer;
old/new orientation, readiness timing and expected black phases passed. Live
brightness/contrast/saturation persisted and reset without PID change.

Text-cache regression compared 1,100 labels against a 1,024-entry cache:
zero differing pixels across 187,909 text pixels. A real scene's live brightness
change increased measured average intensity from 42.21 to 79.19 without restart
or sampled black frames. Global/local playback plans covered all four
combinations for five options; global OFF always won.

A 205-project synthetic UI catalog exercised pagination, 50/100/200 page sizes,
grouped resolutions, image sliders, local playback, splitter, transparency and
all 22 languages. Real read-only Steam queries checked grouped OR resolutions
inside AND, dependencies and two 100-result pages without duplicate IDs. Workshop
UI auto-applied filters and page size; no new thumbnail subscription was performed
in this version. These checks did not repeat the whole project compatibility audit
or establish HDR color management.

## 0.15 — Local media, active anchor and Caelestia (2026-10-29)

Eleven CTest targets passed, including 328 messages × 22 locales. Native carousel
checks verified favorite order, active-item no-op and temporary nonfavorite
anchors. UI transparency at 0/30% left thumbnail/live-scene pixels identical.
Selecting one output preserved the other renderer's identity.

Generated PNG/MP4 imports verified metadata, thumbnails, deduplication, custom
roots and unchanged original SHA-256. Both rendered with an empty asset directory.
The picker response was simulated while import/refresh were real. Sixteen theme
presets and independent display/theme controls passed UI checks.

Caelestia tests covered valid publication, conditional restore, manual edits,
invalid palettes and stale-result cancellation. A real installed CLI generated
more than 30 roles in isolated XDG state and restored its previous palette on
disable. No static wallpaper was set. At this version the seed was the thumbnail;
rendered-frame generation followed in 0.16. PNG/MP4 success does not certify every
accepted codec. The complete 0.14 transition/MPRIS checks were not rerun here.

## 0.16 — Animation customization and transforms (2026-10-29)

Eleven CTest targets passed. Native UI checks covered switcher size, zoom,
entry/exit/movement, inversion, Wave parameters, mouse geometry and previews.
Image fixtures covered X/Y flips, rotation, color transforms, neutral reset and
same-process live updates. Slide/wipe used four directions with configured
1.2-second linear duration.

All four Material You variants generated distinct chromatic primary/surface
roles in dark/light modes. Optional bootstrap from an empty environment installed
materialyoucolor 3.0.4 and Pillow 12.3.0 into a private venv and generated a valid
palette. A red rendered frame with a deliberately blue thumbnail proved the
actual frame was sampled; enable/disable and prior-palette restoration passed.

Carousel order/anchor checks passed with restored zoom. The text-cache comparison
again produced zero differing pixels for 1,100 labels. Panorama transforms remain
per output and do not transform logical scene interaction. Caelestia updates are
event-driven and global, with a separate shell installation.

## 0.17 — Handoff, reset and languages (2026-10-29)

Application/engine builds and 11/11 CTest passed; ordered patches and manifest
hashes matched unchanged upstream pins. A stress test reproduced a missing effect
on one mirrored output in 0.16 while inspector requests overlapped. Separate
snapshots and a shared gate fixed the same test: 808 concurrent inspector writes,
36 intermediate desktop crossfade samples and no black flash, including delayed
old-renderer retirement.

Eight embedded effects passed orientation and live/persist/reset checks. Nine
real scene changes across three projects and single/mirror/panorama modes used
native output snapshots and one shared gate, with 669 concurrent live requests.
The first-start/no-feed case was also corrected.

Native UI checked direct flips, selective/full/already-neutral reset, preserved
position/zoom/playback/creator values and all 22 languages. The 368-message
catalog included 34 previously fallback rows; Pango rendering found zero missing
glyphs with installed fonts. Native-speaker review, RTL panel mirroring and full
backend localization remained outside scope. Earlier real-project checks covered
readiness/completion but did not yet detect compositor-added transformations.

## 0.18 — Compositor isolation and anchors (2026-10-29)

Application/engine builds and 11/11 CTest passed. On Hyprland 0.56.2 with layer
slide/fade enabled, a striped fixture sampled the desktop before the app's gate.
Without the targeted runtime rule: two unexpected changes in 90 samples. With
`wallpaper-lab-background` isolation: zero in 90. An initial test stopped the old
renderer too early and was discarded after correcting child identity checks.

The GL test reproduced RGBA 180,30,60,0 with alpha writes masked, then
180,30,60,255 after defensive composition, restoring original state. A GPU matrix
of 162 combinations covered eight effects, three curves, three origins, relevant
directions and zoom strengths. Endpoints, opaque alpha, expected black phases and
monotonic iris passed. Six native changes also passed without the alpha defense,
so it was not treated as the sole compositor defect explanation.

Twenty-seven real scene changes across three projects/display modes/configurations
passed snapshot comparisons, shared-gate timing and retirement. Minimum frames
per output were 42/39/132; maximum measured interval was 18.2 ms. Effects configured
for 700/350/1200 ms ended within 716/359/1209 ms. Separate 1.2-second slide/wipe
measurements were 1.176–1.203 seconds. These are environment-specific observations.

All four layouts loaded the nonfavorite active thumbnail and skipped its temporary
slot over subsequent wraps without changing favorites. UI reset preserved
position/zoom/playback; all 29 historical presets and six collections persisted.
Pango found zero missing glyphs across 373 messages × 22 locales. The runtime rule
uses Hyprland's Lua API and is recreated at renderer startup after config reload.

## 0.19 — Binary toggles (2026-10-29)

Eleven CTest targets and localization coverage passed. UI checks exercised
resolution alternatives, five playback controls, creator bool/two-option combo
values, display measurement, inversion and AND/OR. Three-option controls remained
menus; independent multi-select values remained independent. Label copies avoided
static-buffer reuse. Tests used read-only apply mode to avoid renderer reloads
and inspected shortcut/integration toggles without enabling bindings or preparing
extras. An initial focus-loss run was repeated successfully with PID focus checks.
No renderer or engine patch change was needed.

## 0.20–0.21 — Theme design and backgrounds (2026-10-29)

Eleven CTest targets passed in each version. Migration checked 32 historical IDs,
mode/font retention and independent scoped palettes. Native UI checks selected
all historical palettes in 0.20, then all eight active Strawberry/2077 palettes
in 0.21, including restart persistence and four carousel previews.

Background import from a path containing spaces/apostrophe copied to XDG,
survived original-file removal and restart, retained the previous image on invalid
input and reset to the theme preset. Reduced motion produced identical skyline
captures 0.5 seconds apart. Width, Pango centering, UTF-8 clipping and preview
issues were corrected and the full targeted UI test repeated successfully.

These were small synthetic libraries, not large-library benchmarks. Imported
backgrounds were static/center-cropped, GIFs used their first frame and transient
popups initially retained earlier geometry with theme colors. A 0.20 install
exposed the executable-identity effect of replacing a live binary (`deleted`
suffix); subsequent handoff used a verified replacement before retiring the old
process. Configuration and prior runtimes were backed up.

## 0.22 — Fonts, Workshop performance and live mute (2026-10-29)

Application/engine Release builds and 11/11 CTest passed. Patch application was
verified in a temporary copy and all manifest SHA-256 values matched. OFL font
sources, commit pins and hashes were checked. UI tests covered contextual font
menus, fixed theme modes, backgrounds/import, restart/reset and four layouts.

A real 100-result Workshop scroll test observed median 90 FPS and zero new text
rasterizations while idle. CPU text measurement created no textures in 3,000
measurements. Eviction testing again produced zero differing pixels across
187,909 text pixels for 1,100 labels in a 1,024-entry cache.

An isolated PulseAudio null-sink web fixture included HTML audio, Web Audio and
an iframe. Global/local mute, reload and page-side unmute attempts measured RMS
0.0; both gates open measured approximately 0.152–0.158. The renderer PID stayed
unchanged. No default audio device was changed. Synthetic browser mute coverage
does not certify every web wallpaper; fullscreen pause was not forced during the
test, though host mute updates execute before the pause branch.

## 0.23 — Four themes and switchers (2026-10-29)

Release build and 11/11 CTest passed. Engine and pins were unchanged from 0.22.
All four themes, 21 palettes and 16 distinct native backgrounds were checked.
Silkscreen provenance/OFL hashes and process-local font registration were verified.
Suggested/general font selection, override toggling and return to theme font
passed.

The theme suite checked hover feedback, Pixel animation, reduced motion,
16 switcher previews and 16 real popups. Theme, palette, effective font and border
pixels matched, including a General Fonts override. Pixel animation changed more
than 20 pixels by over three RGB levels; reduced motion froze the scene within
one-level compositor dithering tolerance. Fixtures contained three synthetic
projects, so this was not a large-library or rendering-compatibility benchmark.
Install handoff verified replacement readiness and process identity before
retirement, with recoverable runtime/configuration backups.

## 0.24 — Arch open beta (2026-10-29)

Application 0.24.0 / release `v0.24.0-beta.1` retained the 0.23 feature set and
engine pins/patches. A targeted audit of 448 prior historical blobs found no
matches for the credential patterns searched, unexpected binaries beyond fonts,
or user-media paths. It was not proof against every possible sensitive value.

The approximately 135 MiB installer was built from allowlisted outputs: app,
engine, CEF resources/locales, kissfft, helpers, fonts and notices including CEF
`CREDITS.html`. Installed RPATHs were relative and did not reference the development
build directory. System dependencies remained Arch packages, with glibc >= 2.44.
Steamworks preparation separately verified pinned archive/library SHA-256 and ABI
symbols without executing npm/JavaScript.

Twelve CTest targets passed, including seven installer contract cases covering
integrity, tampering/duplicates, unexpected entries, traversal, process identity,
atomic writes and rollback. `beta_install_integration.py` passed isolated
HOME/XDG install, CLI, real Steamworks download/checksum, real Workshop search,
upgrade with preferences/backups, check mode and uninstall preserving data.
An active synthetic-image handoff verified the new renderer before retiring the
old one. A missing-project restore failure kept the old renderer alive and
restored launcher/menu/receipt/configuration intact.

The environment was not a clean Arch VM. HOME/XDG were fresh, but missing package
database entries corresponded to extracted Arch dependencies used by the build.
Only `pacman -Q` was simulated; actual `sudo pacman -S` was not exercised. Loader,
app, Steam network, renderer, filesystem and handoff were real. Other machines
and drivers remain beta-testing scope.

The extracted installer runtime also passed the complete theme suite, all
16 backgrounds, font override, hover/reduced motion, 16 previews and 16 actual
popups. Image/video metadata, deduplication and import passed. Web mute measured
RMS 0.0 when muted and approximately 0.153 when open, with the same PID.

The full source archive was checked without `.git`: 12,239 entries, safe paths,
original submodule pins and standalone patch application. All 107 modified inputs
matched the build worktree byte for byte. The uninitialized optional upstream
test262 corpus was excluded and was not required by the build. No certification
is claimed for other compositors, Windows/macOS or every Workshop project.

## Running checks

After a configured build with its runtime dependencies available:

```sh
ctest --test-dir build --output-on-failure
```

Graphical scripts require an unlocked desktop and, where applicable, Hyprland,
ydotoold and grim. Run desktop input tests one at a time. Tests that apply a
wallpaper must use isolated state and verify owned PID/start/executable before
cleanup. Steam tests may perform real subscriptions/downloads; inspect each
script's options and fixture requirements before running it.

Representative targeted checks:

```sh
python3 tests/local_library_integration.py --help
python3 tests/transition_graphics_integration.py --help
python3 tests/transition_handoff_integration.py --help
python3 tests/theme_rework_integration.py --help
python3 tests/web_mute_integration.py --help
python3 tests/beta_install_integration.py --help
```

## Documentation cleanup validation

- `ctest --test-dir build --output-on-failure`: **12/12 passed** using the
  existing configured build; application source and binaries were unchanged.
- All 11 tracked Markdown documents checked for personal names and absolute
  home paths; no matches. Canonical GitHub project URLs retain the account name.
- Local Markdown link targets validated; all resolved. Tracked project text
  checked for removed personal identifiers; no matches.
- All 17 patch SHA-256 hashes matched `build-manifest.json`; submodule pins and
  pre-existing vendor worktree modifications were preserved.
- `git diff --check`: passed. No new graphical or Workshop-download integration
  run was needed for the documentation change. Historical installer and desktop
  results above remain attributed to their original versions.

This cleanup updates the current source tree. Earlier Git commits and already
published beta archives retain their original content.


## Repository rename and independent runtime check — 2026-10-30

- Renamed the GitHub repository to `WallpaperLab`, verified the repository ID was
  preserved and visibility remained public, and updated `origin`, public guide
  links, the repository homepage and beta release-note links.
- Removed an inspected user-local WallpaperX installation, its application-menu
  entry, launchers, autostart entry and inactive legacy service. Configuration and
  startup files were backed up locally before removal. Steam library content was
  outside the removed application directories.
- Before signaling WallpaperX processes, verified PID, start time and executable;
  used pidfds to prevent PID reuse. Final scan found no remaining WallpaperX
  processes. Wallpaper Lab retained the same live PID/start/executable, status,
  configuration hashes and independent Steamworks library hash. Its process maps
  and runtime links had no dependency on the removed installation.
- After removal, the installed application read its catalog successfully
  (106 entries in the verification environment). A read-only Workshop query
  reached Steam initialization but could not connect because the Steam client was
  not running. A successful online Workshop query is therefore not claimed.
- Validation: **12/12 CTest passed** using the existing configured build; local
  documentation links resolved, all three beta release assets used the renamed
  repository URL, and `git diff --check` passed. No application code, engine
  patches, upstream pins or release binaries changed.

## 0.25 — Independent switcher themes and Arcade (2026-10-30)

- Release build and **12/12 CTest passed**, including catalog, preferences,
  popup lifecycle/motion, process-related persistence and translation coverage.
  Added normalization/migration checks proving UI changes preserve independent
  switcher theme/palette preferences and reject palettes from other themes.
- Updated the graphical theme suite on the active Wayland compositor:
  **16/16 in-app previews and 16/16 actual popups passed** across four themes and
  four layouts. Actual popup UI themes deliberately differed from switcher themes.
  Checked theme/palette trace values, border colors, opaque decorative margin
  pixels, scoped palette counts, palette recall, font override behavior and
  continued separation between preview thumbnails and desktop rendering.
- All 16 app backgrounds produced distinct preview samples within their theme.
  Arcade animation changed **100 pixels** by more than three RGB levels in the
  exposed background sample; reduced motion produced **zero channel difference**.
  The sample now covers the exposed star field rather than a static cabinet row.
  Strawberry button hover feedback remained visible.
- The standalone `theme-art-graphics-test OUTPUT_DIRECTORY` target exported
  eight full-size Strawberry/Arcade compositions and eight chase stages in one
  contact sheet. Visually inspected the simplified rabbit, cakes, neon/prize
  cabinets, power pellet, frightened ghosts, capture scores and returning eyes.
- Native scene and video previews survived their lifecycle smoke checks; a
  foreign process with a matching recorded start time was preserved by Stop.
  The current catalog contained 106 entries. Isolated image/video imports,
  metadata, deduplication, configurable library paths and original-file hashes
  passed. UI tests preserved the personal desktop renderer throughout.
- Installed the verified 0.25 executable locally with configuration/binary
  backups. Started the same wallpaper successfully before retiring its previous
  renderer; verified PID, start time and executable and used pidfds for signaling.
  The installed selector opened successfully. No autostart or shortcut was enabled.
- Limits: graphical results cover the tested Linux/Wayland environment only.
  The arcade chase is a scripted decorative loop, not an interactive maze game.
  UI fonts and reduced motion remain shared settings. Reference images are not
  shipped; the published 0.24 beta installer has not been replaced by this change.
  Existing engine/raylib patch worktrees and upstream pins were preserved.

## 0.26 — Default palettes and plain switchers (2026-10-30)

- Release build and **12/12 CTest passed**. Default now provides **20 palettes,
  10 light and 10 dark**, with stable existing IDs. Preference tests cover
  palette-driven mode, all Default colors, independence from UI changes,
  migration from every old themed palette and removal of retired theme keys.
- The interactive theme suite selected all 20 Default palettes and checked their
  mode. It also passed the 16 background samples, scoped font overrides and
  Arcade motion/reduced-motion checks. Visually inspected light Porcelain and
  the captured plain switcher canvases. Primary text/surface contrast across all
  20 Default palettes was at least **9.07:1** in the palette definitions.
- That interactive run was interrupted by pointer movement before the switcher
  controls section; a complete interactive rerun is not claimed. Added a
  capture-only hidden-surface option so native graphical checks need no desktop
  input. `switcher_colors_integration.py` passed **32 native popup captures**:
  all 20 colors, with four representative light/dark colors across all four
  layouts. Verified fixed Default styling, independent color persistence,
  expected border colors and transparent top margins without decoration bands.
  These captures exercise native OpenGL popup rendering without mapping the
  windows; live compositor placement/focus is not covered by this run.
- The catalog contained 106 entries. Native scene/video preview smoke tests and
  foreign-process preservation passed. Isolated graphics checks preserved the
  personal renderer and never applied their thumbnail fixtures to the desktop.
- Installed 0.26 locally with binary/configuration backups. The same wallpaper
  started successfully before retiring the previous renderer, verifying PID,
  start time and executable and signaling with pidfds. Opened the new selector.
  Other UI themes remain available; switcher artwork and its Theme control are
  removed. Engine/raylib worktrees and pins remain unchanged. No autostart or
  shortcut changes and no replacement public beta installer were made.
