# Wallpaper Lab user guide

For installation, updates, rollback and removal, see the [beta guide](BETA.md).
The current beta targets Arch Linux x86_64 with Hyprland. Project compatibility
depends on the features used by each wallpaper.

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
a modifier confirms the selection; Esc cancels. The app respects shortcut
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

Four themes provide 21 palettes and 16 included backgrounds. Each remembers its
palette, background and suggested font. Default, 2077 and Pixel Theme are dark;
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

## Optional Caelestia integration

Enable **Settings → Integrations → Caelestia → Wallpaper colors** to derive shell
colors from a wallpaper. It is initially disabled. Install Caelestia separately.
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
