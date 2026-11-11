# Wallpaper Lab user guide

For installation, updates, rollback and removal, see the [installation guide](BETA.md).
The current release targets Arch Linux x86_64 with Hyprland. Project compatibility
depends on the features used by each wallpaper.

## Local image backend: awww

In builds with awww support, open **Settings → Playback → Local image backend**
and choose **awww**. Install awww separately and start `awww-daemon` through your
normal session setup. Wallpaper Lab uses the existing daemon. Imported images
and animated GIFs go to awww; videos and Workshop scene/web projects continue
using the native renderer. Preview windows still use the native renderer.

**Import from Caelestia CLI** reads `~/.config/caelestia/cli.json` (or its XDG
equivalent), backs up Wallpaper Lab's settings, and imports a wallpaper hook
beginning with `awww img "$WALLPAPER_PATH"`. It recognizes transition type, step,
duration, FPS, angle, named origin, and `--invert-y`, including the Hyprland
cursor expression `$(hyprctl cursorpos | tr -d ' ')`. Unsupported command forms
fail without changing settings. The CLI configuration remains unchanged.

A shell command following `&&` is saved as the **Imported post-hook**. Importing
enables that command; review its displayed text and use the toggle to disable it.
It runs after a successful awww application with `WALLPAPER_PATH` set to the
actual imported media file. This can retain an existing cursor-theme rebuild.
Output and failures are recorded in the app's XDG state directory as
`post-hook.log` and `post-hook-status.json`. A failed post-hook keeps the wallpaper
active and displays a notice in Playback settings.

When **Colors from wallpaper** is enabled, awww images use the installed
Caelestia CLI wallpaper pipeline on the actual media file. Scheme flavour, smart
mode/variant selection, quantization, thumbnail cache and theme application follow
`caelestia wallpaper -f`, as with Hellpaper. The CLI wallpaper hook is suppressed
only in the adapter process: Wallpaper Lab already runs awww with your GUI
settings and executes the imported trailing hook separately. Your `cli.json`
stays unchanged. CLI theme hooks still run normally. There is no additional
Wallpaper Lab tint or forced fidelity preset for these images.

Under **Settings → Carousel → Transition**, select any awww effect. **Transition
step** and **Transition duration** sliders appear directly below the style
selector. Step ranges from **1 to 100**, and duration from **0.1 to 10 seconds**.
Click either number to type a value; Enter or clicking elsewhere saves it, and
Esc cancels the edit. The **↑ / ↓** buttons add or subtract one step or one
second from the current value. Duration supports fractional seconds such as
`2.25`; dragging its slider uses 0.1-second increments. Limits also apply to
typed values and backend arguments.
The customize button also exposes duration, step, FPS, origin and Y inversion.
Wipe/wave also expose the angle. These preferences are separate from native transitions
and from the switcher's own animation. Native shader previews are disabled for
awww; apply an image to see the actual effect. Reduced motion selects `none`.

Use one monitor or mirror mode. Multi-monitor panorama is rejected before
launching because awww does not implement Wallpaper Lab's combined canvas.
Native image adjustments, positioning, zoom and playback controls are hidden for
awww images; awww uses its default crop scaling. Cursor origin uses the same
Hyprland coordinates as the imported CLI expression. Mixed-DPI cursor alignment
has not been independently certified.

Carousel selection, saved monitor assignments and restore use the selected
backend. Stop clears only outputs still displaying that application's awww
image, leaves the shared daemon running, and verifies renderer ownership before
signaling a process. A failed awww launch preserves the previous renderer.
`wallpaper-lab --import-awww` performs the same import as the GUI, and `--plan ID`
prints the actual awww argument list for an imported image.

## Library and displays

Click a wallpaper card to apply it and open its live inspector. Closing the UI
leaves the desktop renderer running. Clicking the already active wallpaper does
not restart rendering or trigger a transition.

In **Display**, choose panorama, mirror or one monitor. In one-monitor mode,
select an output in the map, then a wallpaper. Other outputs retain their own
renderers and assignments. The highlighted output remains the target after the
map closes with X or Esc. Assignments are stored by output name and restored
together. Panorama and mirror target the display group.

The display map supports dragging, edge snapping (hold Alt to bypass it) and
top/center/bottom alignment. Overlapping layouts cannot be saved. Physical-size
calibration compensates for different pixel densities; logical pixels are also
available. Missing panel measurements are marked as estimated. Saving a layout
preserves an XDG backup; **Save and apply** reloads the current wallpaper with
that calibration. Calibration affects panorama, while native output resolutions
and the compositor's monitor/window/cursor geometry remain unchanged.

The inspector has collapsible sections for destination, image/crop, playback and
creator properties. Drag the divider to resize the sidebar. Menus appear above
other controls and provide draggable scrollbars when needed. Binary choices use
toggles; multi-select fields retain independent selections.

## Search and navigation

Resolution filters are grouped by single, dual, ultrawide, triple and portrait
layouts. A group matches any of its resolutions. Individual selections follow
the selected AND/OR combination, as do other active criteria. Workshop name and
author always narrow the search. **Reset** clears filter selections.

The grid footer offers 50, 100 or 200 results per page and navigation arrows.
Workshop has the same page-size control. Page size is independent of filters.

| Control | Action |
| --- | --- |
| White star | Add or remove a carousel favorite |
| Trash | Confirm moving a project to the recoverable trash |
| Previous / Next | Apply the previous or next favorite |
| Mouse wheel / scrollbar | Scroll the current panel |
| Ctrl + mouse wheel | Resize thumbnails |
| Ctrl + L | Focus search |
| Ctrl + A / C / V | Select all, copy or paste in text fields |
| Middle-button vertical drag | Scroll grids, properties, settings and menus |
| Esc | Cancel capture or close the active popup/panel |

Deleting the active wallpaper selects the first available alternative favorite,
or another library item if no favorite is available. Steam may download
subscribed projects again. The app has no permanent-delete fallback.

## Local media and library folders

Use the folder-plus button or drag files into the window to import images and
videos. A native picker requires Zenity or KDialog. Imports copy the files and
use ffprobe/FFmpeg to derive names, dimensions and thumbnails. Originals remain
untouched. Reimporting a file with the same path, size and modification time
reuses the existing import.

Accepted images: PNG, JPEG, WebP, GIF, BMP, TIFF and AVIF. Accepted video containers:
MP4, WebM, MKV, MOV, AVI and M4V. Installed FFmpeg and renderer codecs determine
actual playback. Images use an app-managed web project; videos use the video
renderer. These imports do not require Wallpaper Engine's Steam assets. The file
picker does not import arbitrary projects or executables.

Cards distinguish Steam and Local sources. **Settings → Libraries** selects the
Workshop root (usually `steamapps/workshop/content/431960`) and local import root.
The default local root is `$XDG_DATA_HOME/wallpaper-lab/wallpapers`, or
`~/.local/share/wallpaper-lab/wallpapers`. Changing a root changes scanning and
future import destinations without moving existing files. Steam controls its
own download destination.

```sh
wallpaper-lab --import FILE...
wallpaper-lab --library-path local DIRECTORY
wallpaper-lab --library-path steam DIRECTORY
```

## Workshop

Keep Steam running with an account that owns Wallpaper Engine. Catalog queries
and downloads use that client; the app does not collect passwords or cookies.
Filters, sorting, author and search text trigger queries after a short delay.
Reset also clears author, text and sorting. Download from a thumbnail or the
details panel. Downloads subscribe to the item and dependencies through Steam,
then refresh the library. Download availability does not guarantee rendering
compatibility. Workshop thumbnails are previews, not full scene rendering.

## Carousel and shortcuts

Grid, River, Vertical and Wave are available from the switcher selector. Its eye
button opens an in-app animated preview. The adjacent transition selector has a
separate preview for desktop effects: crossfade, fade through black, zoom, slide,
wipe and pointer-centered iris effects. Click inside that preview to change the
iris origin. Close previews with X, Esc or an outside click.

The first shortcut press opens the switcher on the selected output's active
wallpaper; subsequent presses follow favorite order. An active nonfavorite is a
temporary starting card and is skipped on later wraps without changing favorites.
Confirming it leaves the desktop unchanged. A white dot identifies the active
wallpaper. Grid supports hover selection and click confirmation. Other layouts
allow click confirmation without moving their strips on hover.

The shortcut is configured explicitly in the app. On Hyprland, activation checks
conflicts, backs up configuration and manages an app-specific include/binding.
Disabling removes the managed binding; the reusable include remains. Releasing
a modifier confirms the selection by default; Esc cancels. Beside the shortcut
settings, **Keep carousel open** switches to press-once operation. It is off by
default. When enabled, release the shortcut and browse with arrows, Tab or
subsequent shortcut presses. Enter or a click applies the selected wallpaper;
Esc or an outside click cancels. Grid Up/Down moves by a row. Shift+Tab moves
backward when that combination is not itself captured by a global binding.
The persistent popup stays open until you apply or cancel, without an idle
timeout. Toggling this preference does not change or enable your global shortcut;
it takes effect the next time the carousel opens. The app respects shortcut
inhibitors. On other compositors, a manually configured `wallpaper-lab --next`
binding provides direct switching; popup integration needs separate verification.

The customization button next to each selector exposes:

- Switcher size 70–140%, zoom 105–150%, entry/exit 60–600 ms and movement
  100–900 ms. Strip layouts support inversion; Wave adds speed 0–300% and
  amplitude 0–40 px.
- Desktop transition duration 100–2500 ms (initially 700 ms), smooth/linear/ease
  curves, four slide/wipe directions and zoom intensity 5–60%.

Values save on release or selection. Reduced motion takes precedence over
switcher and desktop animation settings. Empty carousels leave the desktop
unchanged; missing or unsupported entries are skipped. Failed replacement
startup preserves the previous renderer. Concurrent changes are rejected while
a switch is in progress.

Transitions capture only the previous wallpaper framebuffer at native output
resolution. Windows and notifications are excluded. If the old renderer cannot
supply a frame, switching proceeds without a transition. Temporary captures stay
in local XDG state. Each output has a separate capture and a shared animation
start signal after renderer readiness. Mirror mode shares the normalized pointer
origin. Iris edges are antialiased and reach the farthest corner.

On Hyprland, an in-memory rule for `wallpaper-lab-background` prevents compositor
slide/fade animations from adding to the app's transition. It is reapplied at
renderer startup and does not edit compositor configuration files.

## Appearance and languages

Four themes provide 38 palettes and 16 included backgrounds. Each remembers its
palette, background and suggested font. Default offers both light and dark
palettes; 2077 and Arcade are dark;
Strawberry core is light. Fresh installations start with Default.

**Suggested Theme Fonts** selects the theme font. Enable **General Fonts** to
apply an independent font throughout the UI and switcher. Disable the checkbox
to return to the theme font. An app background can be imported and reset to a
theme preset. Reduced motion stops decorative animation.
[Theme details and attribution](THEMES.md).

UI transparency ranges from 0% (opaque) to 30%. It affects interface surfaces;
text, thumbnails, Workshop images and live scenes remain opaque. Desktop
rendering is independent of this setting.

The app supports 22 locales: Italian, English, German, Spanish, French,
Portuguese, Dutch, Polish, Romanian, Swedish, Simplified/Traditional Chinese,
Japanese, Korean, Hindi, Russian, Arabic, Bengali, Indonesian, Turkish,
Vietnamese and Urdu. Coverage checks do not replace native-speaker review.
Creator content and Steam tags remain in their original language; some advanced
options use English fallback. Technical backend messages are not fully localized.
Pango/Cairo handles Unicode shaping and Fontconfig fallback, including scene
text. Required script fonts must be installed. RTL text is shaped, but panel
layouts remain left-to-right.

## Monitor selection, zoom and stopping

Open the monitor button or **Settings → Displays**. The destination dropdown lists
model, connector and resolution; the map uses numbered connector labels, with the
complete selected monitor name below it. In **Single monitor** mode, select a
monitor in the map, dropdown or live inspector before applying a wallpaper.
Panorama and mirror still target all connected outputs. Changing the selected
monitor does not apply a wallpaper by itself.

The live inspector's **Zoom** slider accepts **25–300%**. Values below 100% shrink
the image and reveal black padding outside the source. Position and zoom persist
per project and update the renderer live; Workshop thumbnails remain unchanged.

Use the **power icon** in the top right (hover: **Stop and quit**) to stop all Wallpaper Lab desktop renderers,
close previews and exit the library. The existing desktop background underneath
becomes visible again; its background service must still be running. Normal window
close keeps animated wallpapers running. `wallpaper-lab --stop` stops the desktop
renderers without opening a window. Both stop actions conditionally restore the
previous Caelestia palette, respecting later manual edits. Saved wallpapers remain
available to `--restore`; autostart and shortcuts are not changed.

Only one library window runs per XDG state directory. Another launch focuses the
existing library on Hyprland and exits, including settings and Workshop launch
commands. Renderer workers, CLI operations and the transient carousel keep their
separate lifetimes. The kernel releases the library lock even after a crash.

## Optional Caelestia integration

Enable **Settings → Integrations → Caelestia → Wallpaper colors** to derive shell
colors from a wallpaper. **Colour source monitor** chooses the output whose
wallpaper supplies the shared palette, independently of the currently selected
application monitor. The saved source uses the connector name; if disconnected,
the first connected output is used without replacing the saved choice. If that
output has no running wallpaper, the previous palette stays in place.

Upstream Caelestia's [Colours service](https://github.com/caelestia-dots/shell/blob/main/services/Colours.qml)
uses a singleton palette and one `scheme.json`; its per-monitor configuration does
not provide independent wallpaper-derived shell palettes (checked 2026-11-03,
including the installed shell). Wallpaper Lab therefore selects one source for
all shell bars and widgets. Fork-specific per-monitor palette APIs are not used.
The same source can be selected with `wallpaper-lab --caelestia-source DP-1`
(replace the connector with one listed by `--display-layout`).

The integration is initially disabled. Install Caelestia separately.
The optional Python dependencies use existing modules or a private environment
at `$XDG_DATA_HOME/wallpaper-lab/integrations/caelestia/venv`. Preparation may need
Internet access and Python venv support; it does not use sudo or change global
Python packages.

Vibrant, Soft tones, Expressive and Faithful presets generate Material You colors
with automatic/dark/light brightness. Vibrant is the default. A light surface
tint makes the seed visible in bars and widgets. Output uses the existing
`dynamic/default` schema without replacing Caelestia package files.

The app samples the actual rendered frame, including crop and adjustments, and
falls back to the project thumbnail if unavailable. Updates occur on wallpaper
changes and committed adjustments, rather than every frame or beat. The palette
is global; the last wallpaper applied to the selected target wins. Only
`$XDG_STATE_HOME/caelestia/scheme.json` is published. Caelestia's static wallpaper
browser and other applications' themes are separate.

Disabling restores the previous palette only if the current file still matches
the app's last publication. Later manual changes are respected. Preparation and
generation run off the UI thread; stale results after disabling or switching
are discarded. Integration errors do not stop the renderer.

## Image, playback and media controls

Image adjustments include brightness −100…+100%, contrast/saturation 0…200%,
temperature −100…+100%, hue −180…+180°, gamma 0.25…3, vignette, rotation
−180…+180° and horizontal/vertical flip. Changes preview during dragging and
persist on release without restarting the renderer. Rotation crops to fill the
output. Panorama transforms each display separately and does not transform the
logical coordinates of interactive objects. This final framebuffer pass does
not edit Workshop files or provide an HDR color-management guarantee.

The red **Reset image** button restores neutral image adjustments without changing
position, zoom, playback or creator properties. Creator reset is separate.

Per-wallpaper playback options cover wallpaper audio, music response, mouse
interaction, parallax and particles. Both global and local switches must allow
a feature; global disable takes precedence. Enabling a feature does not add it
to projects that do not implement it. Options other than live audio can reload
the wallpaper; per-project settings follow it across display modes and switches.

Global and local audio controls mute immediately without restarting. Web mute
covers media elements, Web Audio and iframes, including reloads and page-side
unmute attempts. Audio capture for music-reactive effects is independent of
wallpaper sound. A local mute stays effective after global audio is reenabled.

MPRIS supplies title, artist, album, playback state and timeline from compatible
players. Timelines use seconds and require a known duration. Newly initialized
scripts receive existing metadata. MPRIS does not expose a Spotify favorites
library. Creator-provided audio playlists remain separate from system metadata.

## Command-line reference

```sh
wallpaper-lab --list              # JSON catalog
wallpaper-lab --plan PROJECT_ID   # Renderer arguments without startup
wallpaper-lab --preview PROJECT_ID
wallpaper-lab --apply PROJECT_ID
wallpaper-lab --status
wallpaper-lab --restore
wallpaper-lab --stop
wallpaper-lab --carousel          # Ordered favorites
wallpaper-lab --favorite PROJECT_ID
wallpaper-lab --next
wallpaper-lab --previous
wallpaper-lab --shortcut SUPER+Tab
wallpaper-lab --shortcut-off
wallpaper-lab --display-layout
wallpaper-lab --display-layout-save physical top
wallpaper-lab --display-settings
```

A full preview uses a renderer window that stays open until closed. The library's
live inspector uses the desktop renderer feed instead of starting another scene.

## Platform limits

The verified environment is Arch Linux x86_64 with Hyprland. Other distributions,
compositors and drivers need testing. Raylib's portability does not make the POSIX
controller and Linux desktop engine portable as a whole. Windows/macOS are not
supported; see the [Windows assessment](WINDOWS_PORT.md). Other wallpaper renderers
on the same outputs may overlap surfaces or consume resources concurrently.
Wallpaper Lab does not terminate unrelated applications.

### Switcher color (0.26)

Use the customize button next to the switcher layout to select **Color palette**.
Choose from 20 Default colors, including 10 light and 10 dark palettes. The four
layouts use the same plain style; there is no switcher Theme control or themed
artwork. The eye button previews the real popup's colors and layout. Changing
the UI theme or palette leaves the switcher color unchanged. Default's own UI
palette dropdown includes the same 20 choices. Other UI themes remain available.

## Interface language and diagnostics

The interface supports 22 languages. Labels, fixed Workshop filter tags, stored
notifications and application error summaries follow the selected language.
Changing language also updates an already-visible Steam error without retrying
the operation. Workshop query identifiers remain unchanged internally.

Native Steam, operating-system and renderer diagnostics may contain arbitrary
text. The interface shows a localized summary and the location of
`$XDG_STATE_HOME/wallpaper-lab/diagnostics.log` (normally under `~/.local/state`).
The log retains the original technical message for troubleshooting. Project
titles, author names, descriptions, unknown creator tags, font family names and
named theme/palette artwork remain original content. CLI help and developer logs
are technical output rather than translated interface content.
