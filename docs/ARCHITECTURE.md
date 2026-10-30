# Architecture

Wallpaper Lab builds one application executable with a raylib UI and a C++
controller exposed through a C API. Its `--render` mode links the native engine
library directly. Rendering runs in a separate instance of the same executable,
so the UI can close while the wallpaper continues.

## Components

| Component | Responsibility |
| --- | --- |
| `catalog`, `project_source`, `project_properties` | Steam/local discovery, preset dependencies and persistent creator properties |
| `workshop` | Serialized Steam UGC worker, manual callbacks and bounded, cancellable HTTPS thumbnail transfers |
| `preferences`, controller | XDG settings, locks, profiles, assignments and process lifecycle |
| `popup`, selector UI | Four animated thumbnail layouts and favorite navigation |
| `settings_ui` | Deferred menu selection and overlay input blocking |
| `shaped_text`, `i18n` | Pango/Cairo shaping, bounded text cache, Fontconfig fallback and 22 locales |
| `local_library.hpp` | Staged media imports and metadata/thumbnail generation |
| `caelestia_integration.hpp` | Optional asynchronous palette generation and conditional restoration |
| `Transition.h` and engine patches | Output composition, image tuning and renderer handoff |
| `build-manifest.json`, `patches/` | Upstream pins, ordered patches and reproducibility metadata |

Workshop previews are static images. Remote project code is not executed before
download/import. Build outputs, captures and runtime data stay in `local/` or XDG
directories. [Test results](../tests/RESULTS.md) describe the verified scope.

## Controller and process lifecycle

`running.json` records PID, start time and executable identity. In one-monitor
mode, `outputs` maps each output to an independent renderer, and a change replaces
only that output. A global change replaces the whole group. The controller starts
a replacement and waits for readiness before retiring the previous renderer.
Startup failure preserves the old wallpaper. Concurrent operations are locked.

The controller refreshes current-process information after output discovery.
Selecting an already active project is a no-op; property changes requiring reload
explicitly request a forced start. Before signaling any process, its identity
must match the recorded PID/start/executable.

## Transition capture and handoff

The inspector requests reduced frames through `request.json` and receives
`frame-OUTPUT.bin`; `frame.bin` remains a compatibility alias. Desktop transitions
instead use `transition-request.json` with a session token, project path and
output. The renderer writes an immutable native-resolution
`snapshot-TOKEN-OUTPUT.bin`. Inspector traffic cannot overwrite it, and another
output in the same process cannot answer for it. Magic, PID, dimensions and
payload length are validated. Cleanup removes only files for the current token.

The new renderer shows the previous snapshot while awaiting readiness and
handoff. After recording the new state and retiring the old renderer, the
controller publishes a shared monotonic timestamp in `start.json`. Each output
uses it for animation timing after three ready frames. An unreadable gate has a
five-second wait limit. Direct `--render` calls without a start file retain
standalone timing. If a previous frame is unavailable, switching has no transition.

Mirror mode shares the normalized pointer position of the output containing the
cursor. Panorama and one-monitor mode use output-relative coordinates. Captures
contain wallpaper framebuffers only, rather than compositor windows.

## OpenGL composition and image tuning

`Transition.h` resolves the current framebuffer into an RGBA8 FBO and combines it
with the previous snapshot. Render targets use bottom-up coordinates; snapshots
use top-down coordinates. Textures use clamp-to-edge and zoom sampling stays
inside the source rectangle. The GL guard restores read/draw framebuffer,
viewport, program, VAO, texture state, unpack alignment, color mask, culling,
stencil and modified enables.

The shared shader applies image adjustments to the new frame. The previous
snapshot already contains its own tuning. Profiles at
`wallpaper_profiles.json[path].image` store brightness, contrast, saturation, hue,
temperature, gamma, vignette, rotation and flips. Rotation adapts crop to fill the
output. Panorama transforms each output separately; scene interaction coordinates
are independent of this final image transform.

Atomic live files contain the project path and image or position payload. The
renderer accepts recent live values for two seconds, then returns to persisted
values, with reads limited to once per 40 ms. `lab_adjust_reset` updates the
image subobject under `ControllerLock` in one write and publishes a live message.
A full reset uses an empty image object, interpreted as neutral values; selective
reset removes only advanced keys. Position, zoom, playback and creator properties
remain separate.

`transition_duration`, `transition_direction`, `transition_curve` and
`transition_zoom` travel in output configuration and preview uniforms. Iris radius
is `length(max(origin, 1-origin) * aspect)`, reaching the farthest corner.
`fwidth`/`smoothstep` antialias the edge; endpoints return the exact old/new image.
Opt-in traces include RGBA samples, snapshot comparisons, timing and frame counts.

## Compositor isolation

On Hyprland, `desktop_surface.hpp` registers a named Lua runtime layer rule using
`hyprctl eval` before `app.setup()`. It matches only
`wallpaper-lab-background` and sets `no_anim=true`, preventing compositor slide/fade
from adding to application effects. A global handle prevents duplicates. After a
configuration reload, the next renderer startup recreates it. Sessions without
`HYPRLAND_INSTANCE_SIGNATURE` skip this step. Popup animation remains separate.
No desktop configuration file is edited.

Native framebuffer tests cannot detect compositor-added transforms. Desktop
pixel regression is therefore separate from shader and readiness checks.

## UI composition, fonts and menus

The UI composes in RGBA when transparency is needed. `window_transparency` (0–30)
affects interface surfaces while media and text remain opaque. Final presentation
uses premultiplied alpha. `window_opacity` remains an inverse legacy key.

Pango/Cairo shapes ASCII and Unicode with grayscale antialiasing and light
hinting. The LRU cache holds 1,024 labels. Texture creation and destruction happen
after flushing queued raylib draws; CPU measurement does not create textures.
FreeType remains for branding and font previews. Fontconfig registers bundled
families within the process. Icons use a 3× atlas with padding and mipmaps;
filled geometry avoids unscalable one-pixel OpenGL outlines.

Menus draw after other fields and defer selection until drawing finishes. Their
overlays block input to covered controls. `Choice` uses `ToggleChoice` for exactly
two alternatives; `BoolChoice`, switches and creator booleans share toggle
components. Two nonboolean options preserve their original value/index. Creator
labels are copied before drawing because the C API reuses a static buffer.
Two-value multi-select fields retain independent toggles. Larger sets use menus.

The translation catalog uses Italian lookup keys and translated values for the
other 21 locales. These are internal identifiers, separate from the English
repository documentation. RTL text is shaped, but panels remain left-to-right.
Backend diagnostics and creator content are not fully localized.

## Catalog, filters and imports

Resolution groups match with an internal OR and then combine with other criteria
using AND/OR. Workshop AND uses `AddRequiredTagGroup` plus individual tags; OR uses
a flattened list. UI pages of 50/100/200 use 1/2/4 Steam queries of 50. Sequential
cursor navigation can exceed numbered-query limits; cursors are cached for up to
64 queries. Filtered author searches scan author pages and count locally, with
the historical 1,000-page author limit.

`local_library.hpp` uses argv-based subprocesses for ffprobe/FFmpeg. It validates
extensions and dimensions, copies to hidden staging, generates metadata and a
thumbnail, then publishes with rename. Scans ignore staging. Import identity
hashes canonical path, size and modification time. Images become managed web
projects with `object-fit: cover`; videos keep native media files. An app-managed
empty asset directory permits local media without Steam assets. Manually placed
scene projects may still need upstream assets.

Catalog metadata includes `lab_origin`, `lab_media_kind`, `lab_width` and
`lab_height`; without markers, projects under the local root are classified as
local. Custom roots replace defaults without migrating files. Pickers, copies
and scans run asynchronously. Catalog replacement happens on the UI thread after
thumbnail workers finish.

## Switcher and themes

Switcher navigation follows `carousel.json`, starting from the selected output's
active wallpaper. A nonfavorite active item is a temporary anchor shown on opening
and skipped on subsequent wraps. Confirmation of the active item is a no-op.
Mouse input uses the actual drawn rectangles, including offsets and enlarged
layers. Grid hover changes selection; strips keep geometry stable until click.
The active dot is shared by cards and selector.

Switcher preferences use explicit percent, pixel and millisecond units.
`popup_frame_scaled` reserves room for maximum zoom and Wave amplitude without
growing the surface on each advance. The in-app demo uses `PopupImages` and the
same rounded shader without starting another process.

The current theme catalog provides four designs, 38 palettes and 16 native
backgrounds. Suggested fonts are remembered per theme; General Fonts is an
independent opt-in override. Historical collection/migration markers preserve
older preferences. Native shaders and drawings implement effects; external theme
CSS and reference-image assets are not executed or bundled.

## Playback and media

Profiles store audio, music reaction, mouse, parallax and particles separately.
Global disables take precedence over local enables. Live audio gates are reread
every 100 ms; CEF host mute is applied before its message loop, including when
visual rendering is paused. MPV updates volume on changes and SDL uses the same
live volume policy. Audio capture for reactive effects is independent of mute.
Other playback changes can reload the project.

MPRIS metadata and timelines are forwarded to scene/web scripts, including those
initialized after metadata arrives. Timeline units are converted to seconds and
require a known duration. Creator audio playlists are separate from system media.

## Optional Caelestia palette generation

`caelestia_palette.py` selects one of four Material You generators. `--prepare`
uses existing Python modules or installs materialyoucolor/Pillow in an XDG venv
only when the feature is enabled. Presets are registered in the data directory;
the shell is installed separately.

The controller requests a fresh frame from its own verified renderer, writes a
temporary PPM and removes it after generation. If unavailable, it uses the project
thumbnail. Surface/background roles get a restrained seed tint. Publication uses
`dynamic/default` for shell compatibility; it does not populate the static
wallpaper browser or change Caelestia package files.

Before publishing `scheme.json`, the callback rechecks enablement, active project,
variant and brightness mode under `ControllerLock`. Late results are discarded
or regenerated. Generation runs off the UI thread and errors are independent of
renderer status. Disabling restores the previous palette only when the current
file still matches the app's last publication. Palette state is global and sampled
after transitions or committed adjustments, rather than continuously per frame.

## Plain switcher colors (0.26)

`switcher_palette` is scoped to the 20 Default palettes and independent of the
UI's `ui_design` and `ui_theme`. Default offers 10 light and 10 dark palettes;
the palette's mode is used consistently by settings, the live UI and the popup.
Existing numeric palette IDs remain stable, with new entries appended.

The renderer scopes a fixed Default override to popup rendering and restores
the UI palette after each preview. Preview and popup share plain tray/card
rendering without themed illustrations, card decorations or margin bands.
`switcher_theme` and `switcher_palette_<design>` have been retired. Migration
maps old themed accents to the nearest Default accent in the same light/dark
mode; existing Default selections remain unchanged. Font selection and reduced
motion remain shared preferences. UI themes themselves remain available.
