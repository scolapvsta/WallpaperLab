# Themes, fonts and switchers

| Theme | Mode | Palettes | Initial suggested font | Included backgrounds |
| --- | --- | ---: | --- | ---: |
| Default | Light / Dark | 20 | System Sans | 4 |
| Strawberry core | Light | 3 | System Sans | 4 |
| 2077 | Dark | 11 | Oxanium | 4 |
| Arcade | Dark | 4 | Silkscreen | 4 |

**Themes** and **Color Palettes** are dropdowns. Each theme has its own palettes,
background and suggested-font selection. Default mode follows its palette; the other themes retain their own mode. Fresh
installations start with Default/Graphite. Existing Strawberry and 2077 palette
IDs are preserved; superseded presets are outside the active catalog.

## Default

Soft surfaces and understated borders with 20 palettes. Dark options are
**Graphite**, **Blue Slate**, **Forest Ink**, **Midnight Blue**, **Aubergine**,
**Rosewood**, **Ember**, **Golden Noir**, **Ocean Depths** and **Carbon**.
Light options are **Porcelain**, **Paper**, **Pearl Grey**, **Sky Mist**,
**Mint Cream**, **Sage Linen**, **Lavender Haze**, **Rose Quartz**,
**Peach Sorbet** and **Buttercream**. Each light palette includes light surfaces,
dark text and a contrasting accent. Backgrounds: **Quiet Gradient**, **Blueprint**, **Contour** and
**Night Dots**. Suggested fonts include Sans, Adwaita Sans and Noto Sans, with
system fallback when needed.

## Strawberry core

Palettes: **Strawberry Milk**, **Raspberry Ribbon** and **Vanilla Shortcake**.
Backgrounds: **Strawberry Gingham** (checks), **Ribbon Atelier** (diagonal ribbons
and bows), **Strawberry Garden** (strawberries, flowers, a small rabbit and cakes) and
**Tea Party Lace** (lace bands and doilies).

Cookie-shaped controls use smooth capsules, pink hover feedback, darker pressed
states and contained icons. Decorations are drawn in application code.

## 2077

Palettes: **Violet Afterglow**, **Pink Voltage**, **Electric Blue**, **Night Drive**,
**Hazard Yellow**, **Crimson Protocol**, **Toxic Lime**, **Amber Circuit**,
**Ice Chrome**, **Ultraviolet** and **Copper Sunset**.

Backgrounds: **Neon District**, **Afterlife Alley**, **Binary Rain** and
**Edge Signal**. Neon skylines, rain, LEDs and binary details accompany the
WALLPAPER LAB / CONTROL DECK header. The 2077 mark uses chromatic separation and
glitch bands for about 0.32 seconds every 1.8 seconds. Reduced motion stops these
animations. Small side accents appear on text buttons.

## Arcade

Formerly Pixel Theme; existing palette, background and font settings retain their
stable IDs. Stepped panels, neon cabinet rows, prize machines, candy platforms,
stars and a crescent moon use original geometric pixel drawings. Palettes remain
**Arcade Berry**, **Dungeon Moss**, **Coin Rush** and **CRT Ocean**. Backgrounds are
**Neon Arcade**, **Candy Quest**, **Coin-op Club** and **Star Dream**.

A decorative Pac-Man chase loops along the lower edge: dots, a power pellet,
blue ghosts, four captures with 200/400/800/1600 scores and returning eyes. This is
a scripted animation, not an interactive game or a simulation of the original
maze AI. Reduced motion freezes it, the header mascot and the background stars.
Silkscreen is bundled under SIL OFL 1.1; alternative suggested fonts include
Oxanium and DejaVu Sans Mono.

Visual direction combines the supplied pink/cyan pixel and arcade references
with [KojochoStudio's arcade props reference](https://kojochostudio.itch.io/neon-accented-arcade-game-center-props-16-pixel-art-pngs).
The [official PAC-MAN manual](https://www.nintendo.co.jp/clv/manuals/en/pdf/CLV-P-NABME_en.pdf)
informs the pellet, frightened-ghost and returning-eyes sequence. Reference
images, game sprites and third-party asset packs are not bundled.

## Suggested Theme Fonts and General Fonts

**Suggested Theme Fonts** selects a font for the current theme and remembers it
per theme. **General Fonts** stores an independent selection and has an adjacent
checkbox. Enabling it overrides the suggested font throughout the UI, including
headers and the switcher; disabling it immediately restores the theme font.
The stored general selection remains available while the override is disabled.
Fonts are registered only within the application process.

2077 suggests Oxanium, Rajdhani and Orbitron. Default and Strawberry suggest Sans,
Adwaita Sans and Noto Sans. Bundled fonts come from Google Fonts at the commit in
`assets/fonts/manifest.json`, with SHA-256 hashes and OFL notices. Missing fonts
and uncovered scripts use Fontconfig/Pango fallback.

- [Silkscreen](https://github.com/google/fonts/tree/main/ofl/silkscreen)
- [Oxanium](https://github.com/google/fonts/tree/main/ofl/oxanium)
- [Orbitron](https://github.com/google/fonts/tree/main/ofl/orbitron)
- [Rajdhani](https://github.com/google/fonts/tree/main/ofl/rajdhani)
- [Edgerunners visual reference](https://www.cyberpunk.net/en/edgerunners)

## Switcher colors

Open the customize button beside the switcher layout and choose **Color palette**.
All 20 Default palettes are available, independently of the main UI theme and
palette. Grid, River, Vertical and Wave always use the plain Default style in
both the preview and the real popup. The Theme control and themed artwork have
been removed from the switcher, including decorative bands and card motifs.
The existing four layouts, zoom and motion settings are retained.

Old themed switcher preferences are migrated to the closest Default accent with
the same light/dark mode. Valid Default colors are preserved. Retired theme and
per-theme switcher palette keys are removed when settings are next saved.
Text continues to use the configured UI font and General Fonts override.
The switcher displays wallpaper thumbnails; confirmation starts full desktop
rendering. Imported app backgrounds remain separate from switcher content.

## Importing an app background

Choose one of four included backgrounds or **Load from PC**. Imported files are
copied as static PNGs, up to 2048 × 2048, into the XDG data directory under
`wallpaper-lab/app-backgrounds/`. The original can then be moved or removed.
**Restore theme** selects the theme's first included background. Invalid images
retain the previous selection.

The imported image is shared across themes and is separate from the desktop
wallpaper catalog. Thumbnails use a canonical 1280 × 720 composition before
scaling. GIFs use their first frame. Import requires Zenity/KDialog and FFmpeg.
Imported backgrounds are static and center-cropped. Native decorative scenes are
stylized drawings; they do not provide 3D rendering.

## Historical collections (0.18)

Before the theme redesign, collections grouped 29 palettes: Classics (5), Neon
(6), Pastel (9), Glass (3), Pixel (3) and LED/RGB (3). These historical options
are not the current theme menu. Palette selection remained independent of UI
transparency, and reduced motion stopped decorative effects.

Historical visual references:

- [Pinterest Palette 2026](https://newsroom.pinterest.com/pdf/pinterest-palette-2026-pdf/):
  cool tones, vivid accents and the Plum Noir surface reference `#351E28`.
- [Dark Neon UI](https://in.pinterest.com/pin/ui_neonline--353954851938453965/):
  dark surfaces and luminous outlines.
- [Lavender Mint](https://in.pinterest.com/pin/696369161170866719/): pastel pairings.
- [Retro Pixel Art Computer](https://de.pinterest.com/pin/retro-pixel-art-computer--704743041686782481/):
  geometric retro panels.

These are palette and visual references. Effects, panels and shaders are
implemented in application code; reference images are not shipped. Some Pinterest
references may require login beyond their indexed preview. License and font
attributions are in [THIRD_PARTY.md](../THIRD_PARTY.md).
